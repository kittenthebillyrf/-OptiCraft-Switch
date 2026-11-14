#include "opticraft_classfile.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 // public class T { public static void main(String[] a) {} }
 const uint8_t bytes[]={
  0xca,0xfe,0xba,0xbe,0,0,0,49,0,12,
  1,0,1,'T',7,0,1,1,0,16,'j','a','v','a','/','l','a','n','g','/','O','b','j','e','c','t',7,0,3,
  1,0,4,'m','a','i','n',1,0,22,'(','[','L','j','a','v','a','/','l','a','n','g','/','S','t','r','i','n','g',';',')','V',
  1,0,4,'C','o','d','e',12,0,5,0,6,
  0,0x21,0,2,0,4,0,0,0,0,0,1,0,9,0,5,0,6,0,1,0,8,0,0,0,13,
  0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0
 };
 OptiClassFile *f=nullptr;assert(opticraft_classfile_parse(bytes,sizeof(bytes),&f));
 OptiClassSummary s{};assert(opticraft_classfile_summary(f,&s));assert(s.major_version==49&&s.methods_count==1);
 uint16_t flags=0;uint32_t code=99;assert(opticraft_classfile_find_method(f,"main","([Ljava/lang/String;)V",&flags,&code));assert(code==1);
 opticraft_classfile_destroy(f);assert(!opticraft_classfile_parse(bytes,8,&f));
 return 0;
}
