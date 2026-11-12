#include "lwjgl_event_buffers.hpp"

namespace opti {
namespace {

void put_u32_be(std::vector<std::uint8_t>& out, std::uint32_t value) {
    out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xffu));
    out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xffu));
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xffu));
    out.push_back(static_cast<std::uint8_t>(value & 0xffu));
}

void put_u64_be(std::vector<std::uint8_t>& out, std::uint64_t value) {
    for (int shift = 56; shift >= 0; shift -= 8) {
        out.push_back(static_cast<std::uint8_t>((value >> shift) & 0xffu));
    }
}

void write_keyboard_event(
    std::vector<std::uint8_t>& out,
    const PackedInputEvent& event,
    std::uint64_t timestamp_ns) {
    put_u32_be(out, static_cast<std::uint32_t>(event.code));
    out.push_back(event.type == 1 ? 1u : 0u); // key down / key up
    put_u32_be(out, 0u);                     // no translated character
    put_u64_be(out, timestamp_ns);
    out.push_back(0u);                       // controller events never repeat
}

void write_mouse_event(
    std::vector<std::uint8_t>& out,
    const PackedInputEvent& event,
    std::uint64_t timestamp_ns) {
    const std::int8_t button = event.type == 5
        ? static_cast<std::int8_t>(-1)
        : static_cast<std::int8_t>(event.code);
    const std::uint8_t pressed = event.type == 3 ? 1u : 0u;
    out.push_back(static_cast<std::uint8_t>(button));
    out.push_back(pressed);
    put_u32_be(out, static_cast<std::uint32_t>(event.x));
    put_u32_be(out, static_cast<std::uint32_t>(event.y));
    put_u32_be(out, 0u); // wheel delta
    put_u64_be(out, timestamp_ns);
}

} // namespace

bool serialize_lwjgl_event_buffers(
    const std::vector<PackedInputEvent>& events,
    std::uint64_t timestamp_ns,
    const SwitchInputBridge& bridge,
    LwjglEventBuffers& output) {
    output.keyboard_events.clear();
    output.mouse_events.clear();
    output.key_state.fill(0u);
    output.mouse_state.fill(0u);
    output.mouse_x = bridge.mouse_x();
    output.mouse_y = bridge.mouse_y();

    for (int key = 0; key < static_cast<int>(LWJGL_KEYBOARD_STATE_SIZE); ++key) {
        output.key_state[static_cast<std::size_t>(key)] =
            bridge.key_down(key) ? 1u : 0u;
    }
    for (int button = 0; button < static_cast<int>(LWJGL_MOUSE_STATE_SIZE); ++button) {
        output.mouse_state[static_cast<std::size_t>(button)] =
            bridge.mouse_down(button) ? 1u : 0u;
    }

    output.keyboard_events.reserve(events.size() * LWJGL_KEYBOARD_EVENT_SIZE);
    output.mouse_events.reserve(events.size() * LWJGL_MOUSE_EVENT_SIZE);

    for (const PackedInputEvent& event : events) {
        switch (event.type) {
        case 1: // key down
        case 2: // key up
            if (event.code < 0 || event.code > 255) return false;
            write_keyboard_event(output.keyboard_events, event, timestamp_ns);
            break;
        case 3: // mouse button down
        case 4: // mouse button up
            if (event.code < 0 || event.code > 127) return false;
            write_mouse_event(output.mouse_events, event, timestamp_ns);
            break;
        case 5: // mouse move; LWJGL convention uses button = -1
            write_mouse_event(output.mouse_events, event, timestamp_ns);
            break;
        default:
            return false;
        }
    }

    return output.keyboard_events.size() % LWJGL_KEYBOARD_EVENT_SIZE == 0 &&
           output.mouse_events.size() % LWJGL_MOUSE_EVENT_SIZE == 0;
}

} // namespace opti
