#include "renderer.h"

#include "cimgui_impl.h"
#include "cglm/cglm.h"
#include "project/ecs/world.h"
#include "project/ecs/components/model.h"

typedef struct ImGui_ImplVulkan_InitInfo ImGui_ImplVulkan_InitInfo;

typedef struct {
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
} frame_data;

void renderer_init_imgui(renderer_renderer* renderer) {
    igCreateContext(0);

    igStyleColorsDark(NULL);
    ImVec4* colors = igGetStyle()->Colors;
    ImVec4 dark1 = { 0.1f, 0.105f, 0.11f, 1.0f };
    ImVec4 dark2 = { 0.15f, 0.1505f, 0.151f, 1.0f };
    ImVec4 dark3 = { 0.2f, 0.205f, 0.21f, 1.0f };
    ImVec4 dark4 = { 0.28f, 0.2805f, 0.281f, 1.0f };
    ImVec4 dark5 = { 0.3f, 0.305f, 0.31f, 1.0f };
    ImVec4 dark6 = {  0.38f, 0.3805f, 0.381f, 1.0f };
	colors[ImGuiCol_WindowBg] = dark1;

	// Headers
	colors[ImGuiCol_Header] = dark3;
	colors[ImGuiCol_HeaderHovered] = dark5;
	colors[ImGuiCol_HeaderActive] = dark2;

	// Buttons
	colors[ImGuiCol_Button] = dark3;
	colors[ImGuiCol_ButtonHovered] = dark5;
	colors[ImGuiCol_ButtonActive] = dark2;

	// Frame BG
	colors[ImGuiCol_FrameBg] = dark3;
	colors[ImGuiCol_FrameBgHovered] = dark5;
	colors[ImGuiCol_FrameBgActive] = dark2;

	// Tabs
	colors[ImGuiCol_Tab] = dark2;
	colors[ImGuiCol_TabHovered] = dark6;
	colors[ImGuiCol_TabActive] = dark4;
	colors[ImGuiCol_TabUnfocused] = dark2;
	colors[ImGuiCol_TabUnfocusedActive] = dark3;

	// Title
	colors[ImGuiCol_TitleBg] = dark2;
	colors[ImGuiCol_TitleBgActive] = dark2;
	colors[ImGuiCol_TitleBgCollapsed] = dark2;

    ImGuiIO* io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Create ImGui Descriptor Pool
    VkDescriptorPoolSize poolSizes[11] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo poolInfo;
    CLEAR_MEMORY(&poolInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * 11;
    poolInfo.poolSizeCount = 11;
    poolInfo.pPoolSizes = poolSizes;
    VkResult imguiDescriptorPoolResult = vkCreateDescriptorPool(renderer->ctx->device->device, &poolInfo, NULL, &renderer->imguiDescriptorPool);
    if (imguiDescriptorPoolResult != VK_SUCCESS) {
        FATAL("Imgui descriptor pool creation failed with error code: %d", imguiDescriptorPoolResult);
    }

    ImGui_ImplGlfw_InitForVulkan(renderer->ctx->window->window, true);
    ImGui_ImplVulkan_InitInfo imguiVulkan;
    CLEAR_MEMORY(&imguiVulkan);
    imguiVulkan.Instance = renderer->ctx->instance->instance;
    imguiVulkan.PhysicalDevice = renderer->ctx->physical->physical;
    imguiVulkan.Device = renderer->ctx->device->device;
    imguiVulkan.QueueFamily = renderer->ctx->physical->queues.graphicsIndex;
    imguiVulkan.Queue = renderer->ctx->device->graphics;
    imguiVulkan.PipelineCache = NULL;
    imguiVulkan.DescriptorPool = renderer->imguiDescriptorPool;
    imguiVulkan.Allocator = NULL;
    imguiVulkan.MinImageCount = renderer->swapchain->numImages;
    imguiVulkan.ImageCount = renderer->swapchain->numImages;
    imguiVulkan.CheckVkResultFn = NULL;
    ImGui_ImplVulkan_Init(&imguiVulkan, renderer->imguiPass->renderpass);
}

void renderer_destroy_swapchain(renderer_renderer* renderer) {
    vkDeviceWaitIdle(renderer->ctx->device->device);
    for (u32 i = 0; i < renderer->numFramebuffers; i++) {
        vulkan_framebuffer_destroy(renderer->imguiFramebuffers[i]);
    }
    free(renderer->imguiFramebuffers);

    vulkan_swapchain_destroy(renderer->swapchain);
}

void renderer_recreate_swapchain(renderer_renderer* renderer) {
    vkDeviceWaitIdle(renderer->ctx->device->device);
    renderer_destroy_swapchain(renderer);

    renderer->swapchain = vulkan_swapchain_create(renderer->ctx);

    renderer->numFramebuffers = renderer->swapchain->numImages;
    renderer->imguiFramebuffers = malloc(sizeof(vulkan_framebuffer*) * renderer->numFramebuffers);
    CLEAR_MEMORY_ARRAY(renderer->imguiFramebuffers, renderer->numFramebuffers);

    CLEAR_MEMORY_ARRAY(renderer->imguiFramebuffers, renderer->numFramebuffers);
    for (u32 i = 0; i < renderer->numFramebuffers; i++) {
        renderer->imguiFramebuffers[i] = vulkan_framebuffer_create(renderer->ctx->device, renderer->imguiPass, 1, &renderer->swapchain->images[i]);
    }
}

void renderer_create_descriptors(renderer_renderer* renderer) {
    vulkan_descriptor_set_layout_builder* vpLayoutBuilder = vulkan_descriptor_set_layout_builder_create();
    vulkan_descriptor_set_layout_builder_add(vpLayoutBuilder, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    renderer->vpLayout = vulkan_descriptor_set_layout_builder_build(vpLayoutBuilder, renderer->ctx->device);
    renderer->vpAllocator = vulkan_descriptor_allocator_create(renderer->ctx->device, renderer->vpLayout);
    renderer->vpSet = vulkan_descriptor_set_allocate(renderer->vpAllocator);
    renderer->vpBuffer = vulkan_buffer_create(renderer->ctx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(frame_data));
    vulkan_descriptor_set_write_buffer(renderer->vpSet, 0, renderer->vpBuffer);

    vulkan_descriptor_set_layout_builder* materialSetLayoutBuilder = vulkan_descriptor_set_layout_builder_create();
    vulkan_descriptor_set_layout_builder_add(materialSetLayoutBuilder, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    vulkan_descriptor_set_layout_builder_add(materialSetLayoutBuilder, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    renderer->materialSetLayout = vulkan_descriptor_set_layout_builder_build(materialSetLayoutBuilder, renderer->ctx->device);

    vulkan_descriptor_set_layout_builder* modelSetLayoutBuilder = vulkan_descriptor_set_layout_builder_create();
    vulkan_descriptor_set_layout_builder_add(modelSetLayoutBuilder, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    renderer->modelSetLayout = vulkan_descriptor_set_layout_builder_build(modelSetLayoutBuilder, renderer->ctx->device);
}

renderer_renderer* renderer_create(window_window* window) {
    renderer_renderer* renderer = malloc(sizeof(renderer_renderer));
    CLEAR_MEMORY(renderer);
    renderer->ctx = vulkan_context_create(window);
    renderer->swapchain = vulkan_swapchain_create(renderer->ctx);
    renderer_create_descriptors(renderer);

    vulkan_renderpass_builder* renderPassBuilder = vulkan_renderpass_builder_create();
    VkAttachmentDescription colorDescription = vulkan_renderpass_get_default_color_attachment(VK_FORMAT_R8G8B8A8_SRGB , renderer->ctx->physical->maxSamples);
    vulkan_subpass_attachment colorAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &colorDescription);
    VkAttachmentDescription depthDescription = vulkan_renderpass_get_default_depth_attachment(renderer->ctx->physical->maxSamples);
    vulkan_subpass_attachment depthAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &depthDescription);
    VkAttachmentDescription resolveDescription = vulkan_renderpass_get_default_resolve_attachment(VK_FORMAT_R8G8B8A8_SRGB);
    resolveDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vulkan_subpass_attachment resolveAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &resolveDescription);

    vulkan_subpass_config renderSubpass;
    CLEAR_MEMORY(&renderSubpass);
    renderSubpass.numColorAttachments = 1;
    renderSubpass.colorAttachments = &colorAttachment;
    renderSubpass.isDepthBuffered = true;
    renderSubpass.depthAttachment = depthAttachment;
    renderSubpass.isResolving = true;
    renderSubpass.resolveAttachment = resolveAttachment;
    vulkan_renderpass_builder_add_subpass(renderPassBuilder, &renderSubpass);

    renderer->renderPass = vulkan_renderpass_builder_build(renderPassBuilder, renderer->ctx->device);

    vulkan_renderpass_builder* imguiPassBuilder = vulkan_renderpass_builder_create();
    VkAttachmentDescription imguiDescription = vulkan_renderpass_get_default_resolve_attachment(renderer->swapchain->format);
    vulkan_subpass_attachment imguiAttachment = vulkan_renderpass_builder_add_attachment(imguiPassBuilder, &imguiDescription);

    vulkan_subpass_config imguiSubpass;
    CLEAR_MEMORY(&imguiSubpass);
    imguiSubpass.numColorAttachments = 1;
    imguiSubpass.colorAttachments = &imguiAttachment;
    vulkan_renderpass_builder_add_subpass(imguiPassBuilder, &imguiSubpass);

    renderer->imguiPass = vulkan_renderpass_builder_build(imguiPassBuilder, renderer->ctx->device);

    vulkan_shader* vertexShader = vulkan_shader_load_from_file(renderer->ctx->device, "assets/shaders/shader.vert.spv", VERTEX);
    vulkan_shader* fragmentShader = vulkan_shader_load_from_file(renderer->ctx->device, "assets/shaders/shader.frag.spv", FRAGMENT);

    VkPipelineColorBlendAttachmentState blendAttachment;
    CLEAR_MEMORY(&blendAttachment);
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
    blendAttachment.blendEnable = VK_FALSE;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    vulkan_descriptor_set_layout* setLayouts[] = {renderer->vpLayout, renderer->materialSetLayout, renderer->modelSetLayout};

    vulkan_pipeline_config pipelineConfig;
    CLEAR_MEMORY(&pipelineConfig);
    pipelineConfig.vertexShader = vertexShader;
    pipelineConfig.fragmentShader = fragmentShader;
    pipelineConfig.subpass = 0;
    pipelineConfig.renderpass = renderer->renderPass;
    pipelineConfig.numSetLayouts = 3;
    pipelineConfig.setLayouts = setLayouts;
    pipelineConfig.numBlendingAttachments = 1;
    pipelineConfig.blendingAttachments = &blendAttachment;
    pipelineConfig.rasterizerCullMode = VK_CULL_MODE_NONE;
    pipelineConfig.samples = renderer->ctx->physical->maxSamples;
    pipelineConfig.numDynamicStates = 2;
    pipelineConfig.dynamicStates = dynamicStates;
    renderer->renderPipeline = vulkan_pipeline_create(renderer->ctx->device, &pipelineConfig);

    vulkan_shader_destroy(vertexShader);
    vulkan_shader_destroy(fragmentShader);

    renderer->imageAvailable = vulkan_context_get_semaphore(renderer->ctx, 0);
    renderer->renderFinished = vulkan_context_get_semaphore(renderer->ctx, 0);
    renderer->inFlight = vulkan_context_get_fence(renderer->ctx, VK_FENCE_CREATE_SIGNALED_BIT);

    renderer->numFramebuffers = renderer->swapchain->numImages;
    renderer->imguiFramebuffers = malloc(sizeof(vulkan_framebuffer*) * renderer->numFramebuffers);

    CLEAR_MEMORY_ARRAY(renderer->imguiFramebuffers, renderer->numFramebuffers);
    for (u32 i = 0; i < renderer->numFramebuffers; i++) {
        renderer->imguiFramebuffers[i] = vulkan_framebuffer_create(renderer->ctx->device, renderer->imguiPass, 1, &renderer->swapchain->images[i]);
    }

    renderer_init_imgui(renderer);
    renderer->editor = ui_editor_create(renderer->ctx);

    renderer->sceneImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
    renderer->depthImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1, 1, VK_IMAGE_ASPECT_DEPTH_BIT, renderer->ctx->physical->maxSamples);

    vulkan_image* images[] = {renderer->sceneImageMS, renderer->depthImageMS, renderer->editor->viewport->sceneImage};
    renderer->renderFramebuffer = vulkan_framebuffer_create(renderer->ctx->device, renderer->renderPass, 3, images);

    return renderer;
}

void renderer_destroy(renderer_renderer* renderer) {
    vkDeviceWaitIdle(renderer->ctx->device->device);

    ui_editor_destroy(renderer->editor);

    vkDestroyDescriptorPool(renderer->ctx->device->device, renderer->imguiDescriptorPool, NULL);

    vkDestroySemaphore(renderer->ctx->device->device, renderer->imageAvailable, NULL);
    vkDestroySemaphore(renderer->ctx->device->device, renderer->renderFinished, NULL);
    vkDestroyFence(renderer->ctx->device->device, renderer->inFlight, NULL);

    vulkan_framebuffer_destroy(renderer->renderFramebuffer);

    vulkan_renderpass_destroy(renderer->renderPass);
    vulkan_renderpass_destroy(renderer->imguiPass);
    vulkan_pipeline_destroy(renderer->renderPipeline);

    vulkan_image_destroy(renderer->sceneImageMS);
    vulkan_image_destroy(renderer->depthImageMS);

    vulkan_descriptor_allocator_destroy(renderer->vpAllocator);
    vulkan_descriptor_set_layout_destroy(renderer->vpLayout);
    vulkan_descriptor_set_layout_destroy(renderer->materialSetLayout);
    vulkan_descriptor_set_layout_destroy(renderer->modelSetLayout);
    vulkan_buffer_destroy(renderer->vpBuffer);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);

    project_destroy();

    renderer_destroy_swapchain(renderer);
    vulkan_context_destroy(renderer->ctx);
    free(renderer);
}

