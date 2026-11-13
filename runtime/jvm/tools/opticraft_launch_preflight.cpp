#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <zlib.h>

namespace {
uint16_t le16(const uint8_t *p) { return uint16_t(p[0]) | (uint16_t(p[1]) << 8); }
uint32_t le32(const uint8_t *p) { return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24); }
uint16_t be16(const uint8_t *p) { return (uint16_t(p[0]) << 8) | uint16_t(p[1]); }
uint32_t be32(const uint8_t *p) { return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]); }
bool range(size_t at, size_t len, size_t total) { return at <= total && len <= total - at; }
struct Entry { std::string name; uint32_t crc=0, packed=0, unpacked=0, offset=0; uint16_t method=0, flags=0; };

bool find_entry(const std::vector<uint8_t>& zip, const std::string& wanted, Entry& out, std::string& error) {
    if (zip.size() < 22) { error="JAR too small"; return false; }
    const size_t start=zip.size()>65557?zip.size()-65557:0;
    size_t eocd=zip.size();
    for(size_t p=zip.size()-22;;--p) {
        if(le32(&zip[p])==0x06054b50u && range(p,22u+le16(&zip[p+20]),zip.size()) &&
           p+22u+le16(&zip[p+20])==zip.size()) { eocd=p; break; }
        if(p==start) break;
    }
    if(eocd==zip.size()) { error="ZIP directory terminator not found"; return false; }
    const uint16_t disk=le16(&zip[eocd+4]), cd_disk=le16(&zip[eocd+6]);
    const uint16_t count=le16(&zip[eocd+10]);
    const uint32_t cd_size=le32(&zip[eocd+12]), cd_off=le32(&zip[eocd+16]);
    if(disk||cd_disk||count==0xffff||cd_size==0xffffffffu||cd_off==0xffffffffu||
       !range(cd_off,cd_size,zip.size())||size_t(cd_off)+cd_size>eocd) {
        error="multi-disk, ZIP64 or malformed JAR is unsupported"; return false;
    }
    size_t p=cd_off;
    for(uint32_t i=0;i<count;i++) {
        if(!range(p,46,zip.size())||le32(&zip[p])!=0x02014b50u) { error="bad central directory"; return false; }
        Entry e; e.flags=le16(&zip[p+8]); e.method=le16(&zip[p+10]); e.crc=le32(&zip[p+16]);
        e.packed=le32(&zip[p+20]); e.unpacked=le32(&zip[p+24]); e.offset=le32(&zip[p+42]);
        const uint16_t nl=le16(&zip[p+28]), el=le16(&zip[p+30]), cl=le16(&zip[p+32]);
        const size_t total=46u+nl+el+cl;
        if(!range(p,total,zip.size())) { error="truncated central directory entry"; return false; }
        e.name.assign(reinterpret_cast<const char*>(&zip[p+46]),nl);
        if(e.name==wanted) { out=e; return true; }
        p+=total;
    }
    error="Minecraft main class entry missing"; return false;
}
bool extract(const std::vector<uint8_t>& zip,const Entry& e,std::vector<uint8_t>& out,std::string& error) {
    if(e.flags&1u) { error="encrypted class entry"; return false; }
    if(e.method!=0&&e.method!=8) { error="unsupported ZIP compression"; return false; }
    if(!range(e.offset,30,zip.size())||le32(&zip[e.offset])!=0x04034b50u) { error="invalid local header"; return false; }
    const size_t data=size_t(e.offset)+30u+le16(&zip[e.offset+26])+le16(&zip[e.offset+28]);
    if(!range(data,e.packed,zip.size())) { error="class payload outside JAR"; return false; }
    out.resize(e.unpacked);
    if(e.method==0) {
        if(e.packed!=e.unpacked) { error="stored entry size mismatch"; return false; }
        std::copy(zip.begin()+data,zip.begin()+data+e.packed,out.begin());
    } else {
        z_stream s{}; s.next_in=const_cast<Bytef*>(reinterpret_cast<const Bytef*>(&zip[data]));
        s.avail_in=e.packed; uint8_t empty=0;
        s.next_out=out.empty()?&empty:reinterpret_cast<Bytef*>(out.data());
        s.avail_out=out.empty()?1u:static_cast<uInt>(out.size());
        if(inflateInit2(&s,-MAX_WBITS)!=Z_OK) { error="inflate init failed"; return false; }
        const int rc=inflate(&s,Z_FINISH);
        const bool ok=rc==Z_STREAM_END&&s.total_in==e.packed&&s.total_out==e.unpacked;
        inflateEnd(&s); if(!ok) { error="DEFLATE stream invalid"; return false; }
    }
    const uint32_t crc=static_cast<uint32_t>(::crc32(0L,out.empty()?Z_NULL:reinterpret_cast<const Bytef*>(out.data()),static_cast<uInt>(out.size())));
    if(crc!=e.crc) { error="class CRC mismatch"; return false; }
    return true;
}
struct Reader {
    const std::vector<uint8_t>& b; size_t p=0;
    bool take(size_t n) { if(!range(p,n,b.size())) return false; p+=n; return true; }
    bool u1(uint8_t& v) { if(!take(1)) return false; v=b[p-1]; return true; }
    bool u2(uint16_t& v) { if(!take(2)) return false; v=be16(&b[p-2]); return true; }
    bool u4(uint32_t& v) { if(!take(4)) return false; v=be32(&b[p-4]); return true; }
};
bool skip_attributes(Reader& r,uint16_t count) {
    for(uint32_t i=0;i<count;i++) { uint16_t name; uint32_t len; if(!r.u2(name)||!r.u4(len)||!r.take(len)) return false; }
    return true;
}
bool inspect_main(const std::vector<uint8_t>& c,size_t& code_size,std::string& error) {
    if(c.size()<10||be32(c.data())!=0xcafebabeu) { error="invalid classfile header"; return false; }
    Reader r{c}; uint32_t magic; uint16_t minor,major,cp_count;
    if(!r.u4(magic)||!r.u2(minor)||!r.u2(major)||!r.u2(cp_count)||major!=49||cp_count<1) {
        error="expected Java 5 classfile (major 49)"; return false;
    }
    (void)minor;\n    std::vector<std::string> utf(cp_count); std::vector<uint8_t> tags(cp_count);
    for(uint32_t i=1;i<cp_count;i++) {
        uint8_t tag; if(!r.u1(tag)) { error="truncated constant pool"; return false; } tags[i]=tag;
        uint16_t a,b; uint32_t x;
        switch(tag) {
        case 1: if(!r.u2(a)||!range(r.p,a,c.size())) { error="bad UTF8 constant"; return false; }
                utf[i].assign(reinterpret_cast<const char*>(&c[r.p]),a); if(!r.take(a)) return false; break;
        case 3: case 4: if(!r.u4(x)) return false; break;
        case 5: case 6: if(!r.take(8)||++i>=cp_count) { error="bad wide constant"; return false; } break;
        case 7: case 8: case 16: if(!r.u2(a)) return false; break;
        case 9: case 10: case 11: case 12: case 18: if(!r.u2(a)||!r.u2(b)) return false; break;
        case 15: { uint8_t k; if(!r.u1(k)||!r.u2(a)) return false; break; }
        default: error="unsupported constant pool tag"; return false;
        }
    }
    uint16_t access,this_class,super_class,interfaces,fields,methods;
    if(!r.u2(access)||!r.u2(this_class)||!r.u2(super_class)||!r.u2(interfaces)||!r.take(size_t(interfaces)*2)||!r.u2(fields)) {
        error="truncated class declaration"; return false;
    }
    (void)access; (void)this_class; (void)super_class;\n    for(uint32_t i=0;i<fields;i++) { uint16_t a,n,d,ac; if(!r.u2(a)||!r.u2(n)||!r.u2(d)||!r.u2(ac)||!skip_attributes(r,ac)) { error="bad fields"; return false; } }
    if(!r.u2(methods)) { error="missing methods"; return false; }
    bool found=false;
    for(uint32_t i=0;i<methods;i++) {
        uint16_t flags,name,desc,attrs; if(!r.u2(flags)||!r.u2(name)||!r.u2(desc)||!r.u2(attrs)) { error="truncated method"; return false; }
        (void)flags;\n        for(uint32_t j=0;j<attrs;j++) {
            uint16_t an; uint32_t len; if(!r.u2(an)||!r.u4(len)||!range(r.p,len,c.size())) { error="bad method attribute"; return false; }
            const std::string attr=(an<utf.size()?utf[an]:std::string());
            if(name<utf.size()&&desc<utf.size()&&utf[name]=="main"&&utf[desc]=="([Ljava/lang/String;)V"&&attr=="Code") {
                Reader cr{c}; cr.p=r.p; uint16_t ms,ml,ex,ac; uint32_t clen;
                if(!cr.u2(ms)||!cr.u2(ml)||!cr.u4(clen)||clen>65535||!cr.take(clen)) { error="invalid main Code attribute"; return false; }\n                (void)ms; (void)ml;
                code_size=clen; found=true;
            }
            if(!r.take(len)) return false;
        }
    }
    uint16_t class_attrs; if(!r.u2(class_attrs)||!skip_attributes(r,class_attrs)||r.p!=c.size()) { error="malformed class tail"; return false; }
    if(!found) { error="public static main(String[]) method Code not found"; return false; }
    return true;
}
}
int main(int argc,char** argv) {
    if(argc!=2) { std::cerr<<"usage: opticraft_launch_preflight <minecraft.jar>\n"; return 2; }
    std::ifstream f(argv[1],std::ios::binary);
    if(!f) { std::cerr<<"cannot open JAR: "<<argv[1]<<"\n"; return 2; }
    std::vector<uint8_t> zip((std::istreambuf_iterator<char>(f)),{});
    Entry entry; std::string error;
    if(!find_entry(zip,"net/minecraft/client/Minecraft.class",entry,error)) { std::cerr<<"JAR FAIL: "<<error<<"\n"; return 1; }
    std::vector<uint8_t> cls;
    if(!extract(zip,entry,cls,error)) { std::cerr<<"EXTRACT FAIL: "<<error<<"\n"; return 1; }
    size_t code_size=0;
    if(!inspect_main(cls,code_size,error)) { std::cerr<<"CLASS FAIL: "<<error<<"\n"; return 1; }
    std::cout<<"JAR_OK entries_class=net/minecraft/client/Minecraft.class bytes="<<cls.size()<<" crc=verified\n";
    std::cout<<"CLASS_OK version=Java5 main=([Ljava/lang/String;)V code_bytes="<<code_size<<"\n";
    std::cout<<"PREFLIGHT_OK (class located and parsed; bytecode execution is not performed)\n";
    return 0;
}
