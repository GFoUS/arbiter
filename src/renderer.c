#include "renderer.h"

#include "cimgui_impl.h"
#include "cglm/cglm.h"
#include "project/ecs/world.h"
#include "project/ecs/components/model.h"
#include "ui/colors.h"

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
	colors[ImGuiCol_WindowBg] = color_to_imvec4(STONE900);

	// Headers
	colors[ImGuiCol_Header] = color_to_imvec4(STONE800);
	colors[ImGuiCol_HeaderHovered] = color_to_imvec4(STONE700);
	colors[ImGuiCol_HeaderActive] = color_to_imvec4(STONE900);

	// Buttons
	colors[ImGuiCol_Button] = color_to_imvec4(STONE800);
	colors[ImGuiCol_ButtonHovered] = color_to_imvec4(STONE700);
	colors[ImGuiCol_ButtonActive] = color_to_imvec4(TEAL500);

	// Frame BG
	colors[ImGuiCol_FrameBg] = color_to_imvec4(STONE800);
	colors[ImGuiCol_FrameBgHovered] = color_to_imvec4(STONE700);
	colors[ImGuiCol_FrameBgActive] = color_to_imvec4(STONE900);

	// Tabs
	colors[ImGuiCol_Tab] = color_to_imvec4(STONE800);
	colors[ImGuiCol_TabHovered] = color_to_imvec4(STONE700);
	colors[ImGuiCol_TabActive] = color_to_imvec4(TEAL500);
	colors[ImGuiCol_TabUnfocused] = color_to_imvec4(STONE800);
	colors[ImGuiCol_TabUnfocusedActive] = color_to_imvec4(STONE900);

	// Title
	colors[ImGuiCol_TitleBg] = color_to_imvec4(STONE800);
	colors[ImGuiCol_TitleBgActive] = color_to_imvec4(STONE900);
	colors[ImGuiCol_TitleBgCollapsed] = color_to_imvec4(STONE800);

    colors[ImGuiCol_MenuBarBg] = color_to_imvec4(STONE900);

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
    vulkan_descriptor_set_layout_builder_add(materialSetLayoutBuilder, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    vulkan_descriptor_set_layout_builder_add(materialSetLayoutBuilder, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    renderer->materialSetLayout = vulkan_descriptor_set_layout_builder_build(materialSetLayoutBuilder, renderer->ctx->device);

    vulkan_descriptor_set_layout_builder* modelSetLayoutBuilder = vulkan_descriptor_set_layout_builder_create();
    vulkan_descriptor_set_layout_builder_add(modelSetLayoutBuilder, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    renderer->modelSetLayout = vulkan_descriptor_set_layout_builder_build(modelSetLayoutBuilder, renderer->ctx->device);

    vulkan_descriptor_set_layout_builder* gBufferSetLayoutBuilder = vulkan_descriptor_set_layout_builder_create();
    vulkan_descriptor_set_layout_builder_add(gBufferSetLayoutBuilder, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    vulkan_descriptor_set_layout_builder_add(gBufferSetLayoutBuilder, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    vulkan_descriptor_set_layout_builder_add(gBufferSetLayoutBuilder, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    vulkan_descriptor_set_layout_builder_add(gBufferSetLayoutBuilder, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    renderer->gBufferSetLayout = vulkan_descriptor_set_layout_builder_build(gBufferSetLayoutBuilder, renderer->ctx->device);
    renderer->gBufferAllocator = vulkan_descriptor_allocator_create(renderer->ctx->device, renderer->gBufferSetLayout);
}

void renderer_create_resources(renderer_renderer* renderer) {
    if (renderer->editor->viewport->sceneImage->width != 0 && renderer->editor->viewport->sceneImage->height != 0) {
        u32 dims[2] = { renderer->editor->viewport->sceneImage->width, renderer->editor->viewport->sceneImage->height };
        renderer->albedoImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, dims[0], dims[1], VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
        renderer->positionImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, dims[0], dims[1], VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
        renderer->normalImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, dims[0], dims[1], VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
        renderer->metallicRoughnessImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, dims[0], dims[1], VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
        renderer->depthImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, dims[0], dims[1], VK_IMAGE_ASPECT_DEPTH_BIT, renderer->ctx->physical->maxSamples);
        renderer->sceneImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, dims[0], dims[1], VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
    } else {
        renderer->albedoImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
        renderer->positionImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
        renderer->normalImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
        renderer->metallicRoughnessImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
        renderer->depthImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1, 1, VK_IMAGE_ASPECT_DEPTH_BIT, renderer->ctx->physical->maxSamples);
        renderer->sceneImageMS = vulkan_image_create(renderer->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, renderer->ctx->physical->maxSamples);
    }

    vulkan_image* renderFramebufferImages[] = {renderer->albedoImageMS, renderer->positionImageMS, renderer->normalImageMS, renderer->metallicRoughnessImageMS, renderer->depthImageMS, renderer->sceneImageMS, renderer->editor->viewport->sceneImage};
    renderer->renderFramebuffer = vulkan_framebuffer_create(renderer->ctx->device, renderer->renderPass, ARRAY_SIZE(renderFramebufferImages), renderFramebufferImages);
    renderer->gBufferSet = vulkan_descriptor_set_allocate(renderer->gBufferAllocator);
    vulkan_descriptor_set_write_input_attachment(renderer->gBufferSet, 0, renderer->albedoImageMS);
    vulkan_descriptor_set_write_input_attachment(renderer->gBufferSet, 1, renderer->positionImageMS);
    vulkan_descriptor_set_write_input_attachment(renderer->gBufferSet, 2, renderer->normalImageMS);
    vulkan_descriptor_set_write_input_attachment(renderer->gBufferSet, 3, renderer->metallicRoughnessImageMS);
}

void renderer_destroy_resources(renderer_renderer* renderer) {
    vulkan_image_destroy(renderer->albedoImageMS);
    vulkan_image_destroy(renderer->positionImageMS);
    vulkan_image_destroy(renderer->normalImageMS);
    vulkan_image_destroy(renderer->metallicRoughnessImageMS);
    vulkan_image_destroy(renderer->depthImageMS);
    vulkan_image_destroy(renderer->sceneImageMS);
    vulkan_framebuffer_destroy(renderer->renderFramebuffer);
}

renderer_renderer* renderer_create(window_window* window) {
    renderer_renderer* renderer = malloc(sizeof(renderer_renderer));
    CLEAR_MEMORY(renderer);
    renderer->ctx = vulkan_context_create(window);
    renderer->swapchain = vulkan_swapchain_create(renderer->ctx);
    renderer_create_descriptors(renderer);

    VkAttachmentDescription colorDescription = vulkan_renderpass_get_default_color_attachment(VK_FORMAT_R8G8B8A8_SRGB, renderer->ctx->physical->maxSamples);
    colorDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAttachmentDescription depthDescription = vulkan_renderpass_get_default_depth_attachment(renderer->ctx->physical->maxSamples);
    VkAttachmentDescription resolveDescription = vulkan_renderpass_get_default_resolve_attachment(VK_FORMAT_R8G8B8A8_SRGB);

    vulkan_renderpass_builder* renderPassBuilder = vulkan_renderpass_builder_create();
    
    vulkan_subpass_attachment albedoMSAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &colorDescription);
    VkAttachmentDescription positionMSDescription = vulkan_renderpass_get_default_color_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, renderer->ctx->physical->maxSamples);
    vulkan_subpass_attachment positionMSAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &positionMSDescription);
    vulkan_subpass_attachment normalMSAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &colorDescription);
    vulkan_subpass_attachment metallicRoughnessMSAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &positionMSDescription);
    vulkan_subpass_attachment depthMSAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &depthDescription);
    vulkan_subpass_attachment sceneMSAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &colorDescription);
    
    resolveDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vulkan_subpass_attachment sceneAttachment = vulkan_renderpass_builder_add_attachment(renderPassBuilder, &resolveDescription);

    vulkan_subpass_config geometrySubpass;
    CLEAR_MEMORY(&geometrySubpass);
    vulkan_subpass_attachment geometryColorAttachments[] = {albedoMSAttachment, positionMSAttachment, normalMSAttachment, metallicRoughnessMSAttachment};
    geometrySubpass.numColorAttachments = ARRAY_SIZE(geometryColorAttachments);
    geometrySubpass.colorAttachments = geometryColorAttachments;
    geometrySubpass.isDepthBuffered = true;
    geometrySubpass.depthAttachment = depthMSAttachment;
    vulkan_renderpass_builder_add_subpass(renderPassBuilder, &geometrySubpass);

    vulkan_subpass_config renderSubpass;
    CLEAR_MEMORY(&renderSubpass);
    renderSubpass.numInputAttachments = ARRAY_SIZE(geometryColorAttachments);
    renderSubpass.inputAttachments = geometryColorAttachments;
    renderSubpass.numColorAttachments = 1;
    renderSubpass.colorAttachments = &sceneMSAttachment;
    renderSubpass.isDepthBuffered = false;
    renderSubpass.isResolving = true;
    renderSubpass.resolveAttachments = &sceneAttachment;
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

    vulkan_shader* geometryVertexShader = vulkan_shader_load_from_file(renderer->ctx->device, "assets/shaders/geometry.vert.spv", VERTEX);
    vulkan_shader* geometryFragmentShader = vulkan_shader_load_from_file(renderer->ctx->device, "assets/shaders/geometry.frag.spv", FRAGMENT);

    VkPipelineColorBlendAttachmentState geometryBlendAttachment;
    CLEAR_MEMORY(&geometryBlendAttachment);
    geometryBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
    geometryBlendAttachment.blendEnable = VK_FALSE;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    vulkan_descriptor_set_layout* geometrySetLayouts[] = {renderer->vpLayout, renderer->materialSetLayout, renderer->modelSetLayout};

    vulkan_pipeline_config geometryPipelineConfig;
    CLEAR_MEMORY(&geometryPipelineConfig);
    geometryPipelineConfig.vertexShader = geometryVertexShader;
    geometryPipelineConfig.fragmentShader = geometryFragmentShader;
    geometryPipelineConfig.subpass = 0;
    geometryPipelineConfig.renderpass = renderer->renderPass;
    geometryPipelineConfig.numSetLayouts = 3;
    geometryPipelineConfig.setLayouts = geometrySetLayouts;
    geometryPipelineConfig.numBlendingAttachments = 4;
    VkPipelineColorBlendAttachmentState geometryBlendAttachments[] = {geometryBlendAttachment, geometryBlendAttachment, geometryBlendAttachment, geometryBlendAttachment};
    geometryPipelineConfig.blendingAttachments = geometryBlendAttachments;
    geometryPipelineConfig.rasterizerCullMode = VK_CULL_MODE_NONE;
    geometryPipelineConfig.samples = renderer->ctx->physical->maxSamples;
    geometryPipelineConfig.numDynamicStates = 2;
    geometryPipelineConfig.dynamicStates = dynamicStates;
    geometryPipelineConfig.verticesFromBuffer = true;
    renderer->geometryPipeline = vulkan_pipeline_create(renderer->ctx->device, &geometryPipelineConfig);

    vulkan_shader_destroy(geometryVertexShader);
    vulkan_shader_destroy(geometryFragmentShader);

    vulkan_shader* renderVertexShader = vulkan_shader_load_from_file(renderer->ctx->device, "assets/shaders/render.vert.spv", VERTEX);
    vulkan_shader* renderFragmentShader = vulkan_shader_load_from_file(renderer->ctx->device, "assets/shaders/render.frag.spv", FRAGMENT);

    VkPipelineColorBlendAttachmentState renderBlendAttachment;
    CLEAR_MEMORY(&renderBlendAttachment);
    renderBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
    renderBlendAttachment.blendEnable = VK_FALSE;

    vulkan_descriptor_set_layout* renderSetLayouts[] = {renderer->vpLayout, renderer->gBufferSetLayout};

    vulkan_pipeline_config renderPipelineConfig;
    CLEAR_MEMORY(&renderPipelineConfig);
    renderPipelineConfig.vertexShader = renderVertexShader;
    renderPipelineConfig.fragmentShader = renderFragmentShader;
    renderPipelineConfig.subpass = 1;
    renderPipelineConfig.renderpass = renderer->renderPass;
    renderPipelineConfig.numSetLayouts = 2;
    renderPipelineConfig.setLayouts = renderSetLayouts;
    renderPipelineConfig.numBlendingAttachments = 1;
    renderPipelineConfig.blendingAttachments = &renderBlendAttachment;
    renderPipelineConfig.rasterizerCullMode = VK_CULL_MODE_NONE;
    renderPipelineConfig.samples = renderer->ctx->physical->maxSamples;
    renderPipelineConfig.numDynamicStates = 2;
    renderPipelineConfig.dynamicStates = dynamicStates;
    renderer->renderPipeline = vulkan_pipeline_create(renderer->ctx->device, &renderPipelineConfig);

    vulkan_shader_destroy(renderVertexShader);
    vulkan_shader_destroy(renderFragmentShader);

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

    renderer_create_resources(renderer);

    return renderer;
}

