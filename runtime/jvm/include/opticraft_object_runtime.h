#ifndef OPTICRAFT_OBJECT_RUNTIME_H
#define OPTICRAFT_OBJECT_RUNTIME_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef uint32_t OptiClassId;
typedef uint64_t OptiObjectRef;
typedef enum OptiValueKind { OPTI_VALUE_I32=1, OPTI_VALUE_I64=2, OPTI_VALUE_REF=3 } OptiValueKind;
typedef struct OptiMethodInfo {
    OptiClassId owner;
    const char *name;
    const char *descriptor;
    uint32_t method_id;
} OptiMethodInfo;
/* Fixed-capacity deterministic heap. References are generation-checked handles,
 * never native pointers. Reset invalidates every outstanding handle. */
int opticraft_vm_reset(size_t object_capacity, size_t class_capacity);
int opticraft_class_define(OptiClassId id, OptiClassId super_id, uint32_t instance_slots);
int opticraft_object_new(OptiClassId class_id, OptiObjectRef *out_ref);
int opticraft_object_release(OptiObjectRef ref);
int opticraft_object_class(OptiObjectRef ref, OptiClassId *out_class);
int opticraft_field_get(OptiObjectRef ref, uint32_t slot, uint64_t *out_value);
int opticraft_field_set(OptiObjectRef ref, uint32_t slot, uint64_t value);
int opticraft_array_new(OptiValueKind kind, uint32_t length, OptiObjectRef *out_ref);
int opticraft_array_length(OptiObjectRef ref, uint32_t *out_length);
int opticraft_array_get(OptiObjectRef ref, uint32_t index, uint64_t *out_value);
int opticraft_array_set(OptiObjectRef ref, uint32_t index, uint64_t value);
/* Resolves a virtual method by walking runtime class -> superclass chain. */
int opticraft_virtual_resolve(OptiObjectRef receiver, const OptiMethodInfo *methods,
                              size_t method_count, const char *name,
                              const char *descriptor, uint32_t *out_method_id);
size_t opticraft_heap_live_count(void);
#ifdef __cplusplus
}
#endif
#endif
