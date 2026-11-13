#include "world_anvil_chunk.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <zlib.h>
static void b(std::vector<unsigned char>&v,unsigned x){v.push_back(static_cast<unsigned char>(x));}
static void s(std::vector<unsigned char>&v,const char*x){size_t n=std::strlen(x);b(v,n>>8);b(v,n);v.insert(v.end(),x,x+n);}
static void i(std::vector<unsigned char>&v,unsigned x){b(v,x>>24);b(v,x>>16);b(v,x>>8);b(v,x);}
static std::vector<unsigned char> nbt(){
 std::vector<unsigned char>v;b(v,10);s(v,"");b(v,10);s(v,"Level");
 b(v,9);s(v,"Sections");b(v,10);i(v,1);
 b(v,1);s(v,"Y");b(v,0);
 b(v,7);s(v,"Blocks");i(v,4096);std::vector<unsigned char> blocks(4096,0);
 blocks[(4<<8)|(3<<4)|2]=2;blocks[(5<<8)|(3<<4)|2]=3;
 v.insert(v.end(),blocks.begin(),blocks.end());b(v,0);b(v,0);b(v,0);return v;
}
static std::vector<unsigned char> zlib_data(const std::vector<unsigned char>&in){
 uLongf n=compressBound(in.size());std::vector<unsigned char>out(n);assert(compress(out.data(),&n,in.data(),in.size())==Z_OK);out.resize(n);return out;
}
int main(){
 char root[]="/tmp/opticraft-anvil-XXXXXX";assert(mkdtemp(root));
 char region_dir[512];std::snprintf(region_dir,sizeof(region_dir),"%s/region",root);assert(mkdir(region_dir,0700)==0);
 char path[560];std::snprintf(path,sizeof(path),"%s/r.0.0.mca",region_dir);FILE*f=std::fopen(path,"wb");assert(f);
 std::vector<unsigned char> region(3*4096,0);region[2]=2;region[3]=1;
 auto compressed=zlib_data(nbt());unsigned length=static_cast<unsigned>(compressed.size()+1);
 const size_t off=2*4096;region[off]=length>>24;region[off+1]=length>>16;region[off+2]=length>>8;region[off+3]=length;region[off+4]=2;
 std::memcpy(region.data()+off+5,compressed.data(),compressed.size());
 assert(std::fwrite(region.data(),1,region.size(),f)==region.size());std::fclose(f);
 OpticraftAnvilChunkPreview out{};const int result=opticraft_world_read_spawn_chunk(root,0,0,&out);std::fprintf(stderr,"reader result=%d valid=%u block=%u height=%u\n",result,out.valid_columns,out.block_id[50],out.height[50]);assert(result==0);
 const int cell=3*16+2;assert(out.block_id[cell]==3&&out.height[cell]==5);assert(out.valid_columns==1);
 std::puts("PASS Anvil region header + zlib + legacy Sections/Blocks");
}
