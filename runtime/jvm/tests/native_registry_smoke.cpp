#include "opticraft_native_registry.h"
#include "opticraft_platform.h"
#include "opticraft_switch_window_native.h"
#include <cassert>
int main() {
    assert(opticraft_platform_init()==0);
    assert(opticraft_native_method_count()==19);
    auto *create=opticraft_native_resolve("org/lwjgl/switchport/SwitchWindow","create","(II)Z");
    assert(create);
    uint64_t dimensions[2]={640,360};
    OptiNativeCall call{}; call.args=dimensions; call.arg_count=2;
    assert(opticraft_native_invoke(create,&call)==1 && call.result==1);
    auto *pack=opticraft_native_resolve("org/lwjgl/switchport/SwitchWindow","pack","()V");
    assert(pack);
    call={}; assert(opticraft_native_invoke(pack,&call)==1);
    auto *stride=opticraft_native_resolve("org/lwjgl/switchport/SwitchWindow","framebufferStride","()I");
    assert(stride); call={}; assert(opticraft_native_invoke(stride,&call)==1 && call.result==640);
    assert(opticraft_native_resolve("org/lwjgl/switchport/SwitchWindow","pack","(I)V")==nullptr);
    auto *bad=opticraft_native_resolve("org/lwjgl/switchport/SwitchWindow","create","(II)Z");
    call={}; assert(opticraft_native_invoke(bad,&call)==1); /* no args -> exception code */
    assert(call.exception_code==1);
    opticraft_switch_window_destroy();
    opticraft_platform_shutdown();
    return 0;
}
