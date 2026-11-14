#ifndef OPTICRAFT_JAR_CLASSLOADER_H
#define OPTICRAFT_JAR_CLASSLOADER_H
#include <stddef.h>
#include <stdint.h>
#include "opticraft_classfile.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct OptiJar OptiJar;
typedef struct OptiLoadedClass OptiLoadedClass;
/* Opens a user-supplied Java archive. max_archive_bytes=0 selects 128 MiB. */
int opticraft_jar_open_file(const char *path, size_t max_archive_bytes,
                            OptiJar **out_jar);
void opticraft_jar_close(OptiJar *jar);
/* Loads and parses one class by JVM internal name, e.g. net/minecraft/client/Minecraft.
 * The returned class is owned by the caller and must be destroyed. */
int opticraft_jar_load_class(OptiJar *jar, const char *internal_name,
                             OptiLoadedClass **out_class);
void opticraft_loaded_class_destroy(OptiLoadedClass *loaded);
int opticraft_loaded_class_summary(const OptiLoadedClass *loaded,
                                    OptiClassSummary *out_summary);
int opticraft_loaded_class_name(const OptiLoadedClass *loaded, int which,
                                 const char **out_text, uint16_t *out_length);
int opticraft_loaded_class_find_method(const OptiLoadedClass *loaded,
                                        const char *name, const char *descriptor,
                                        uint16_t *out_access_flags,
                                        uint32_t *out_code_length);
/* Last diagnostic for a failed open/load operation; valid until the next operation. */
const char *opticraft_jar_last_error(const OptiJar *jar);
#ifdef __cplusplus
}
#endif
#endif
