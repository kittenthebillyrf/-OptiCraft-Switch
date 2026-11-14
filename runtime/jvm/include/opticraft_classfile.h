#ifndef OPTICRAFT_CLASSFILE_H
#define OPTICRAFT_CLASSFILE_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct OptiClassFile OptiClassFile;
typedef struct OptiClassSummary {
    uint16_t minor_version, major_version;
    uint16_t access_flags;
    uint16_t constant_pool_count;
    uint16_t fields_count, methods_count;
    uint16_t this_class, super_class;
} OptiClassSummary;
/* Parse a Java classfile into an owned, bounds-checked representation.
 * No bytecode is executed by this API. */
int opticraft_classfile_parse(const uint8_t *bytes, size_t length,
                              OptiClassFile **out_file);
void opticraft_classfile_destroy(OptiClassFile *file);
int opticraft_classfile_summary(const OptiClassFile *file,
                                OptiClassSummary *out_summary);
int opticraft_classfile_utf8(const OptiClassFile *file, uint16_t index,
                             const char **out_text, uint16_t *out_length);
/* which=0 returns this_class; which=1 returns super_class (empty for java/lang/Object). */
int opticraft_classfile_class_name(const OptiClassFile *file, int which,
                                   const char **out_text, uint16_t *out_length);
int opticraft_classfile_find_method(const OptiClassFile *file,
                                    const char *name, const char *descriptor,
                                    uint16_t *out_access_flags,
                                    uint32_t *out_code_length);
#ifdef __cplusplus
}
#endif
#endif
