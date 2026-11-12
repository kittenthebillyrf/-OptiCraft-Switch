#include "opticraft_input_api.h"
#include "../../platform/include/opticraft_platform.h"
#include <array>
#include <cassert>
int main(){
 assert(opticraft_platform_init()==0);opticraft_input_reset();int32_t out[64]{};
 int n=opticraft_input_update_snapshot(OPTI_BUTTON_B,0,0,0,0,0,0,0,960,544,out,64);
 assert(n==4&&out[0]==1&&out[1]==1&&opticraft_input_key_down(1));
 assert(opticraft_input_update_snapshot(OPTI_BUTTON_B,0,0,0,0,0,0,0,960,544,out,64)==0);
 n=opticraft_input_update_snapshot(0,0,0,0,0,0,0,0,960,544,out,64);
 assert(n==4&&out[0]==2&&!opticraft_input_key_down(1));
 n=opticraft_input_update_snapshot(OPTI_BUTTON_R,0,0,0,0,0,0,0,960,544,out,64);
 assert(n==4&&out[0]==3&&out[1]==0&&opticraft_input_mouse_down(0));

 opticraft_input_reset();
 std::array<uint8_t,OPTI_LWJGL_MAX_EVENTS_PER_FRAME*OPTI_LWJGL_KEYBOARD_EVENT_SIZE> keyboard{};
 std::array<uint8_t,OPTI_LWJGL_MAX_EVENTS_PER_FRAME*OPTI_LWJGL_MOUSE_EVENT_SIZE> mouse{};
 std::array<uint8_t,OPTI_LWJGL_KEYBOARD_STATE_SIZE> keys{};
 std::array<uint8_t,OPTI_LWJGL_MOUSE_STATE_SIZE> buttons{};
 OptiLwjglInputBuffers buffers{keyboard.data(),keyboard.size(),0,mouse.data(),mouse.size(),0,keys.data(),keys.size(),buttons.data(),buttons.size(),0,0};
 assert(opticraft_input_update_lwjgl_snapshot(OPTI_BUTTON_B|OPTI_BUTTON_R,0,0,0,0,0,0,0,960,544,&buffers)==0);
 assert(buffers.keyboard_bytes==OPTI_LWJGL_KEYBOARD_EVENT_SIZE);
 assert(buffers.mouse_bytes==OPTI_LWJGL_MOUSE_EVENT_SIZE);
 assert(keyboard[3]==1&&keyboard[4]==1);
 assert(mouse[0]==0&&mouse[1]==1);
 assert(keys[1]==1&&buttons[0]==1);
 opticraft_input_reset();assert(!opticraft_input_mouse_down(0));opticraft_platform_shutdown();
}
