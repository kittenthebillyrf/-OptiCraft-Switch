#include "opticraft_texture_atlas.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <zlib.h>
#include <unistd.h>
static void le16(std::vector<unsigned char>&v,unsigned x){v.push_back(x);v.push_back(x>>8);}
static void le32(std::vector<unsigned char>&v,unsigned x){v.push_back(x);v.push_back(x>>8);v.push_back(x>>16);v.push_back(x>>24);}
static void be32(std::vector<unsigned char>&v,unsigned x){v.push_back(x>>24);v.push_back(x>>16);v.push_back(x>>8);v.push_back(x);}
static void chunk(std::vector<unsigned char>&png,const char*name,const std::vector<unsigned char>&data){
 be32(png,data.size());size_t start=png.size();png.insert(png.end(),name,name+4);png.insert(png.end(),data.begin(),data.end());uLong crc=crc32(0,png.data()+start,static_cast<uInt>(4+data.size()));be32(png,crc);
}
static std::vector<unsigned char> make_png(){
 std::vector<unsigned char> png={137,80,78,71,13,10,26,10};std::vector<unsigned char> ihdr;
 be32(ihdr,1);be32(ihdr,1);ihdr.push_back(8);ihdr.push_back(6);ihdr.push_back(0);ihdr.push_back(0);ihdr.push_back(0);chunk(png,"IHDR",ihdr);
 unsigned char scan[]={0,12,34,56,255};uLongf n=compressBound(sizeof(scan));std::vector<unsigned char> idat(n);assert(compress(idat.data(),&n,scan,sizeof(scan))==Z_OK);idat.resize(n);chunk(png,"IDAT",idat);chunk(png,"IEND",{});return png;
}
static std::vector<unsigned char> make_jar(){
 auto png=make_png();std::vector<unsigned char> jar;const std::string name="terrain.png";const unsigned crc=crc32(0,png.data(),png.size());
 le32(jar,0x04034b50);le16(jar,20);le16(jar,0);le16(jar,0);le16(jar,0);le16(jar,0);le32(jar,crc);le32(jar,png.size());le32(jar,png.size());le16(jar,name.size());le16(jar,0);jar.insert(jar.end(),name.begin(),name.end());jar.insert(jar.end(),png.begin(),png.end());
 unsigned cd_offset=jar.size();le32(jar,0x02014b50);le16(jar,20);le16(jar,20);le16(jar,0);le16(jar,0);le16(jar,0);le16(jar,0);le32(jar,crc);le32(jar,png.size());le32(jar,png.size());le16(jar,name.size());le16(jar,0);le16(jar,0);le16(jar,0);le16(jar,0);le32(jar,0);le32(jar,0);jar.insert(jar.end(),name.begin(),name.end());
 unsigned cd_size=jar.size()-cd_offset;le32(jar,0x06054b50);le16(jar,0);le16(jar,0);le16(jar,1);le16(jar,1);le32(jar,cd_size);le32(jar,cd_offset);le16(jar,0);return jar;
}
int main(){
 char path[]="/tmp/opticraft-texture-XXXXXX";int fd=mkstemp(path);assert(fd>=0);auto jar=make_jar();assert(write(fd,jar.data(),jar.size())==static_cast<ssize_t>(jar.size()));close(fd);
 OpticraftTextureAtlas atlas{};assert(opticraft_texture_load_terrain_from_jar(path,&atlas)==0);
 assert(atlas.width==1&&atlas.height==1&&atlas.rgba[0]==12&&atlas.rgba[1]==34&&atlas.rgba[2]==56&&atlas.rgba[3]==255);
 opticraft_texture_atlas_free(&atlas);std::remove(path);std::puts("PASS ZIP terrain.png extraction + PNG RGBA decode");
}
