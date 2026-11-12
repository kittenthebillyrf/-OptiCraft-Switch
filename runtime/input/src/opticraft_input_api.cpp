#include "opticraft_input_api.h"
#include "switch_input_bridge.hpp"
#include "lwjgl_event_buffers.hpp"
#include "../../platform/include/opticraft_platform.h"
#include <algorithm>
#include <mutex>

namespace {
opti::SwitchInputBridge bridge;
std::mutex guard;

int convert(const OptiInputSnapshot& s,int w,int h,int32_t *out,int cap){
 if(w<=0||h<=0||!out||cap<0)return -1;
 opti::ControllerFrame f{s.buttons,s.left_x,s.left_y,s.right_x,s.right_y};
 bridge.update(f,w,h,s.touch_x,s.touch_y,s.touch_down!=0);
 auto events=bridge.drain();int needed=static_cast<int>(events.size()*4);if(needed>cap)return -2;
 for(std::size_t i=0;i<events.size();++i){out[i*4]=events[i].type;out[i*4+1]=events[i].code;out[i*4+2]=events[i].x;out[i*4+3]=events[i].y;}
 return needed;
}

int convert_lwjgl(const OptiInputSnapshot& s,int w,int h,OptiLwjglInputBuffers* out){
 if(w<=0||h<=0||!out||!out->keyboard_events||!out->mouse_events||
    !out->key_state||!out->mouse_state)return -1;
 constexpr size_t keyboard_max=OPTI_LWJGL_MAX_EVENTS_PER_FRAME*OPTI_LWJGL_KEYBOARD_EVENT_SIZE;
 constexpr size_t mouse_max=OPTI_LWJGL_MAX_EVENTS_PER_FRAME*OPTI_LWJGL_MOUSE_EVENT_SIZE;
 if(out->keyboard_capacity<keyboard_max||out->mouse_capacity<mouse_max||
    out->key_state_capacity<OPTI_LWJGL_KEYBOARD_STATE_SIZE||
    out->mouse_state_capacity<OPTI_LWJGL_MOUSE_STATE_SIZE)return -2;

 opti::ControllerFrame f{s.buttons,s.left_x,s.left_y,s.right_x,s.right_y};
 bridge.update(f,w,h,s.touch_x,s.touch_y,s.touch_down!=0);
 const auto events=bridge.drain();
 opti::LwjglEventBuffers encoded;
 if(!opti::serialize_lwjgl_event_buffers(events,opticraft_platform_ticks_ns(),bridge,encoded))return -3;
 if(encoded.keyboard_events.size()>out->keyboard_capacity||
    encoded.mouse_events.size()>out->mouse_capacity)return -4;

 std::copy(encoded.keyboard_events.begin(),encoded.keyboard_events.end(),out->keyboard_events);
 std::copy(encoded.mouse_events.begin(),encoded.mouse_events.end(),out->mouse_events);
 std::copy(encoded.key_state.begin(),encoded.key_state.end(),out->key_state);
 std::copy(encoded.mouse_state.begin(),encoded.mouse_state.end(),out->mouse_state);
 out->keyboard_bytes=encoded.keyboard_events.size();
 out->mouse_bytes=encoded.mouse_events.size();
 out->mouse_x=encoded.mouse_x;
 out->mouse_y=encoded.mouse_y;
 return 0;
}
}

extern "C" int opticraft_input_update_snapshot(uint64_t b,int32_t lx,int32_t ly,int32_t rx,int32_t ry,int32_t tx,int32_t ty,int touch,int w,int h,int32_t *out,int cap){
 std::lock_guard<std::mutex> lock(guard);OptiInputSnapshot s{};s.buttons=b;s.left_x=lx;s.left_y=ly;s.right_x=rx;s.right_y=ry;s.touch_x=tx;s.touch_y=ty;s.touch_down=touch?1:0;return convert(s,w,h,out,cap);
}
extern "C" int opticraft_input_poll(int w,int h,int32_t *out,int cap){
 OptiInputSnapshot s{};opticraft_platform_poll_events(&s);std::lock_guard<std::mutex> lock(guard);return convert(s,w,h,out,cap);
}
extern "C" int opticraft_input_update_lwjgl_snapshot(uint64_t b,int32_t lx,int32_t ly,int32_t rx,int32_t ry,int32_t tx,int32_t ty,int touch,int w,int h,OptiLwjglInputBuffers* out){
 std::lock_guard<std::mutex> lock(guard);OptiInputSnapshot s{};s.buttons=b;s.left_x=lx;s.left_y=ly;s.right_x=rx;s.right_y=ry;s.touch_x=tx;s.touch_y=ty;s.touch_down=touch?1:0;return convert_lwjgl(s,w,h,out);
}
extern "C" int opticraft_input_poll_lwjgl(int w,int h,OptiLwjglInputBuffers* out){
 OptiInputSnapshot s{};opticraft_platform_poll_events(&s);std::lock_guard<std::mutex> lock(guard);return convert_lwjgl(s,w,h,out);
}
extern "C" int opticraft_input_key_down(int k){std::lock_guard<std::mutex> lock(guard);return bridge.key_down(k)?1:0;}
extern "C" int opticraft_input_mouse_down(int b){std::lock_guard<std::mutex> lock(guard);return bridge.mouse_down(b)?1:0;}
extern "C" int opticraft_input_mouse_x(void){std::lock_guard<std::mutex> lock(guard);return bridge.mouse_x();}
extern "C" int opticraft_input_mouse_y(void){std::lock_guard<std::mutex> lock(guard);return bridge.mouse_y();}
extern "C" void opticraft_input_reset(void){std::lock_guard<std::mutex> lock(guard);bridge.reset();}
