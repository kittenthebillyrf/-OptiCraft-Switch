#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct OpticraftAnvilChunkPreview {
    uint8_t block_id[256];
    uint8_t height[256];
    int32_t chunk_x;
    int32_t chunk_z;
    uint8_t valid_columns;
} OpticraftAnvilChunkPreview;
int opticraft_world_read_spawn_chunk(const char* world_directory,
                                     int32_t spawn_x, int32_t spawn_z,
                                     OpticraftAnvilChunkPreview* output);
#ifdef __cplusplus
}
#endif
