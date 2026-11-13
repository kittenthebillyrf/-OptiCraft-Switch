#include "opticraft_native_registry.h"
#include "opticraft_switch_window_native.h"
#include <cstring>

namespace {
void create(OptiNativeCall *c) {
    if (!c || c->arg_count != 2 || !c->args) { if(c)c->exception_code=1; return; }
    c->result = opticraft_switch_window_create(static_cast<int32_t>(c->args[0]),
                                               static_cast<int32_t>(c->args[1])) ? 1u : 0u;
}
void preferred_size(OptiNativeCall *c) {
    if (!c || c->arg_count != 2 || !c->args) { if(c)c->exception_code=1; return; }
    opticraft_switch_window_set_preferred_size(static_cast<int32_t>(c->args[0]),
                                                static_cast<int32_t>(c->args[1]));
}
void pack(OptiNativeCall *c) { if(c && c->arg_count==0) opticraft_switch_window_pack(); else if(c)c->exception_code=1; }
void visible(OptiNativeCall *c) {
    if(!c || c->arg_count!=1 || !c->args){if(c)c->exception_code=1;return;}
    opticraft_switch_window_set_visible(c->args[0] ? 1 : 0);
}
void poll(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=opticraft_switch_window_poll()?1u:0u; else if(c)c->exception_code=1; }
void buttons(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=opticraft_switch_window_buttons(); else if(c)c->exception_code=1; }
void pressed(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=opticraft_switch_window_pressed(); else if(c)c->exception_code=1; }
void left_x(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=static_cast<uint64_t>(static_cast<int64_t>(opticraft_switch_window_left_x())); else if(c)c->exception_code=1; }
void left_y(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=static_cast<uint64_t>(static_cast<int64_t>(opticraft_switch_window_left_y())); else if(c)c->exception_code=1; }
void right_x(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=static_cast<uint64_t>(static_cast<int64_t>(opticraft_switch_window_right_x())); else if(c)c->exception_code=1; }
void right_y(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=static_cast<uint64_t>(static_cast<int64_t>(opticraft_switch_window_right_y())); else if(c)c->exception_code=1; }
void touch_x(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=static_cast<uint64_t>(static_cast<int64_t>(opticraft_switch_window_touch_x())); else if(c)c->exception_code=1; }
void touch_y(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=static_cast<uint64_t>(static_cast<int64_t>(opticraft_switch_window_touch_y())); else if(c)c->exception_code=1; }
void touch_down(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=opticraft_switch_window_touch_down()?1u:0u; else if(c)c->exception_code=1; }
void framebuffer(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=opticraft_switch_window_framebuffer_address(); else if(c)c->exception_code=1; }
void stride(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=static_cast<uint64_t>(opticraft_switch_window_framebuffer_stride()); else if(c)c->exception_code=1; }
void present(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=opticraft_switch_window_present()?1u:0u; else if(c)c->exception_code=1; }
void should_close(OptiNativeCall *c) { if(c && c->arg_count==0)c->result=opticraft_switch_window_should_close()?1u:0u; else if(c)c->exception_code=1; }
void destroy(OptiNativeCall *c) { if(c && c->arg_count==0)opticraft_switch_window_destroy(); else if(c)c->exception_code=1; }
#define ENTRY(name,desc,fn) {"org/lwjgl/switchport/SwitchWindow",name,desc,fn}
const OptiNativeMethod kMethods[] = {
 ENTRY("create","(II)Z",create),
 ENTRY("setPreferredSize","(II)V",preferred_size),
 ENTRY("pack","()V",pack),
 ENTRY("setVisible","(Z)V",visible),
 ENTRY("poll","()Z",poll),
 ENTRY("buttons","()J",buttons),
 ENTRY("pressed","()J",pressed),
 ENTRY("leftX","()I",left_x), ENTRY("leftY","()I",left_y),
 ENTRY("rightX","()I",right_x), ENTRY("rightY","()I",right_y),
 ENTRY("touchX","()I",touch_x), ENTRY("touchY","()I",touch_y),
 ENTRY("touchDown","()Z",touch_down),
 ENTRY("framebufferAddress","()J",framebuffer),
 ENTRY("framebufferStride","()I",stride),
 ENTRY("present","()Z",present),
 ENTRY("shouldClose","()Z",should_close),
 ENTRY("destroy","()V",destroy)
};
#undef ENTRY
}
extern "C" const OptiNativeMethod *opticraft_native_resolve(const char *c,const char *m,const char *d) {
    if(!c||!m||!d)return nullptr;
    for(const auto &entry:kMethods)
        if(std::strcmp(c,entry.class_name)==0 && std::strcmp(m,entry.method_name)==0 &&
           std::strcmp(d,entry.descriptor)==0) return &entry;
    return nullptr;
}
extern "C" size_t opticraft_native_method_count(void) { return sizeof(kMethods)/sizeof(kMethods[0]); }
extern "C" int opticraft_native_invoke(const OptiNativeMethod *method,OptiNativeCall *call) {
    if(!method||!method->handler||!call)return 0;
    call->exception_code=0;
    method->handler(call);
    return call->exception_code==0 ? 1 : 0;
}
