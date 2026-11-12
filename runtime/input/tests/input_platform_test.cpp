#include "opticraft_input_api.h"
#include "../../platform/include/opticraft_platform.h"
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
 opticraft_input_reset();assert(!opticraft_input_mouse_down(0));opticraft_platform_shutdown();
}
