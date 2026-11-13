#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define OPTICRAFT_WORLD_NAME_MAX 128
typedef struct OpticraftWorldPreview {
    char level_name[OPTICRAFT_WORLD_NAME_MAX];
    int64_t time;
    int32_t spawn_x, spawn_y, spawn_z;
    uint8_t has_level_name, has_time, has_spawn;
} OpticraftWorldPreview;
/* Reads level.dat, falling back to level.dat_old. Returns 0 on success. */
int opticraft_world_save_read_preview(const char* world_directory, OpticraftWorldPreview* output);
#ifdef __cplusplus
}
#endif
