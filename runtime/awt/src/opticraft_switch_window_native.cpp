#include "opticraft_switch_window_native.h"
#include "opticraft_awt_bridge.h"
#include <cstdint>
#include <cstring>

namespace {
OptiAwtWindow g_window{};
OptiInputSnapshot g_input{};
bool g_platform_ready = false;
}

extern "C" int32_t opticraft_switch_window_create(int32_t width, int32_t height) {
    if (!g_platform_ready) {
        if (opticraft_platform_init() != 0) return 0;
        g_platform_ready = true;
    }
    opticraft_awt_window_shutdown(&g_window);
    std::memset(&g_window, 0, sizeof(g_window));
    return opticraft_awt_window_init(&g_window, width, height) ? 1 : 0;
}
extern "C" void opticraft_switch_window_set_preferred_size(int32_t width, int32_t height) {
    opticraft_awt_window_set_preferred_size(&g_window, width, height);
}
extern "C" void opticraft_switch_window_pack(void) {
    opticraft_awt_window_pack(&g_window);
}
extern "C" void opticraft_switch_window_set_visible(int32_t visible) {
    opticraft_awt_window_set_visible(&g_window, visible != 0);
}
extern "C" int32_t opticraft_switch_window_poll(void) {
    return opticraft_awt_window_poll(&g_window, &g_input);
}
extern "C" uint64_t opticraft_switch_window_buttons(void) { return g_input.buttons; }
extern "C" uint64_t opticraft_switch_window_pressed(void) { return g_input.pressed; }
extern "C" int32_t opticraft_switch_window_left_x(void) { return g_input.left_x; }
extern "C" int32_t opticraft_switch_window_left_y(void) { return g_input.left_y; }
extern "C" int32_t opticraft_switch_window_right_x(void) { return g_input.right_x; }
extern "C" int32_t opticraft_switch_window_right_y(void) { return g_input.right_y; }
extern "C" int32_t opticraft_switch_window_touch_x(void) { return g_input.touch_x; }
extern "C" int32_t opticraft_switch_window_touch_y(void) { return g_input.touch_y; }
extern "C" int32_t opticraft_switch_window_touch_down(void) { return g_input.touch_down ? 1 : 0; }
extern "C" uint64_t opticraft_switch_window_framebuffer_address(void) {
    int32_t stride = 0;
    uint32_t *pixels = opticraft_awt_window_pixels(&g_window, &stride);
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pixels));
}
extern "C" int32_t opticraft_switch_window_framebuffer_stride(void) {
    int32_t stride = 0;
    (void)opticraft_awt_window_pixels(&g_window, &stride);
    return stride;
}
extern "C" int32_t opticraft_switch_window_present(void) {
    return opticraft_awt_window_present(&g_window) == 0 ? 1 : 0;
}
extern "C" int32_t opticraft_switch_window_should_close(void) {
    return g_window.closed ? 1 : 0;
}
extern "C" void opticraft_switch_window_destroy(void) {
    opticraft_awt_window_shutdown(&g_window);
    if (g_platform_ready) {
        opticraft_platform_shutdown();
        g_platform_ready = false;
    }
    std::memset(&g_input, 0, sizeof(g_input));
}
