#include "opticraft_object_runtime.h"
#include <vector>
#include <string>
#include <cstring>
#include <limits>
namespace {
struct ClassDef { OptiClassId id, super_id; uint32_t slots; bool used; };
struct Object { uint32_t generation; OptiClassId class_id; bool used, array; OptiValueKind kind; std::vector<uint64_t> values; };
std::vector<ClassDef> g_classes;
std::vector<Object> g_objects;
uint32_t g_next_generation=1;
OptiObjectRef make_ref(size_t index,uint32_t generation) {
    return (static_cast<uint64_t>(generation)<<32) | static_cast<uint32_t>(index+1);
}
bool decode(OptiObjectRef ref,size_t *index,Object **object) {
    uint32_t low=static_cast<uint32_t>(ref), gen=static_cast<uint32_t>(ref>>32);
    if(!low||!gen) return false;
    size_t i=static_cast<size_t>(low-1);
    if(i>=g_objects.size()||!g_objects[i].used||g_objects[i].generation!=gen) return false;
    if(index)*index=i; if(object)*object=&g_objects[i]; return true;
}
ClassDef *find_class(OptiClassId id) {
    for(auto &c:g_classes) if(c.used&&c.id==id)return &c;
    return nullptr;
}
size_t free_slot() {
    for(size_t i=0;i<g_objects.size();++i)if(!g_objects[i].used)return i;
    return g_objects.size();
}
int allocate(OptiClassId id,bool array,OptiValueKind kind,uint32_t length,uint32_t slots,OptiObjectRef *out) {
    if(!out)return 0; size_t i=free_slot(); if(i==g_objects.size())return 0;
    uint32_t gen=++g_next_generation; if(gen==0)gen=++g_next_generation;
    Object &o=g_objects[i]; o.generation=gen;o.class_id=id;o.used=true;o.array=array;o.kind=kind;
    o.values.assign(array?length:slots,0);*out=make_ref(i,gen);return 1;
}
}
extern "C" int opticraft_vm_reset(size_t object_capacity,size_t class_capacity) {
    if(!object_capacity||!class_capacity||object_capacity>0xffffffffu)return 0;
    g_objects.clear();g_objects.resize(object_capacity);g_classes.clear();g_classes.reserve(class_capacity);
    ++g_next_generation;if(!g_next_generation)++g_next_generation;return 1;
}
extern "C" int opticraft_class_define(OptiClassId id,OptiClassId super_id,uint32_t slots) {
    if(!id||find_class(id)||(super_id&&!find_class(super_id)))return 0;
    if(g_classes.size()==g_classes.capacity())return 0;
    g_classes.push_back({id,super_id,slots,true});return 1;
}
extern "C" int opticraft_object_new(OptiClassId id,OptiObjectRef *out) {
    ClassDef *c=find_class(id);if(!c)return 0;return allocate(id,false,OPTI_VALUE_I64,0,c->slots,out);
}
extern "C" int opticraft_object_release(OptiObjectRef ref) {
    Object *o=nullptr;if(!decode(ref,nullptr,&o))return 0;o->used=false;o->values.clear();return 1;
}
extern "C" int opticraft_object_class(OptiObjectRef ref,OptiClassId *out) {
    Object *o=nullptr;if(!out||!decode(ref,nullptr,&o)||o->array)return 0;*out=o->class_id;return 1;
}
extern "C" int opticraft_field_get(OptiObjectRef ref,uint32_t slot,uint64_t *out) {
    Object *o=nullptr;if(!out||!decode(ref,nullptr,&o)||o->array||slot>=o->values.size())return 0;*out=o->values[slot];return 1;
}
extern "C" int opticraft_field_set(OptiObjectRef ref,uint32_t slot,uint64_t value) {
    Object *o=nullptr;if(!decode(ref,nullptr,&o)||o->array||slot>=o->values.size())return 0;o->values[slot]=value;return 1;
}
extern "C" int opticraft_array_new(OptiValueKind kind,uint32_t length,OptiObjectRef *out) {
    if(kind<OPTI_VALUE_I32||kind>OPTI_VALUE_REF)return 0;return allocate(0,true,kind,length,0,out);
}
extern "C" int opticraft_array_length(OptiObjectRef ref,uint32_t *out) {
    Object *o=nullptr;if(!out||!decode(ref,nullptr,&o)||!o->array)return 0;*out=static_cast<uint32_t>(o->values.size());return 1;
}
extern "C" int opticraft_array_get(OptiObjectRef ref,uint32_t index,uint64_t *out) {
    Object *o=nullptr;if(!out||!decode(ref,nullptr,&o)||!o->array||index>=o->values.size())return 0;*out=o->values[index];return 1;
}
extern "C" int opticraft_array_set(OptiObjectRef ref,uint32_t index,uint64_t value) {
    Object *o=nullptr;if(!decode(ref,nullptr,&o)||!o->array||index>=o->values.size())return 0;
    if(o->kind==OPTI_VALUE_I32)value=static_cast<uint32_t>(value);o->values[index]=value;return 1;
}
extern "C" int opticraft_virtual_resolve(OptiObjectRef receiver,const OptiMethodInfo *methods,size_t count,const char *name,const char *desc,uint32_t *out) {
    Object *o=nullptr;if(!methods||!name||!desc||!out||!decode(receiver,nullptr,&o)||o->array)return 0;
    OptiClassId current=o->class_id;
    while(current) {
        for(size_t i=0;i<count;++i)if(methods[i].owner==current&&methods[i].name&&methods[i].descriptor&&
            std::strcmp(methods[i].name,name)==0&&std::strcmp(methods[i].descriptor,desc)==0) {
                *out=methods[i].method_id;return 1;
            }
        ClassDef *c=find_class(current);if(!c)return 0;current=c->super_id;
    }
    return 0;
}
extern "C" size_t opticraft_heap_live_count(void) {
    size_t n=0;for(const auto &o:g_objects)if(o.used)++n;return n;
}
