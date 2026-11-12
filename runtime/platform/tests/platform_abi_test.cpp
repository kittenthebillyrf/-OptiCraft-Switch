#include "../include/opticraft_platform.h"
#include <cassert>
int main(){
 assert(opticraft_platform_abi_version()==1u);assert(opticraft_platform_init()==0);
 const auto t0=opticraft_platform_ticks_ns();OptiInputSnapshot s{};opticraft_platform_poll_events(&s);
 assert(s.buttons==0&&s.pressed==0&&s.released==0);assert(opticraft_platform_ticks_ns()>=t0);
 char buf[8]{};size_t n=7;assert(opticraft_platform_read_file("../escape",buf,sizeof(buf),&n)==-1&&n==0);
 const uint32_t px=0;assert(opticraft_platform_present_rgba8888(&px,1,1,1)==-10);
 opticraft_platform_shutdown();assert(opticraft_platform_present_rgba8888(&px,1,1,1)==-1);
}
