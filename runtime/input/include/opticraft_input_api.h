#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int opticraft_input_poll(int width,int height,int32_t *out,int capacity_values);
int opticraft_input_update_snapshot(uint64_t buttons,int32_t lx,int32_t ly,int32_t rx,int32_t ry,int32_t tx,int32_t ty,int touch,int width,int height,int32_t *out,int capacity_values);
int opticraft_input_key_down(int key);
int opticraft_input_mouse_down(int button);
int opticraft_input_mouse_x(void);
int opticraft_input_mouse_y(void);
void opticraft_input_reset(void);
#ifdef __cplusplus
}
#endif
