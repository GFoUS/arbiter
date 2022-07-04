#pragma once

#include "aether.h"
#include "vulkan/context.h"
#include "vulkan/renderpass.h"
#include "vulkan/pipeline.h"

#include "ui/editor.h"

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
    u32 numFramebuffers;
    vulkan_framebuffer** imguiFramebuffers;
    vulkan_framebuffer* renderFramebuffer;

    VkDescriptorPool imguiDescriptorPool;

    ui_editor* editor;
} renderer_renderer;

renderer_renderer* renderer_create(window_window* window);
void renderer_destroy(renderer_renderer* renderer);

void renderer_render(renderer_renderer* renderer);