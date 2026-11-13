#include "opticraft_texture_atlas.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <zlib.h>
namespace {
uint16_t le16(const unsigned char* p){return static_cast<uint16_t>(p[0]|(p[1]<<8));}
uint32_t le32(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
uint32_t be32(const unsigned char* p){return (uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|uint32_t(p[3]);}
bool read_file(const char* path,std::vector<unsigned char>& out){
 FILE* f=std::fopen(path,"rb");if(!f)return false;
 if(std::fseek(f,0,SEEK_END)!=0){std::fclose(f);return false;}
 long n=std::ftell(f);if(n<=0||n>64*1024*1024){std::fclose(f);return false;}
 if(std::fseek(f,0,SEEK_SET)!=0){std::fclose(f);return false;}
 out.resize(static_cast<size_t>(n));bool ok=std::fread(out.data(),1,out.size(),f)==out.size();std::fclose(f);return ok;
}
bool extract_zip_entry(const std::vector<unsigned char>& zip,const char* wanted,std::vector<unsigned char>& out){
 if(zip.size()<22)return false;
 const size_t search_start=zip.size()>65557?zip.size()-65557:0;
 size_t eocd=zip.size();
 for(size_t p=zip.size()-22;;--p){
  if(le32(zip.data()+p)==0x06054b50u){eocd=p;break;}
  if(p==search_start)break;
 }
 if(eocd==zip.size()||eocd+22>zip.size())return false;
 const uint16_t entries=le16(zip.data()+eocd+10);
 const uint32_t cd_size=le32(zip.data()+eocd+12),cd_offset=le32(zip.data()+eocd+16);
 if(uint64_t(cd_offset)+cd_size>zip.size())return false;
 size_t p=cd_offset;
 for(uint16_t i=0;i<entries;++i){
  if(p+46>zip.size()||le32(zip.data()+p)!=0x02014b50u)return false;
  const uint16_t method=le16(zip.data()+p+10),name_len=le16(zip.data()+p+28);
  const uint16_t extra_len=le16(zip.data()+p+30),comment_len=le16(zip.data()+p+32);
  const uint32_t crc=le32(zip.data()+p+16),compressed=le32(zip.data()+p+20),plain=le32(zip.data()+p+24);
  const uint32_t local=le32(zip.data()+p+42);
  if(p+46u+name_len+extra_len+comment_len>zip.size())return false;
  const std::string name(reinterpret_cast<const char*>(zip.data()+p+46),name_len);
  if(name==wanted){
   if(local+30u>zip.size()||le32(zip.data()+local)!=0x04034b50u)return false;
   const uint16_t local_name=le16(zip.data()+local+26),local_extra=le16(zip.data()+local+28);
   const size_t data_offset=static_cast<size_t>(local)+30u+local_name+local_extra;
   if(data_offset+compressed>zip.size()||plain>8u*1024u*1024u)return false;
   out.resize(plain);
   if(method==0){if(compressed!=plain)return false;std::memcpy(out.data(),zip.data()+data_offset,plain);}
   else if(method==8){
    z_stream z{};z.next_in=const_cast<Bytef*>(reinterpret_cast<const Bytef*>(zip.data()+data_offset));
    z.avail_in=compressed;z.next_out=reinterpret_cast<Bytef*>(out.data());z.avail_out=plain;
    if(inflateInit2(&z,-MAX_WBITS)!=Z_OK)return false;
    int status=inflate(&z,Z_FINISH);inflateEnd(&z);
    if(status!=Z_STREAM_END||z.total_out!=plain)return false;
   }else return false;
   return crc32(0,out.data(),static_cast<uInt>(out.size()))==crc;
  }
  p+=46u+name_len+extra_len+comment_len;
 }
 return false;
}
int paeth(int a,int b,int c){int p=a+b-c,pa=std::abs(p-a),pb=std::abs(p-b),pc=std::abs(p-c);return pa<=pb&&pa<=pc?a:(pb<=pc?b:c);}
bool decode_png(const std::vector<unsigned char>& png,OpticraftTextureAtlas& out){
 static const unsigned char signature[8]={137,80,78,71,13,10,26,10};
 if(png.size()<8||std::memcmp(png.data(),signature,8)!=0)return false;
 uint32_t width=0,height=0;uint8_t depth=0,type=0,interlace=0;bool have_header=false,have_end=false;
 std::vector<unsigned char> compressed;
 size_t p=8;
 while(p+12<=png.size()){
  uint32_t length=be32(png.data()+p);if(uint64_t(p)+12+length>png.size())return false;
  const unsigned char* tag=png.data()+p+4;const unsigned char* data=png.data()+p+8;
  if(std::memcmp(tag,"IHDR",4)==0){
   if(length!=13)return false;width=be32(data);height=be32(data+4);depth=data[8];type=data[9];interlace=data[12];have_header=true;
  }else if(std::memcmp(tag,"IDAT",4)==0){
   if(compressed.size()+length>8u*1024u*1024u)return false;
   compressed.insert(compressed.end(),data,data+length);
  }else if(std::memcmp(tag,"IEND",4)==0){have_end=true;break;}
  p+=12u+length;
 }
 if(!have_header||!have_end||width==0||height==0||width>1024||height>1024||depth!=8||interlace!=0||(type!=2&&type!=6))return false;
 const uint32_t bpp=type==6?4u:3u,row_bytes=width*bpp;
 const uint64_t expected=uint64_t(row_bytes+1)*height;if(expected>16u*1024u*1024u)return false;
 std::vector<unsigned char> raw(static_cast<size_t>(expected));uLongf raw_size=static_cast<uLongf>(raw.size());
 if(uncompress(raw.data(),&raw_size,compressed.data(),static_cast<uLong>(compressed.size()))!=Z_OK||raw_size!=raw.size())return false;
 std::vector<unsigned char> decoded(static_cast<size_t>(row_bytes)*height);
 for(uint32_t y=0;y<height;++y){
  const unsigned char* src=raw.data()+static_cast<size_t>(y)*(row_bytes+1)+1;
  unsigned char* dst=decoded.data()+static_cast<size_t>(y)*row_bytes;
  const unsigned char* prev=y?decoded.data()+static_cast<size_t>(y-1)*row_bytes:nullptr;
  const uint8_t filter=raw[static_cast<size_t>(y)*(row_bytes+1)];
  if(filter>4)return false;
  for(uint32_t x=0;x<row_bytes;++x){
   const int a=x>=bpp?dst[x-bpp]:0,b=prev?prev[x]:0,c=(prev&&x>=bpp)?prev[x-bpp]:0;
   int value=src[x];
   if(filter==1)value+=a;else if(filter==2)value+=b;else if(filter==3)value+=(a+b)/2;else if(filter==4)value+=paeth(a,b,c);
   dst[x]=static_cast<unsigned char>(value);
  }
 }
 out.rgba=static_cast<uint8_t*>(std::malloc(static_cast<size_t>(width)*height*4));
 if(!out.rgba)return false;
 for(size_t i=0,pixel=0;i<decoded.size();i+=bpp,pixel+=4){
  out.rgba[pixel]=decoded[i];out.rgba[pixel+1]=decoded[i+1];out.rgba[pixel+2]=decoded[i+2];out.rgba[pixel+3]=bpp==4?decoded[i+3]:255;
 }
 out.width=width;out.height=height;return true;
}
}
extern "C" int opticraft_texture_load_terrain_from_jar(const char* jar_path,OpticraftTextureAtlas* output){
 if(!jar_path||!output)return -2;output->width=output->height=0;output->rgba=nullptr;
 std::vector<unsigned char> jar,png;if(!read_file(jar_path,jar))return -1;
 if(!extract_zip_entry(jar,"terrain.png",png))return -3;
 if(!decode_png(png,*output)){opticraft_texture_atlas_free(output);return -4;}
 return 0;
}
extern "C" void opticraft_texture_atlas_free(OpticraftTextureAtlas* atlas){
 if(!atlas)return;std::free(atlas->rgba);atlas->rgba=nullptr;atlas->width=atlas->height=0;
}
