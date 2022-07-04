#include "viewport.h"

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_VULKAN
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

void ui_viewport_resize(ui_viewport* viewport, ImVec2 size) {
    vulkan_image_destroy(viewport->sceneImage);
    viewport->sceneImage = vulkan_image_create(viewport->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, (u32)size.x, (u32)size.y, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT);
    
    VkDescriptorImageInfo imageInfo;
    CLEAR_MEMORY(&imageInfo);
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = viewport->sceneImage->imageView;
    imageInfo.sampler = viewport->sceneImageSampler->sampler;

    VkWriteDescriptorSet writeInfo;
    CLEAR_MEMORY(&writeInfo);
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.dstBinding = 0;
    writeInfo.dstSet = viewport->sceneTexture;
    writeInfo.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(viewport->ctx->device->device, 1, &writeInfo, 0, NULL);
}

void ui_viewport_render(ui_element* viewportElement, void(*body)(ui_element*)) {
    ui_viewport* viewport = (ui_viewport*)viewportElement;

    igBegin("Viewport", &viewport->element.open, 0);
    ImVec2 size;
    igGetContentRegionAvail(&size);

    if (size.x != viewport->sceneImage->width || size.y != viewport->sceneImage->height) {
        ui_viewport_resize(viewport, size);
    }

    ImVec2 uv0 = {0.0f, 0.0f};
    ImVec2 uv1 = {1.0f, 1.0f};
    ImVec4 white = {1.0f, 1.0f, 1.0f, 1.0f};
    ImVec4 black = {0.0f, 0.0f, 0.0f, 0.0f};
    igImage(viewport->sceneTexture, size, uv0, uv1, white, black);

    body(&viewport->element);

    igEnd();
}

void ui_viewport_destroy(ui_element* viewportElement) {
    ui_viewport* viewport = (ui_viewport*)viewportElement;
    vulkan_image_destroy(viewport->sceneImage);
    vulkan_sampler_destroy(viewport->sceneImageSampler);
}

ui_viewport* ui_viewport_create(ui_dockspace* parent, vulkan_context* ctx) {
    ui_viewport* viewport = malloc(sizeof(ui_viewport));
    CLEAR_MEMORY(viewport);

    ui_element_config config;
    config.type = UI_ELEMENT_VIEWPORT;
    config.parent = &parent->element;
    config.destroyCallback = ui_viewport_destroy;
    config.renderCallback = ui_viewport_render;
    ui_element_create((ui_element*)viewport, &config);

    viewport->ctx = ctx;
    viewport->sceneImage = vulkan_image_create(viewport->ctx, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT);
    viewport->sceneImageSampler = vulkan_sampler_create(ctx, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    viewport->sceneTexture = ImGui_ImplVulkan_AddTexture(viewport->sceneImageSampler->sampler, viewport->sceneImage->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return viewport;
}