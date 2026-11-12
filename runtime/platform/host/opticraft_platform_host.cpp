#include "../include/opticraft_platform.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
namespace {
const auto start_time=std::chrono::steady_clock::now();
bool initialized=false;
bool safe_path(const char *p) {
 if (!p || !*p || p[0]=='/' || p[0]=='\\') return false;
 const std::string s(p);
 return s.find("..")==std::string::npos && s.find(':')==std::string::npos;
}
}
extern "C" uint32_t opticraft_platform_abi_version(void){return OPTI_PLATFORM_ABI_VERSION;}
extern "C" int opticraft_platform_init(void){initialized=true;return 0;}
extern "C" void opticraft_platform_shutdown(void){initialized=false;}
extern "C" void opticraft_platform_poll_events(OptiInputSnapshot *out){if(out)std::memset(out,0,sizeof(*out));}
extern "C" uint64_t opticraft_platform_ticks_ns(void){
 return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-start_time).count());
}
extern "C" int opticraft_platform_read_file(const char *path,void *dst,size_t cap,size_t *read){
 if(read)*read=0;
 if(!initialized||!safe_path(path)||(!dst&&cap))return -1;
 const std::string full=std::string("romfs/")+path;
 std::FILE *f=std::fopen(full.c_str(),"rb");if(!f)return -2;
 const size_t n=std::fread(dst,1,cap,f);const bool err=std::ferror(f)!=0;std::fclose(f);
 if (read) *read=n;
 return err ? -3 : 0;
}
extern "C" int opticraft_platform_present_rgba8888(const uint32_t *p,int w,int h,int stride){
 (void)p;(void)w;(void)h;(void)stride;return initialized?-10:-1;
}
