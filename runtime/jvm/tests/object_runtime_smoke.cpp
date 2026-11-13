#include "opticraft_object_runtime.h"
#include <cassert>
int main() {
    assert(opticraft_vm_reset(16,8));
    assert(opticraft_class_define(1,0,2));
    assert(opticraft_class_define(2,1,3));
    OptiObjectRef obj=0;assert(opticraft_object_new(2,&obj));
    OptiClassId cid=0;assert(opticraft_object_class(obj,&cid)&&cid==2);
    assert(opticraft_field_set(obj,2,0x12345678u));
    uint64_t value=0;assert(opticraft_field_get(obj,2,&value)&&value==0x12345678u);
    assert(!opticraft_field_get(obj,3,&value));
    const OptiMethodInfo methods[]={{1,"tick","()V",10},{2,"render","(I)V",20},{1,"render","(I)V",11}};
    uint32_t method=0;assert(opticraft_virtual_resolve(obj,methods,3,"tick","()V",&method)&&method==10);
    assert(opticraft_virtual_resolve(obj,methods,3,"render","(I)V",&method)&&method==20);
    assert(!opticraft_virtual_resolve(obj,methods,3,"missing","()V",&method));
    OptiObjectRef arr=0;assert(opticraft_array_new(OPTI_VALUE_I32,4,&arr));
    uint32_t len=0;assert(opticraft_array_length(arr,&len)&&len==4);
    assert(opticraft_array_set(arr,3,0x100000002ull));
    assert(opticraft_array_get(arr,3,&value)&&value==2);
    assert(!opticraft_array_get(arr,4,&value));
    assert(opticraft_heap_live_count()==2);
    assert(opticraft_object_release(obj));assert(!opticraft_object_class(obj,&cid));
    OptiObjectRef replacement=0;assert(opticraft_object_new(1,&replacement));
    assert(replacement!=obj);assert(!opticraft_field_get(obj,0,&value));
    assert(opticraft_heap_live_count()==2);
    assert(opticraft_vm_reset(4,4));assert(!opticraft_array_length(arr,&len));
    return 0;
}
