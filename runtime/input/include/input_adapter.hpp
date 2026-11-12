#pragma once
#include <cstdint>
namespace opti {
enum : int { KEY_ESCAPE=1,KEY_Q=16,KEY_W=17,KEY_E=18,KEY_A=30,KEY_S=31,KEY_D=32,KEY_UP=200,KEY_LEFT=203,KEY_RIGHT=205,KEY_DOWN=208 };
enum : int { MOUSE_LEFT=0,MOUSE_RIGHT=1 };
struct ControllerFrame { std::uint64_t buttons; std::int32_t left_x,left_y,right_x,right_y; };
struct InputEvent { enum Type { KeyDown,KeyUp,MouseDown,MouseUp } type; int code; };
struct EventBatch { InputEvent events[16]; unsigned count; };
class InputAdapter { public: EventBatch update(const ControllerFrame&); void reset(); private: std::uint32_t previous_keys_=0,previous_mouse_=0; };
}
