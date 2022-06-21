#pragma once

#include "aether.h"
#include "vulkan/context.h"
#include "vulkan/renderpass.h"
#include "vulkan/pipeline.h"

typedef struct {
    vulkan_context* ctx;
    vulkan_pipeline* pipeline;
    vulkan_renderpass* renderpass;

    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence inFlight;

    vulkan_image* colorImage;
    u32 numFramebuffers;
    vulkan_framebuffer** framebuffers;
} renderer_renderer;

renderer_renderer* renderer_create(window_window* window);
void renderer_destroy(renderer_renderer* renderer);

void renderer_render(renderer_renderer* renderer);