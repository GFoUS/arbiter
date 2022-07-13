#include "aether.h"
#include "layers/layers.h"
#include "renderer.h"

static bool shouldClose = false;

void onWindowClose(void* data) {
    shouldClose = true;
}

int main() {
    aether_init();
    layers_init();
    INFO("Initialised layers");

    event_listener* listener = event_listener_create();
    listener->windowClose = onWindowClose;
    event_bus_listen(*(event_listener_generic*)listener);

    i32 width, height;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), NULL, NULL, &width, &height);
    window_config windowConfig;
    CLEAR_MEMORY(&windowConfig);
    windowConfig.width = width;
    windowConfig.height = height;
    windowConfig.title = "ARBITER";
    window_window* window = window_create(&windowConfig);

    renderer_renderer* renderer = renderer_create(window);

    while (!shouldClose) {
        renderer_render(renderer);
        window_poll(window);
    }

    renderer_destroy(renderer);
    window_destroy(window);
    layers_deinit();
    aether_deinit();
    free(listener);
    return 0;
}