#include "lwjgl_event_buffers.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

using opti::LwjglEventBuffers;
using opti::PackedInputEvent;
using opti::SwitchInputBridge;

static void expect_bytes(const std::vector<std::uint8_t>& actual,
                         const std::vector<std::uint8_t>& expected) {
    assert(actual == expected);
}

int main() {
    SwitchInputBridge bridge;
    opti::ControllerFrame frame{};
    frame.buttons = (1ull << 1) | (1ull << 7); // B => Escape, R => left mouse
    bridge.update(frame, 960, 544, 0, 0, false);
    const auto events = bridge.drain();

    assert(events.size() == 2);
    LwjglEventBuffers buffers;
    constexpr std::uint64_t timestamp = 0x0102030405060708ull;
    assert(opti::serialize_lwjgl_event_buffers(events, timestamp, bridge, buffers));

    assert(buffers.keyboard_events.size() == opti::LWJGL_KEYBOARD_EVENT_SIZE);
    assert(buffers.mouse_events.size() == opti::LWJGL_MOUSE_EVENT_SIZE);
    assert(buffers.key_state[1] == 1u);
    assert(buffers.mouse_state[0] == 1u);
    assert(buffers.mouse_x == 0 && buffers.mouse_y == 0);

    expect_bytes(buffers.keyboard_events, {
        0,0,0,1, 1, 0,0,0,0, 1,2,3,4,5,6,7,8, 0
    });
    expect_bytes(buffers.mouse_events, {
        0, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0, 1,2,3,4,5,6,7,8
    });

    bridge.reset();
    frame = {};
    frame.right_x = 20000;
    bridge.update(frame, 960, 544, 0, 0, false);
    const auto move_events = bridge.drain();
    assert(move_events.size() == 1);
    assert(move_events[0].type == 5);
    assert(opti::serialize_lwjgl_event_buffers(move_events, timestamp, bridge, buffers));
    assert(buffers.keyboard_events.empty());
    assert(buffers.mouse_events.size() == opti::LWJGL_MOUSE_EVENT_SIZE);
    assert(buffers.mouse_events[0] == 0xffu); // button = -1 for movement
    assert(buffers.mouse_events[1] == 0u);
    assert(buffers.mouse_x > 0);

    const std::vector<PackedInputEvent> invalid{{99, 0, 0, 0}};
    assert(!opti::serialize_lwjgl_event_buffers(invalid, timestamp, bridge, buffers));
    return 0;
}