void renderer_render(renderer_renderer* renderer) {
    static VkCommandBuffer cmd = NULL;

    vkWaitForFences(renderer->ctx->device->device, 1, &renderer->inFlight, VK_TRUE, UINT64_MAX);
    vkResetFences(renderer->ctx->device->device, 1, &renderer->inFlight);

    if (cmd != NULL) {
        vulkan_command_pool_free_buffer(renderer->ctx->commandPool, cmd);
    }

    uint32_t imageIndex;
    VkResult aquireResult = vkAcquireNextImageKHR(renderer->ctx->device->device, renderer->swapchain->swapchain, UINT64_MAX, renderer->imageAvailable, VK_NULL_HANDLE, &imageIndex);
    if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR || aquireResult == VK_SUBOPTIMAL_KHR) {
        renderer_recreate_swapchain(renderer);
        vkAcquireNextImageKHR(renderer->ctx->device->device, renderer->swapchain->swapchain, UINT64_MAX, renderer->imageAvailable, VK_NULL_HANDLE, &imageIndex);
    } else if (aquireResult != VK_SUCCESS) {
        FATAL("Image aquisition failed with error code: %d", aquireResult);
    }

    ui_editor_render(renderer->editor);

    ImDrawData* drawData = igGetDrawData();

    if (renderer->sceneImageMS->width != renderer->editor->viewport->sceneImage->width || renderer->sceneImageMS->height != renderer->editor->viewport->sceneImage->height) {
        vulkan_image_destroy(renderer->sceneImageMS);
        vulkan_image_destroy(renderer->depthImageMS);
        vulkan_framebuffer_destroy(renderer->renderFramebuffer);

        renderer->sceneImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, renderer->editor->viewport->sceneImage->width, renderer->editor->viewport->sceneImage->height, VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
        renderer->depthImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, renderer->editor->viewport->sceneImage->width, renderer->editor->viewport->sceneImage->height, VK_IMAGE_ASPECT_DEPTH_BIT, renderer->ctx->physical->maxSamples);
        vulkan_image* images[] = {renderer->sceneImageMS, renderer->depthImageMS, renderer->editor->viewport->sceneImage};
        renderer->renderFramebuffer = vulkan_framebuffer_create(renderer->ctx->device, renderer->renderPass, 3, images);
    }

    // Update view/ projection matrices
    frame_data frame;
    CLEAR_MEMORY(&frame);
    static float cameraDist = 0.0f;
    cameraDist += 0.01f;
    vec3 eye = {cameraDist, 0.5f, cameraDist};
    vec3 center = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, -1.0f, 0.0f};
    glm_lookat(eye, center, up, frame.view);
    glm_perspective(45.0f, (float)renderer->editor->viewport->sceneImage->width / (float)renderer->editor->viewport->sceneImage->height, 0.1f, 10000.0f, frame.proj);
    memcpy(frame.cameraPosition, eye, sizeof(vec3));

    vulkan_buffer_update(renderer->vpBuffer, sizeof(frame), (void*)&frame);

    u32 numModels = 0;
    project_asset** assets = project_get_assets_with_type(&numModels, ASSET_TYPE_MODEL);
    project_asset_model_load_config modelLoadConfig;
    CLEAR_MEMORY(&modelLoadConfig);
    modelLoadConfig.ctx = renderer->ctx;
    modelLoadConfig.materialSetLayout = renderer->materialSetLayout;
    modelLoadConfig.modelSetLayout = renderer->modelSetLayout;
    for (u32 i = 0; i < numModels; i++) {
        if (!assets[i]->numTimesLoaded) {
            project_asset_model_load(assets[i], &modelLoadConfig);
        }
    }

    cmd = vulkan_context_start_recording(renderer->ctx);
    VkRenderPassBeginInfo renderpassInfo;
    CLEAR_MEMORY(&renderpassInfo);
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassInfo.renderPass = renderer->renderPass->renderpass;
    renderpassInfo.framebuffer = renderer->renderFramebuffer->framebuffer;
    renderpassInfo.renderArea.offset.x = 0;
    renderpassInfo.renderArea.offset.y = 0;
    renderpassInfo.renderArea.extent.width = renderer->editor->viewport->sceneImage->width;
    renderpassInfo.renderArea.extent.height = renderer->editor->viewport->sceneImage->height;
    VkClearValue clearValues[2];
    CLEAR_MEMORY_ARRAY(clearValues, 2);
    memset(clearValues[0].color.float32, 0, sizeof(float) * 4);
    clearValues[1].depthStencil.depth = 1.0f;
    renderpassInfo.clearValueCount = 2;
    renderpassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(cmd, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderPipeline->pipeline);

    VkViewport viewport;
    CLEAR_MEMORY(&viewport);
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)renderer->editor->viewport->sceneImage->width;
    viewport.height = (float)renderer->editor->viewport->sceneImage->height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor;
    CLEAR_MEMORY(&scissor);
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = renderer->editor->viewport->sceneImage->width;
    scissor.extent.height = renderer->editor->viewport->sceneImage->height;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderPipeline->layout->layout, 0, 1, &renderer->vpSet->set, 0, NULL);

    ecs_world* world = ecs_world_get();
    for (u32 i = 0; i < world->numEntities; i++) {
        ecs_component_model* modelComponent = (ecs_component_model*)ecs_entity_get_component(world->entities[i], COMPONENT_TYPE_MODEL);
        if (modelComponent != NULL && modelComponent->asset != NULL) {
            model_render(((project_asset_model*)modelComponent->asset->loadedData)->model, cmd, renderer->renderPipeline->layout->layout);
        }
    }

    vkCmdEndRenderPass(cmd);
    
    renderpassInfo.renderPass = renderer->imguiPass->renderpass;
    renderpassInfo.framebuffer = renderer->imguiFramebuffers[imageIndex]->framebuffer;
    renderpassInfo.renderArea.extent = renderer->swapchain->extent;
    vkCmdBeginRenderPass(cmd, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(drawData, cmd, VK_NULL_HANDLE);
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
    presentInfo.pSwapchains = &renderer->swapchain->swapchain;
    presentInfo.pImageIndices = &imageIndex;
    
    vkQueuePresentKHR(renderer->ctx->device->present, &presentInfo);
}