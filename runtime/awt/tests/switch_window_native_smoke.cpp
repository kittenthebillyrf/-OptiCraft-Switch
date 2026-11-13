#include "opticraft_switch_window_native.h"
#include <cassert>
#include <cstdint>
int main() {
    assert(opticraft_switch_window_create(640, 360) == 1);
    opticraft_switch_window_set_preferred_size(800, 450);
    opticraft_switch_window_pack();
    opticraft_switch_window_set_visible(1);
    const uint64_t address = opticraft_switch_window_framebuffer_address();
    assert(address != 0);
    assert(opticraft_switch_window_framebuffer_stride() == 800);
    auto *pixels = reinterpret_cast<uint32_t *>(static_cast<uintptr_t>(address));
    pixels[0] = 0xff123456u;
    /* The host backend deliberately reports no physical display. */
    assert(opticraft_switch_window_present() == 0);
    assert(opticraft_switch_window_should_close() == 0);
    opticraft_switch_window_destroy();
    return 0;
}
