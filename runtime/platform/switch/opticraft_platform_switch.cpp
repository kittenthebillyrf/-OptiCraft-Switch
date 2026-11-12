#include "../include/opticraft_platform.h"
#include <switch.h>
#include <cstdio>
#include <cstring>
namespace { bool initialized=false,fb_ready=false;uint64_t previous_buttons=0;Framebuffer fb{};HidNpadFullKeyState pad{};constexpr int W=1280,H=720; }
extern "C" uint32_t opticraft_platform_abi_version(void){return OPTI_PLATFORM_ABI_VERSION;}
extern "C" int opticraft_platform_init(void){
 if(initialized)return 0;
 Result rc=hidInitialize();if(R_FAILED(rc))return -1;hidInitializeNpad();
 rc=framebufferCreate(&fb,nwindowGetDefault(),W,H,PIXEL_FORMAT_RGBA_8888,2);
 if(R_FAILED(rc)){hidExit();return -2;}
 rc=framebufferMakeLinear(&fb);if(R_FAILED(rc)){framebufferClose(&fb);hidExit();return -3;}
 fb_ready=true;initialized=true;previous_buttons=0;return 0;
}
extern "C" void opticraft_platform_shutdown(void){if(!initialized)return;if(fb_ready)framebufferClose(&fb);hidExit();fb_ready=false;initialized=false;}
extern "C" void opticraft_platform_poll_events(OptiInputSnapshot *out){
 if (!out) return;
 std::memset(out,0,sizeof(*out));
 if (!initialized) return;
 size_t count=hidGetNpadStatesHandheld(HidNpadIdType_Handheld,&pad,1);
 if (count==0) count=hidGetNpadStatesFullKey(HidNpadIdType_No1,&pad,1);
 if (count==0) return;
 uint64_t held=0;
 if(pad.buttons&HidNpadButton_A)held|=OPTI_BUTTON_A;
 if(pad.buttons&HidNpadButton_B)held|=OPTI_BUTTON_B;
 if(pad.buttons&HidNpadButton_X)held|=OPTI_BUTTON_X;
 if(pad.buttons&HidNpadButton_Y)held|=OPTI_BUTTON_Y;
 if(pad.buttons&HidNpadButton_L)held|=OPTI_BUTTON_L;
 if(pad.buttons&HidNpadButton_R)held|=OPTI_BUTTON_R;
 if(pad.buttons&HidNpadButton_Left)held|=OPTI_BUTTON_DPAD_LEFT;
 if(pad.buttons&HidNpadButton_Up)held|=OPTI_BUTTON_DPAD_UP;
 if(pad.buttons&HidNpadButton_Right)held|=OPTI_BUTTON_DPAD_RIGHT;
 if(pad.buttons&HidNpadButton_Down)held|=OPTI_BUTTON_DPAD_DOWN;
 if(pad.buttons&HidNpadButton_Plus)held|=OPTI_BUTTON_PLUS;
 out->buttons=held;out->pressed=held&~previous_buttons;out->released=previous_buttons&~held;previous_buttons=held;
 out->should_exit=(held&OPTI_BUTTON_PLUS)?1:0;
 out->left_x=pad.analog_stick_l.x; out->left_y=pad.analog_stick_l.y;
 out->right_x=pad.analog_stick_r.x; out->right_y=pad.analog_stick_r.y;
}
extern "C" uint64_t opticraft_platform_ticks_ns(void){return static_cast<uint64_t>(armGetSystemTick())*1000000000ull/static_cast<uint64_t>(armGetSystemTickFreq());}
extern "C" int opticraft_platform_read_file(const char *path,void *dst,size_t cap,size_t *read){
 if (read) *read=0;
 if (!initialized || !path || !*path || path[0]=='/' || std::strstr(path,"..") || (!dst && cap)) return -1;
 std::FILE *f=std::fopen(path,"rb");if(!f)return -2;const size_t n=std::fread(dst,1,cap,f);const bool err=std::ferror(f)!=0;std::fclose(f);
 if (read) *read=n;
 return err ? -3 : 0;
}
extern "C" int opticraft_platform_present_rgba8888(const uint32_t *pixels,int w,int h,int stride){
 if(!initialized||!fb_ready||!pixels||w!=W||h!=H||stride<w)return -1;
 uint32_t pitch=0;uint8_t *dst=static_cast<uint8_t*>(framebufferBegin(&fb,&pitch));if(!dst)return -2;
 for(int y=0;y<h;++y)std::memcpy(dst+y*pitch,pixels+y*stride,static_cast<size_t>(w)*4);
 framebufferEnd(&fb);return 0;
}
