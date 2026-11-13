#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct OpticraftTextureAtlas {
    uint32_t width;
    uint32_t height;
    uint8_t* rgba;
} OpticraftTextureAtlas;
/* Loads terrain.png from the user's own minecraft.jar (ZIP + PNG). */
int opticraft_texture_load_terrain_from_jar(const char* jar_path, OpticraftTextureAtlas* output);
void opticraft_texture_atlas_free(OpticraftTextureAtlas* atlas);
#ifdef __cplusplus
}
#endif