void renderer_destroy(renderer_renderer* renderer) {
    vkDeviceWaitIdle(renderer->ctx->device->device);

    ui_editor_destroy(renderer->editor);

    vkDestroyDescriptorPool(renderer->ctx->device->device, renderer->imguiDescriptorPool, NULL);

    vkDestroySemaphore(renderer->ctx->device->device, renderer->imageAvailable, NULL);
    vkDestroySemaphore(renderer->ctx->device->device, renderer->renderFinished, NULL);
    vkDestroyFence(renderer->ctx->device->device, renderer->inFlight, NULL);

    renderer_destroy_resources(renderer);

    vulkan_renderpass_destroy(renderer->renderPass);
    vulkan_renderpass_destroy(renderer->imguiPass);
    vulkan_pipeline_destroy(renderer->renderPipeline);
    vulkan_pipeline_destroy(renderer->geometryPipeline);

    vulkan_descriptor_allocator_destroy(renderer->vpAllocator);
    vulkan_descriptor_allocator_destroy(renderer->gBufferAllocator);
    vulkan_descriptor_set_layout_destroy(renderer->vpLayout);
    vulkan_descriptor_set_layout_destroy(renderer->materialSetLayout);
    vulkan_descriptor_set_layout_destroy(renderer->modelSetLayout);
    vulkan_descriptor_set_layout_destroy(renderer->gBufferSetLayout);
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
        renderer_destroy_resources(renderer);
        renderer_create_resources(renderer);
    }

    // Update view/ projection matrices
    frame_data frame;
    ui_camera_get_eye(renderer->editor->viewport->camera, frame.cameraPosition);
    glm_mat4_copy(renderer->editor->viewport->camera->view, frame.view);
    glm_mat4_copy(renderer->editor->viewport->camera->proj, frame.proj);

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
    VkClearValue geometryPassClearValues[7];
    CLEAR_MEMORY_ARRAY(geometryPassClearValues, ARRAY_SIZE(geometryPassClearValues));
    memset(geometryPassClearValues[0].color.float32, 0, sizeof(float) * 4);
    memset(geometryPassClearValues[1].color.float32, 0, sizeof(float) * 4);
    memset(geometryPassClearValues[2].color.float32, 0, sizeof(float) * 4);
    memset(geometryPassClearValues[3].color.float32, 0, sizeof(float) * 4);
    geometryPassClearValues[4].depthStencil.depth = 1.0f;
    memset(geometryPassClearValues[5].color.float32, 0, sizeof(float) * 4);
    memset(geometryPassClearValues[6].color.float32, 0, sizeof(float) * 4);
    renderpassInfo.clearValueCount = ARRAY_SIZE(geometryPassClearValues);
    renderpassInfo.pClearValues = geometryPassClearValues;

    vkCmdBeginRenderPass(cmd, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->geometryPipeline->pipeline);

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

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->geometryPipeline->layout->layout, 0, 1, &renderer->vpSet->set, 0, NULL);

    ecs_world* world = ecs_world_get();
    for (u32 i = 0; i < world->numEntities; i++) {
        ecs_component_model* modelComponent = (ecs_component_model*)ecs_entity_get_component(world->entities[i], COMPONENT_TYPE_MODEL);
        if (modelComponent != NULL && modelComponent->asset != NULL) {
            model_render(((project_asset_model*)modelComponent->asset->loadedData)->model, cmd, renderer->geometryPipeline->layout->layout);
        }
    }

    vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderPipeline->pipeline);

    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderPipeline->layout->layout, 1, 1, &renderer->gBufferSet->set, 0, NULL);

    vkCmdDraw(cmd, 3, 1, 0, 0);

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