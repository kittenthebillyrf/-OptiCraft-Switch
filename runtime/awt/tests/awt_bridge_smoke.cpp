#include "opticraft_awt_bridge.h"
#include <cassert>
#include <cstdint>

int main() {
    assert(opticraft_platform_init());
    OptiAwtWindow window{};
    assert(opticraft_awt_window_init(&window, 854, 480));
    assert(window.width == 854 && window.height == 480);
    opticraft_awt_window_set_preferred_size(&window, 960, 544);
    opticraft_awt_window_pack(&window);
    assert(window.packed == 1);
    int32_t stride = 0;
    uint32_t *pixels = opticraft_awt_window_pixels(&window, &stride);
    assert(pixels != nullptr && stride == 960);
    pixels[0] = 0xff336699u;
    opticraft_awt_window_set_visible(&window, 1);
    assert(opticraft_awt_window_present(&window));
    OptiInputSnapshot input{};
    assert(opticraft_awt_window_poll(&window, &input));
    opticraft_awt_window_shutdown(&window);
    opticraft_platform_shutdown();
    return 0;
}
