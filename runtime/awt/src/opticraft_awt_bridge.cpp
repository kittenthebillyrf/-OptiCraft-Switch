#include "opticraft_awt_bridge.h"
#include <cstdlib>
#include <cstring>

namespace {
constexpr int32_t kMaxWidth = 1280;
constexpr int32_t kMaxHeight = 720;
uint32_t *g_pixels = nullptr;
size_t g_capacity = 0;

bool ensure_pixels(int32_t width, int32_t height) {
    if (width <= 0 || height <= 0 || width > kMaxWidth || height > kMaxHeight) return false;
    const size_t needed = static_cast<size_t>(width) * static_cast<size_t>(height);
    if (needed > g_capacity) {
        void *next = std::realloc(g_pixels, needed * sizeof(uint32_t));
        if (!next) return false;
        g_pixels = static_cast<uint32_t *>(next);
        g_capacity = needed;
    }
    std::memset(g_pixels, 0, needed * sizeof(uint32_t));
    return true;
}
}

extern "C" int opticraft_awt_window_init(OptiAwtWindow *window, int32_t width, int32_t height) {
    if (!window) return 0;
    std::memset(window, 0, sizeof(*window));
    window->width = width;
    window->height = height;
    if (!ensure_pixels(width, height)) return 0;
    return 1;
}

extern "C" void opticraft_awt_window_set_preferred_size(OptiAwtWindow *window, int32_t width, int32_t height) {
    if (!window || width <= 0 || height <= 0) return;
    if (width > kMaxWidth || height > kMaxHeight) return;
    window->width = width;
    window->height = height;
    window->packed = 0;
}

extern "C" void opticraft_awt_window_pack(OptiAwtWindow *window) {
    if (!window) return;
    window->packed = ensure_pixels(window->width, window->height) ? 1 : 0;
}

extern "C" void opticraft_awt_window_set_visible(OptiAwtWindow *window, int visible) {
    if (!window) return;
    window->visible = visible ? 1 : 0;
}

extern "C" int opticraft_awt_window_poll(OptiAwtWindow *window, OptiInputSnapshot *input) {
    if (!window || !input || window->closed) return 0;
    std::memset(input, 0, sizeof(*input));
    opticraft_platform_poll_events(input);
    if (input->should_exit) window->closed = 1;
    return window->closed ? 0 : 1;
}

extern "C" uint32_t *opticraft_awt_window_pixels(OptiAwtWindow *window, int32_t *stride_pixels) {
    if (!window || !window->packed || !g_pixels) return nullptr;
    if (stride_pixels) *stride_pixels = window->width;
    return g_pixels;
}

extern "C" int opticraft_awt_window_present(OptiAwtWindow *window) {
    if (!window || !window->visible || !window->packed || !g_pixels) return 0;
    return opticraft_platform_present_rgba8888(g_pixels, window->width, window->height, window->width);
}

extern "C" void opticraft_awt_window_shutdown(OptiAwtWindow *window) {
    if (window) std::memset(window, 0, sizeof(*window));
    std::free(g_pixels);
    g_pixels = nullptr;
    g_capacity = 0;
}
