#include "world_save_preview.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <zlib.h>
static void u8(std::vector<unsigned char>& b,unsigned v){b.push_back(static_cast<unsigned char>(v));}
static void u16(std::vector<unsigned char>& b,unsigned v){u8(b,v>>8);u8(b,v);}
static void i32(std::vector<unsigned char>& b,unsigned v){u8(b,v>>24);u8(b,v>>16);u8(b,v>>8);u8(b,v);}
static void i64(std::vector<unsigned char>& b,unsigned long long v){for(int i=7;i>=0;--i)u8(b,static_cast<unsigned>(v>>(i*8)));}
static void str(std::vector<unsigned char>& b,const char* s){size_t n=std::strlen(s);u16(b,static_cast<unsigned>(n));b.insert(b.end(),s,s+n);}
static std::vector<unsigned char> fixture(){
 std::vector<unsigned char>b;u8(b,10);str(b,"");u8(b,10);str(b,"Data");
 u8(b,8);str(b,"LevelName");str(b,"My Java World");
 u8(b,4);str(b,"Time");i64(b,123456);
 u8(b,3);str(b,"SpawnX");i32(b,12);u8(b,3);str(b,"SpawnY");i32(b,70);u8(b,3);str(b,"SpawnZ");i32(b,34);
 u8(b,0);u8(b,0);return b;
}
static std::vector<unsigned char> gzip(const std::vector<unsigned char>& in){
 z_stream z{};assert(deflateInit2(&z,Z_DEFAULT_COMPRESSION,Z_DEFLATED,15+16,8,Z_DEFAULT_STRATEGY)==Z_OK);
 z.next_in=const_cast<Bytef*>(in.data());z.avail_in=static_cast<uInt>(in.size());
 std::vector<unsigned char> out(4096);z.next_out=out.data();z.avail_out=static_cast<uInt>(out.size());
 assert(deflate(&z,Z_FINISH)==Z_STREAM_END);out.resize(z.total_out);deflateEnd(&z);return out;
}
int main(){
 char root[]="/tmp/opticraft-preview-XXXXXX";assert(mkdtemp(root));
 char world[512],path[560];std::snprintf(world,sizeof(world),"%s/Test World",root);assert(mkdir(world,0700)==0);
 std::snprintf(path,sizeof(path),"%s/level.dat_old",world);auto bytes=gzip(fixture());
 FILE*f=std::fopen(path,"wb");assert(f);assert(std::fwrite(bytes.data(),1,bytes.size(),f)==bytes.size());std::fclose(f);
 OpticraftWorldPreview p{};assert(opticraft_world_save_read_preview(world,&p)==0);
 assert(p.has_level_name&&std::strcmp(p.level_name,"My Java World")==0);
 assert(p.has_time&&p.time==123456);assert(p.has_spawn&&p.spawn_x==12&&p.spawn_y==70&&p.spawn_z==34);
 std::puts("PASS gzip/NBT world metadata preview (level.dat_old fallback)");
 return 0;
}
