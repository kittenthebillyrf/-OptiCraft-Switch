#include "world_anvil_chunk.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <zlib.h>
namespace {
struct Cursor { const unsigned char* p; size_t n; size_t at; };
bool take(Cursor& c,size_t n,const unsigned char*& p){if(c.at>c.n||n>c.n-c.at)return false;p=c.p+c.at;c.at+=n;return true;}
bool u8(Cursor& c,uint8_t& v){const unsigned char* p=nullptr;if(!take(c,1,p))return false;v=p[0];return true;}
bool u16(Cursor& c,uint16_t& v){const unsigned char* p=nullptr;if(!take(c,2,p))return false;v=static_cast<uint16_t>((p[0]<<8)|p[1]);return true;}
bool i32(Cursor& c,int32_t& v){const unsigned char* p=nullptr;if(!take(c,4,p))return false;uint32_t x=(uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|p[3];v=static_cast<int32_t>(x);return true;}
bool str(Cursor& c,std::string& s){uint16_t n=0;const unsigned char* p=nullptr;if(!u16(c,n)||!take(c,n,p))return false;s.assign(reinterpret_cast<const char*>(p),n);return true;}
bool skip(Cursor& c,uint8_t t,unsigned depth){
 if(depth>64)return false;
 int32_t n=0;const unsigned char* p=nullptr;
 switch(t){
 case 1:return take(c,1,p);case 2:return take(c,2,p);case 3:case 5:return take(c,4,p);case 4:case 6:return take(c,8,p);
 case 7:if(!i32(c,n)||n<0||n>16*1024*1024)return false;return take(c,static_cast<size_t>(n),p);
 case 8:{std::string s;return str(c,s);}
 case 9:{uint8_t child=0;if(!u8(c,child)||!i32(c,n)||n<0||n>1000000)return false;if(child==0)return n==0;for(int32_t i=0;i<n;++i)if(!skip(c,child,depth+1))return false;return true;}
 case 10:for(;;){uint8_t child=0;if(!u8(c,child))return false;if(child==0)return true;std::string name;if(!str(c,name)||!skip(c,child,depth+1))return false;}
 case 11:case 12:if(!i32(c,n)||n<0||n>4000000)return false;return take(c,static_cast<size_t>(n)*(t==11?4u:8u),p);
 default:return false;
 }
}
struct Section {
 int y=-1; bool have_blocks=false,have_data=false,have_add=false;
 unsigned char blocks[4096]{};
 unsigned char data[2048]{};
 unsigned char add[2048]{};
};
uint8_t nibble(const unsigned char* values,int index){
 const uint8_t value=values[index>>1];
 return (index&1)?static_cast<uint8_t>(value>>4):static_cast<uint8_t>(value&0x0f);
}
bool parse_section(Cursor& c,OpticraftAnvilChunkPreview& out){
 Section s;
 for(;;){
  uint8_t t=0;if(!u8(c,t))return false;if(t==0)break;
  std::string name;if(!str(c,name))return false;
  if(name=="Y"&&t==1){
   uint8_t y=0;if(!u8(c,y))return false;s.y=y;
  }else if(name=="Blocks"&&t==7){
   int32_t n=0;const unsigned char* p=nullptr;
   if(!i32(c,n)||n!=4096||!take(c,4096,p))return false;
   std::memcpy(s.blocks,p,4096);s.have_blocks=true;
  }else if((name=="Data"||name=="Add")&&t==7){
   int32_t n=0;const unsigned char* p=nullptr;
   if(!i32(c,n)||n!=2048||!take(c,2048,p))return false;
   if(name=="Data"){std::memcpy(s.data,p,2048);s.have_data=true;}
   else{std::memcpy(s.add,p,2048);s.have_add=true;}
  }else if(!skip(c,t,0))return false;
 }
 if(s.y<0||s.y>15||!s.have_blocks)return true;
 for(int z=0;z<16;++z)for(int x=0;x<16;++x){
  const int column=z*16+x;
  for(int y=15;y>=0;--y){
   const int block_index=(y<<8)|(z<<4)|x;
   const uint16_t id=static_cast<uint16_t>(s.blocks[block_index])|
       static_cast<uint16_t>(s.have_add?nibble(s.add,block_index)<<8:0);
   if(id!=0){
    const int absolute=s.y*16+y;
    const uint8_t meta=s.have_data?nibble(s.data,block_index):0;
    int count=out.side_count[column];
    int insert=0;
    while(insert<count&&out.side_block_y[column][insert]>absolute)++insert;
    if(insert<8){
     if(count<8){++count;out.side_count[column]=static_cast<uint8_t>(count);}
     for(int k=count-1;k>insert;--k){
      out.side_block_id[column][k]=out.side_block_id[column][k-1];
      out.side_block_y[column][k]=out.side_block_y[column][k-1];
      out.side_metadata[column][k]=out.side_metadata[column][k-1];
     }
     out.side_block_id[column][insert]=id;
     out.side_block_y[column][insert]=static_cast<uint8_t>(absolute);
     out.side_metadata[column][insert]=meta;
    }
    if(absolute>=out.height[column]||out.block_id[column]==0){
     out.height[column]=static_cast<uint8_t>(absolute);
     out.block_id[column]=id;
     out.metadata[column]=meta;
    }
   }
  }
 }
 return true;
}
bool parse_sections(Cursor& c,int32_t count,OpticraftAnvilChunkPreview& out){if(count<0||count>16)return false;for(int32_t i=0;i<count;++i)if(!parse_section(c,out))return false;return true;}
bool parse_level(Cursor& c,OpticraftAnvilChunkPreview& out){
 for(;;){uint8_t t=0;if(!u8(c,t))return false;if(t==0)return true;std::string name;if(!str(c,name))return false;
  if(name=="Sections"&&t==9){uint8_t child=0;int32_t count=0;if(!u8(c,child)||child!=10||!i32(c,count)||!parse_sections(c,count,out))return false;}
  else if(!skip(c,t,0))return false;
 }
}
bool parse_chunk_nbt(const std::vector<unsigned char>& bytes,OpticraftAnvilChunkPreview& out){
 Cursor c{bytes.data(),bytes.size(),0};uint8_t root=0;if(!u8(c,root)||root!=10)return false;std::string root_name;if(!str(c,root_name))return false;
 for(;;){uint8_t t=0;if(!u8(c,t))return false;if(t==0)return true;std::string name;if(!str(c,name))return false;
  if(name=="Level"&&t==10){if(!parse_level(c,out))return false;}
  else if(!skip(c,t,0))return false;
 }
}
bool decompress(const unsigned char* data,size_t size,int mode,std::vector<unsigned char>& out){
 if(mode==3){out.assign(data,data+size);return true;}
 z_stream z{};z.next_in=const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data));z.avail_in=static_cast<uInt>(size);
 const int window=mode==1?15+32:15;if(inflateInit2(&z,window)!=Z_OK)return false;
 unsigned char buf[16384];int status=Z_OK;
 while(status==Z_OK){z.next_out=buf;z.avail_out=sizeof(buf);status=inflate(&z,Z_NO_FLUSH);size_t made=sizeof(buf)-z.avail_out;
  if(out.size()+made>8u*1024u*1024u){inflateEnd(&z);return false;}out.insert(out.end(),buf,buf+made);}
 inflateEnd(&z);return status==Z_STREAM_END;
}
int floor_div(int v,int d){int q=v/d,r=v%d;if(r<0)--q;return q;}
int floor_mod(int v,int d){int r=v%d;return r<0?r+d:r;}
bool read_exact(FILE* f,void* p,size_t n){return std::fread(p,1,n,f)==n;}
}
extern "C" int opticraft_world_read_spawn_chunk(const char* directory,int32_t spawn_x,int32_t spawn_z,OpticraftAnvilChunkPreview* output){
 if(!directory||!output)return -2;
 std::memset(output,0,sizeof(*output));
 const int cx=floor_div(spawn_x,16),cz=floor_div(spawn_z,16);
 const int rx=floor_div(cx,32),rz=floor_div(cz,32),lx=floor_mod(cx,32),lz=floor_mod(cz,32);
 output->chunk_x=cx;output->chunk_z=cz;
 char path[640];int n=std::snprintf(path,sizeof(path),"%s/region/r.%d.%d.mca",directory,rx,rz);
 if(n<0||static_cast<size_t>(n)>=sizeof(path))return -3;
 FILE* f=std::fopen(path,"rb");if(!f)return -4;
 const long entry=static_cast<long>(lz*32+lx)*4;
 if(std::fseek(f,entry,SEEK_SET)!=0){std::fclose(f);return -5;}
 unsigned char header[4];if(!read_exact(f,header,4)){std::fclose(f);return -5;}
 const uint32_t sector=(uint32_t(header[0])<<16)|(uint32_t(header[1])<<8)|header[2];
 const uint32_t sectors=header[3];if(sector<2||sectors==0){std::fclose(f);return -6;}
 if(std::fseek(f,static_cast<long>(sector)*4096,SEEK_SET)!=0){std::fclose(f);return -5;}
 unsigned char lenbuf[4];if(!read_exact(f,lenbuf,4)){std::fclose(f);return -5;}
 const uint32_t length=(uint32_t(lenbuf[0])<<24)|(uint32_t(lenbuf[1])<<16)|(uint32_t(lenbuf[2])<<8)|lenbuf[3];
 if(length<1||length>sectors*4096u-4u||length>1024u*1024u){std::fclose(f);return -6;}
 uint8_t compression=0;if(!read_exact(f,&compression,1)){std::fclose(f);return -5;}
 std::vector<unsigned char> compressed(length-1u);if(!read_exact(f,compressed.data(),compressed.size())){std::fclose(f);return -5;}std::fclose(f);
 std::vector<unsigned char> plain;if(compression<1||compression>3||!decompress(compressed.data(),compressed.size(),compression,plain))return -7;
 if(!parse_chunk_nbt(plain,*output))return -8;
 for(int i=0;i<256;++i)if(output->block_id[i])++output->valid_columns;
 return output->valid_columns?0:-9;
}
