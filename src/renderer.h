#pragma once

#include "aether.h"
#include "vulkan/context.h"
#include "vulkan/renderpass.h"
#include "vulkan/pipeline.h"

#include "ui/editor.h"

#include "project/project.h"

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_VULKAN
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define CIMGUI_DEFINE_STR

typedef struct {
    vulkan_context* ctx;
    vulkan_swapchain* swapchain;
    vulkan_pipeline* renderPipeline;
    vulkan_renderpass* renderPass;
    vulkan_renderpass* imguiPass;

    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence inFlight;

    vulkan_image* sceneImageMS;
    vulkan_image* depthImageMS;
    u32 numFramebuffers;
    vulkan_framebuffer** imguiFramebuffers;
    vulkan_framebuffer* renderFramebuffer;

    VkDescriptorPool imguiDescriptorPool;

    ui_editor* editor;
    u32 numModels;
    project_asset_model** models;

    vulkan_descriptor_set_layout* vpLayout;
    vulkan_descriptor_allocator* vpAllocator;
    vulkan_descriptor_set* vpSet;
    vulkan_buffer* vpBuffer;

    vulkan_descriptor_set_layout* materialSetLayout;
    vulkan_descriptor_set_layout* modelSetLayout;
} renderer_renderer;

renderer_renderer* renderer_create(window_window* window);
void renderer_destroy(renderer_renderer* renderer);

void renderer_render(renderer_renderer* renderer);