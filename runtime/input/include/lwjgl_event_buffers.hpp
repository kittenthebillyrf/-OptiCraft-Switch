#pragma once
#include "switch_input_bridge.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace opti {

constexpr std::size_t LWJGL_KEYBOARD_EVENT_SIZE = 18;
constexpr std::size_t LWJGL_MOUSE_EVENT_SIZE = 22;
constexpr std::size_t LWJGL_KEYBOARD_STATE_SIZE = 256;
constexpr std::size_t LWJGL_MOUSE_STATE_SIZE = 8;

// LWJGL 2 reads event records from Java ByteBuffers. ByteBuffer.allocate()
// uses BIG_ENDIAN, so every multi-byte field is serialized in network order.
struct LwjglEventBuffers {
    std::vector<std::uint8_t> keyboard_events;
    std::vector<std::uint8_t> mouse_events;
    std::array<std::uint8_t, LWJGL_KEYBOARD_STATE_SIZE> key_state{};
    std::array<std::uint8_t, LWJGL_MOUSE_STATE_SIZE> mouse_state{};
    std::int32_t mouse_x = 0;
    std::int32_t mouse_y = 0;
};

bool serialize_lwjgl_event_buffers(
    const std::vector<PackedInputEvent>& events,
    std::uint64_t timestamp_ns,
    const SwitchInputBridge& bridge,
    LwjglEventBuffers& output);

} // namespace opti
