#ifndef OPTICRAFT_NATIVE_REGISTRY_H
#define OPTICRAFT_NATIVE_REGISTRY_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* ABI-neutral native call frame for the in-tree JVM.
 * args/results are 64-bit slots; references are opaque handles, never raw
 * host pointers. The JVM owns argument decoding and local-reference lifetime.
 */
typedef struct OptiNativeCall {
    void *vm;
    uint64_t receiver;
    const uint64_t *args;
    size_t arg_count;
    uint64_t result;
    int32_t exception_code;
} OptiNativeCall;
typedef void (*OptiNativeHandler)(OptiNativeCall *call);
typedef struct OptiNativeMethod {
    const char *class_name;
    const char *method_name;
    const char *descriptor;
    OptiNativeHandler handler;
} OptiNativeMethod;
const OptiNativeMethod *opticraft_native_resolve(const char *class_name,
                                                   const char *method_name,
                                                   const char *descriptor);
size_t opticraft_native_method_count(void);
int opticraft_native_invoke(const OptiNativeMethod *method, OptiNativeCall *call);
#ifdef __cplusplus
}
#endif
#endif
