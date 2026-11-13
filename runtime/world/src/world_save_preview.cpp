#include "world_save_preview.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <zlib.h>

namespace {
struct Cursor { const unsigned char* p; size_t n, at; };
bool take(Cursor& c, size_t n, const unsigned char*& out) {
    if (n > c.n - c.at) return false;
    out = c.p + c.at; c.at += n; return true;
}
bool u8(Cursor& c, uint8_t& v) { const unsigned char* p; if(!take(c,1,p))return false;v=p[0];return true; }
bool u16(Cursor& c, uint16_t& v) { const unsigned char* p;if(!take(c,2,p))return false;v=(uint16_t(p[0])<<8)|p[1];return true; }
bool i32(Cursor& c, int32_t& v) { const unsigned char* p;if(!take(c,4,p))return false;uint32_t x=(uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|p[3];v=static_cast<int32_t>(x);return true; }
bool i64(Cursor& c, int64_t& v) { const unsigned char* p;if(!take(c,8,p))return false;uint64_t x=0;for(int i=0;i<8;++i)x=(x<<8)|p[i];v=static_cast<int64_t>(x);return true; }
bool string_value(Cursor& c, std::string& s) { uint16_t n;const unsigned char* p;if(!u16(c,n)||!take(c,n,p))return false;s.assign(reinterpret_cast<const char*>(p),n);return true; }

bool skip_payload(Cursor& c, uint8_t type, unsigned depth) {
    if(depth>64)return false;
    int32_t i;const unsigned char* p;
    switch(type){
        case 1:return take(c,1,p);
        case 2:return take(c,2,p);
        case 3:return take(c,4,p);
        case 4:return take(c,8,p);
        case 5:return take(c,4,p);
        case 6:return take(c,8,p);
        case 7:if(!i32(c,i)||i<0||i>16*1024*1024)return false;return take(c,static_cast<size_t>(i),p);
        case 8:{std::string value;return string_value(c,value);}
        case 9:{
            uint8_t child;int32_t count;if(!u8(c,child)||!i32(c,count)||count<0||count>1000000)return false;
            if(child==0)return count==0;
            for(int32_t k=0;k<count;++k)if(!skip_payload(c,child,depth+1))return false;
            return true;
        }
        case 10:{
            for(;;){uint8_t child;if(!u8(c,child))return false;if(child==0)return true;
                std::string name;if(!string_value(c,name)||!skip_payload(c,child,depth+1))return false;}
        }
        case 11:case 12:if(!i32(c,i)||i<0||i>4000000)return false;return take(c,static_cast<size_t>(i)*(type==11?4u:8u),p);
        default:return false;
    }
}
bool parse_data_compound(Cursor& c, OpticraftWorldPreview& out) {
    for(;;){
        uint8_t type;if(!u8(c,type))return false;if(type==0)return true;
        std::string name;if(!string_value(c,name))return false;
        if(name=="LevelName"&&type==8){
            std::string value;if(!string_value(c,value))return false;
            size_t n=value.size();if(n>=sizeof(out.level_name))n=sizeof(out.level_name)-1;
            std::memcpy(out.level_name,value.data(),n);out.level_name[n]='\\0';out.has_level_name=1;
        } else if(name=="Time"&&type==4){
            if(!i64(c,out.time))return false;out.has_time=1;
        } else if((name=="SpawnX"||name=="SpawnY"||name=="SpawnZ")&&type==3){
            int32_t value;if(!i32(c,value))return false;
            if(name=="SpawnX")out.spawn_x=value;else if(name=="SpawnY")out.spawn_y=value;else out.spawn_z=value;
            out.has_spawn=1;
        } else if(!skip_payload(c,type,0))return false;
    }
}
bool parse_nbt(const std::vector<unsigned char>& bytes, OpticraftWorldPreview& out) {
    Cursor c{bytes.data(),bytes.size(),0};uint8_t root;if(!u8(c,root)||root!=10)return false;
    std::string root_name;if(!string_value(c,root_name))return false;
    for(;;){uint8_t type;if(!u8(c,type))return false;if(type==0)return out.has_level_name!=0;
        std::string name;if(!string_value(c,name))return false;
        if(name=="Data"&&type==10){if(!parse_data_compound(c,out))return false;}
        else if(!skip_payload(c,type,0))return false;
    }
}
bool read_file(const char* path,std::vector<unsigned char>& bytes) {
    FILE* f=std::fopen(path,"rb");if(!f)return false;
    if(std::fseek(f,0,SEEK_END)!=0){std::fclose(f);return false;}
    long n=std::ftell(f);if(n<=0||n>2*1024*1024){std::fclose(f);return false;}
    if(std::fseek(f,0,SEEK_SET)!=0){std::fclose(f);return false;}
    bytes.resize(static_cast<size_t>(n));bool ok=std::fread(bytes.data(),1,bytes.size(),f)==bytes.size();
    std::fclose(f);return ok;
}
bool gunzip(const std::vector<unsigned char>& input,std::vector<unsigned char>& output) {
    z_stream z{};z.next_in=const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data()));
    z.avail_in=static_cast<uInt>(input.size());
    if(inflateInit2(&z,15+32)!=Z_OK)return false;
    unsigned char chunk[16384];int status=Z_OK;
    while(status==Z_OK){
        z.next_out=chunk;z.avail_out=sizeof(chunk);status=inflate(&z,Z_NO_FLUSH);
        size_t produced=sizeof(chunk)-z.avail_out;
        if(output.size()+produced>16u*1024u*1024u){inflateEnd(&z);return false;}
        output.insert(output.end(),chunk,chunk+produced);
    }
    inflateEnd(&z);return status==Z_STREAM_END;
}
} // namespace

extern "C" int opticraft_world_save_read_preview(const char* world_directory, OpticraftWorldPreview* output) {
    if(!world_directory||!output)return -2;
    std::memset(output,0,sizeof(*output));
    const char* names[]={"level.dat","level.dat_old"};
    for(const char* name:names){
        char path[512];int n=std::snprintf(path,sizeof(path),"%s/%s",world_directory,name);
        if(n<0||static_cast<size_t>(n)>=sizeof(path))return -3;
        std::vector<unsigned char> compressed,plain;
        if(!read_file(path,compressed))continue;
        if(!gunzip(compressed,plain))continue;
        if(parse_nbt(plain,*output))return 0;
        std::memset(output,0,sizeof(*output));
    }
    return -1;
}
