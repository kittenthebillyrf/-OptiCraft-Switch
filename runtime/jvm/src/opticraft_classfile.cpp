#include "opticraft_classfile.h"
#include <algorithm>
#include <new>
#include <string>
#include <vector>

namespace {
bool range_ok(size_t at,size_t n,size_t size){return at<=size&&n<=size-at;}
struct Reader {
 const uint8_t *b; size_t n,p=0;
 bool take(size_t k){if(!range_ok(p,k,n))return false;p+=k;return true;}
 bool u1(uint8_t &v){if(!take(1))return false;v=b[p-1];return true;}
 bool u2(uint16_t &v){if(!take(2))return false;v=uint16_t(b[p-2]<<8)|b[p-1];return true;}
 bool u4(uint32_t &v){if(!take(4))return false;v=(uint32_t(b[p-4])<<24)|(uint32_t(b[p-3])<<16)|(uint32_t(b[p-2])<<8)|b[p-1];return true;}
};
struct Cp { uint8_t tag=0; uint16_t a=0,b=0; std::string text; };
struct Method { uint16_t access=0,name=0,desc=0; uint32_t code=0; };
bool skip_attrs(Reader &r,uint16_t count){
 for(uint32_t i=0;i<count;i++){uint16_t name;uint32_t len;if(!r.u2(name)||!r.u4(len)||!r.take(len))return false;}return true;
}
}
struct OptiClassFile {
 OptiClassSummary s{}; std::vector<Cp> cp; std::vector<Method> methods;
};
extern "C" int opticraft_classfile_parse(const uint8_t *bytes,size_t length,OptiClassFile **out){
 if(!bytes||!out)return 0;*out=nullptr; Reader r{bytes,length};
 uint32_t magic;uint16_t minor,major,count;
 if(!r.u4(magic)||magic!=0xcafebabeu||!r.u2(minor)||!r.u2(major)||!r.u2(count)||!count)return 0;
 OptiClassFile *f=new(std::nothrow) OptiClassFile();if(!f)return 0;
 f->cp.resize(count);
 for(uint32_t i=1;i<count;i++){
  uint8_t tag;if(!r.u1(tag)){delete f;return 0;}Cp &c=f->cp[i];c.tag=tag;uint32_t v;
  switch(tag){
   case 1:{uint16_t n;if(!r.u2(n)||!range_ok(r.p,n,r.n)){delete f;return 0;}
    c.text.assign(reinterpret_cast<const char*>(r.b+r.p),n);r.take(n);break;}
   case 3:case 4:if(!r.u4(v)){delete f;return 0;}c.a=uint16_t(v>>16);c.b=uint16_t(v);break;
   case 5:case 6:if(!r.take(8)||++i>=count){delete f;return 0;}break;
   case 7:case 8:case 16:case 19:case 20:if(!r.u2(c.a)){delete f;return 0;}break;
   case 9:case 10:case 11:case 12:case 17:case 18:if(!r.u2(c.a)||!r.u2(c.b)){delete f;return 0;}break;
   case 15:{uint8_t kind;if(!r.u1(kind)||!r.u2(c.a)){delete f;return 0;}c.b=kind;break;}
   default:delete f;return 0;
  }
 }
 uint16_t access,thisc,superc,interfaces,fields;
 if(!r.u2(access)||!r.u2(thisc)||!r.u2(superc)||!r.u2(interfaces)||!r.take(size_t(interfaces)*2)||!r.u2(fields)){delete f;return 0;}
 for(uint32_t i=0;i<fields;i++){uint16_t a,n,d,ac;if(!r.u2(a)||!r.u2(n)||!r.u2(d)||!r.u2(ac)||!skip_attrs(r,ac)){delete f;return 0;}}
 uint16_t methods;if(!r.u2(methods)){delete f;return 0;}f->methods.reserve(methods);
 for(uint32_t i=0;i<methods;i++){
  Method m{};uint16_t ac;if(!r.u2(m.access)||!r.u2(m.name)||!r.u2(m.desc)||!r.u2(ac)){delete f;return 0;}
  for(uint32_t j=0;j<ac;j++){
   uint16_t an;uint32_t len;if(!r.u2(an)||!r.u4(len)||!range_ok(r.p,len,r.n)){delete f;return 0;}
   const std::string attr=(an<count&&f->cp[an].tag==1)?f->cp[an].text:"";
   if(attr=="Code"){
    Reader cr{r.b,r.n,r.p};uint16_t ms,ml;uint32_t code;
    if(!cr.u2(ms)||!cr.u2(ml)||!cr.u4(code)||!range_ok(cr.p,code,cr.n)||code>len){delete f;return 0;}
    m.code=code;
   }
   if(!r.take(len)){delete f;return 0;}
  }
  f->methods.push_back(m);
 }
 uint16_t attrs;if(!r.u2(attrs)||!skip_attrs(r,attrs)||r.p!=length){delete f;return 0;}
 f->s={minor,major,access,count,fields,methods,thisc,superc};*out=f;return 1;
}
extern "C" void opticraft_classfile_destroy(OptiClassFile *f){delete f;}
extern "C" int opticraft_classfile_summary(const OptiClassFile *f,OptiClassSummary *out){if(!f||!out)return 0;*out=f->s;return 1;}
extern "C" int opticraft_classfile_utf8(const OptiClassFile *f,uint16_t i,const char **out,uint16_t *len){
 if(!f||!out||!len||i==0||i>=f->cp.size()||f->cp[i].tag!=1||f->cp[i].text.size()>65535)return 0;
 *out=f->cp[i].text.c_str();*len=static_cast<uint16_t>(f->cp[i].text.size());return 1;
}
extern "C" int opticraft_classfile_find_method(const OptiClassFile *f,const char *name,const char *desc,uint16_t *flags,uint32_t *code){
 if(!f||!name||!desc)return 0;
 for(const Method &m:f->methods){
  if(m.name>=f->cp.size()||m.desc>=f->cp.size())continue;
  const Cp &n=f->cp[m.name],&d=f->cp[m.desc];
  if(n.tag==1&&d.tag==1&&n.text==name&&d.text==desc){if(flags)*flags=m.access;if(code)*code=m.code;return 1;}
 }return 0;
}
