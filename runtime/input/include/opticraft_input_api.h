#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define OPTI_LWJGL_KEYBOARD_EVENT_SIZE 18u
#define OPTI_LWJGL_MOUSE_EVENT_SIZE 22u
#define OPTI_LWJGL_KEYBOARD_STATE_SIZE 256u
#define OPTI_LWJGL_MOUSE_STATE_SIZE 8u
#define OPTI_LWJGL_MAX_EVENTS_PER_FRAME 16u

typedef struct OptiLwjglInputBuffers {
    uint8_t* keyboard_events;
    size_t keyboard_capacity;
    size_t keyboard_bytes;
    uint8_t* mouse_events;
    size_t mouse_capacity;
    size_t mouse_bytes;
    uint8_t* key_state;
    size_t key_state_capacity;
    uint8_t* mouse_state;
    size_t mouse_state_capacity;
    int32_t mouse_x;
    int32_t mouse_y;
} OptiLwjglInputBuffers;

int opticraft_input_poll(int width,int height,int32_t *out,int capacity_values);
int opticraft_input_update_snapshot(uint64_t buttons,int32_t lx,int32_t ly,int32_t rx,int32_t ry,int32_t tx,int32_t ty,int touch,int width,int height,int32_t *out,int capacity_values);
int opticraft_input_poll_lwjgl(int width,int height,OptiLwjglInputBuffers* out);
int opticraft_input_update_lwjgl_snapshot(uint64_t buttons,int32_t lx,int32_t ly,int32_t rx,int32_t ry,int32_t tx,int32_t ty,int touch,int width,int height,OptiLwjglInputBuffers* out);
int opticraft_input_key_down(int key);
int opticraft_input_mouse_down(int button);
int opticraft_input_mouse_x(void);
int opticraft_input_mouse_y(void);
void opticraft_input_reset(void);

#ifdef __cplusplus
}
#endif
