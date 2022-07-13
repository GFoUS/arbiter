#include "font.h"

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_VULKAN
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#include "string.h"

void ui_font_load(vulkan_context* ctx, const char* basePath) {
    const char* weights[] = {"Light", "Regular", "Medium", "SemiBold", "Bold"};
    ImGuiIO* io = igGetIO();
    for (u32 i = 0; i < ARRAY_SIZE(weights); i++) {
        char* path = malloc(sizeof(char) * (strlen(basePath) + 10));
        CLEAR_MEMORY_ARRAY(path, strlen(basePath) + 10);
        memcpy(path, basePath, sizeof(char) * strlen(basePath));
        strcat(path, weights[i]);
        strcat(path, ".ttf");
        ImFontAtlas_AddFontFromFileTTF(io->Fonts, path, 16.0f, NULL, NULL);
    }

    // Upload fonts
    VkCommandBuffer cmd = vulkan_context_start_recording(ctx);
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    vulkan_context_submit(ctx, cmd);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}