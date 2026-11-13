#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPTICRAFT_WORLD_FOLDER_MAX 128

typedef struct OpticraftWorldCatalogEntry {
    char folder[OPTICRAFT_WORLD_FOLDER_MAX];
    uint8_t has_level_dat;
} OpticraftWorldCatalogEntry;

/* Returns number of discovered save folders, or a negative error code.
 * A folder is listed only when level.dat or level.dat_old exists.
 * No world data is modified.
 */
int opticraft_world_catalog_scan(
    const char* saves_root,
    OpticraftWorldCatalogEntry* entries,
    size_t capacity);

#ifdef __cplusplus
}
#endif
