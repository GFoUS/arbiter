#pragma once

#include "element.h"
#include "dockspace.h"

#include "vulkan/image.h"
#include "vulkan/context.h"

typedef struct {
    ui_element element;

    vulkan_context* ctx;
    vulkan_image* sceneImage;
    vulkan_sampler* sceneImageSampler;
    VkDescriptorSet sceneTexture;
} ui_viewport;

ui_viewport* ui_viewport_create(ui_dockspace* parent, vulkan_context* ctx);