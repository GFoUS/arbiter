#include "renderer.h"

renderer_renderer* renderer_create(window_window* window) {
    renderer_renderer* renderer = malloc(sizeof(renderer_renderer));
    CLEAR_MEMORY(renderer);
    renderer->ctx = vulkan_context_create(window);

    vulkan_renderpass_builder* renderpassBuilder = vulkan_renderpass_builder_create();
    VkAttachmentDescription colorDescription = vulkan_renderpass_get_default_color_attachment(renderer->ctx->swapchain->format, renderer->ctx->physical->maxSamples);
    vulkan_subpass_attachment colorAttachment = vulkan_renderpass_builder_add_attachment(renderpassBuilder, &colorDescription);
    VkAttachmentDescription resolveDescription = vulkan_renderpass_get_default_resolve_attachment(renderer->ctx->swapchain->format);
    vulkan_subpass_attachment resolveAttachment = vulkan_renderpass_builder_add_attachment(renderpassBuilder, &resolveDescription);

    vulkan_subpass_config subpassConfig;
    CLEAR_MEMORY(&subpassConfig);
    subpassConfig.numColorAttachments = 1;
    subpassConfig.colorAttachments = &colorAttachment;
    subpassConfig.isResolving = true;
    subpassConfig.resolveAttachment = resolveAttachment;
    vulkan_renderpass_builder_add_subpass(renderpassBuilder, &subpassConfig);

    renderer->renderpass = vulkan_renderpass_builder_build(renderpassBuilder, renderer->ctx->device);

    vulkan_shader* vertexShader = vulkan_shader_load_from_file(renderer->ctx->device, "shaders/shader.vert.spv", VERTEX);
    vulkan_shader* fragmentShader = vulkan_shader_load_from_file(renderer->ctx->device, "shaders/shader.frag.spv", FRAGMENT);

    VkPipelineColorBlendAttachmentState blendAttachment;
    CLEAR_MEMORY(&blendAttachment);
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
    blendAttachment.blendEnable = VK_FALSE;

    vulkan_pipeline_config pipelineConfig;
    CLEAR_MEMORY(&pipelineConfig);
    pipelineConfig.vertexShader = vertexShader;
    pipelineConfig.fragmentShader = fragmentShader;
    pipelineConfig.width = window->width;
    pipelineConfig.height = window->height;
    pipelineConfig.subpass = 0;
    pipelineConfig.renderpass = renderer->renderpass;
    pipelineConfig.numSetLayouts = 0;
    pipelineConfig.numBlendingAttachments = 1;
    pipelineConfig.blendingAttachments = &blendAttachment;
    pipelineConfig.rasterizerCullMode = VK_CULL_MODE_FRONT_BIT;
    pipelineConfig.samples = renderer->ctx->physical->maxSamples;
    renderer->pipeline = vulkan_pipeline_create(renderer->ctx->device, &pipelineConfig);

    vulkan_shader_destroy(vertexShader);
    vulkan_shader_destroy(fragmentShader);

    renderer->imageAvailable = vulkan_context_get_semaphore(renderer->ctx, 0);
    renderer->renderFinished = vulkan_context_get_semaphore(renderer->ctx, 0);
    renderer->inFlight = vulkan_context_get_fence(renderer->ctx, VK_FENCE_CREATE_SIGNALED_BIT);

    renderer->colorImage = vulkan_image_create(renderer->ctx, renderer->ctx->swapchain->format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, renderer->ctx->swapchain->extent.width, renderer->ctx->swapchain->extent.height, VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
    renderer->numFramebuffers = renderer->ctx->swapchain->numImages;
    renderer->framebuffers = malloc(sizeof(vulkan_framebuffer*) * renderer->numFramebuffers);
    CLEAR_MEMORY_ARRAY(renderer->framebuffers, renderer->numFramebuffers);
    for (u32 i = 0; i < renderer->numFramebuffers; i++) {
        vulkan_image* images[] = {renderer->colorImage, renderer->ctx->swapchain->images[i]};
        renderer->framebuffers[i] = vulkan_framebuffer_create(renderer->ctx->device, renderer->renderpass, 2, images);
    }

    return renderer;
}

void renderer_destroy(renderer_renderer* renderer) {
    vkDeviceWaitIdle(renderer->ctx->device->device);

    vulkan_image_destroy(renderer->colorImage);

    vkDestroySemaphore(renderer->ctx->device->device, renderer->imageAvailable, NULL);
    vkDestroySemaphore(renderer->ctx->device->device, renderer->renderFinished, NULL);
    vkDestroyFence(renderer->ctx->device->device, renderer->inFlight, NULL);

    vulkan_renderpass_destroy(renderer->renderpass);
    vulkan_pipeline_destroy(renderer->pipeline);

    vulkan_context_destroy(renderer->ctx);
    free(renderer);
}


void renderer_render(renderer_renderer* renderer) {
    vkWaitForFences(renderer->ctx->device->device, 1, &renderer->inFlight, VK_TRUE, UINT64_MAX);
    vkResetFences(renderer->ctx->device->device, 1, &renderer->inFlight);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(renderer->ctx->device->device, renderer->ctx->swapchain->swapchain, UINT64_MAX, renderer->imageAvailable, VK_NULL_HANDLE, &imageIndex);

    VkCommandBuffer cmd = vulkan_context_start_recording(renderer->ctx);
    VkRenderPassBeginInfo renderpassInfo;
    CLEAR_MEMORY(&renderpassInfo);
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassInfo.renderPass = renderer->renderpass->renderpass;
    renderpassInfo.framebuffer = renderer->framebuffers[imageIndex]->framebuffer;
    renderpassInfo.renderArea.offset.x = 0;
    renderpassInfo.renderArea.offset.y = 0;
    renderpassInfo.renderArea.extent = renderer->ctx->swapchain->extent;
    renderpassInfo.clearValueCount = 1;
    VkClearValue clearValue;
    CLEAR_MEMORY(&clearValue);
    memset(clearValue.color.float32, 0, sizeof(float) * 4);
    renderpassInfo.pClearValues = &clearValue;
    
    vkCmdBeginRenderPass(cmd, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline->pipeline);
    vkCmdDraw(cmd, 3, 1, 0, 0);
    vkCmdEndRenderPass(cmd);
    VkResult endResult = vkEndCommandBuffer(cmd);
    if (endResult != VK_SUCCESS) {
        FATAL("Command buffer recording failed with error code: %d", endResult);
    }

    VkSubmitInfo submitInfo;
    CLEAR_MEMORY(&submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &renderer->imageAvailable;
    submitInfo.pWaitDstStageMask = waitStages;   
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderer->renderFinished;

    VkResult submitResult = vkQueueSubmit(renderer->ctx->device->graphics, 1, &submitInfo, renderer->inFlight);
    if (submitResult != VK_SUCCESS) {  
        FATAL("Command buffer submission failed with error code: %d", submitResult);
    }

    VkPresentInfoKHR presentInfo;
    CLEAR_MEMORY(&presentInfo);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderer->renderFinished;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &renderer->ctx->swapchain->swapchain;
    presentInfo.pImageIndices = &imageIndex;
    
    vkQueuePresentKHR(renderer->ctx->device->present, &presentInfo);
}