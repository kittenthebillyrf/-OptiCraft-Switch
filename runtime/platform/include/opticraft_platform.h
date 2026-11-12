#ifndef OPTICRAFT_PLATFORM_H
#define OPTICRAFT_PLATFORM_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define OPTI_PLATFORM_ABI_VERSION 1u
typedef struct OptiInputSnapshot {
 uint64_t buttons, pressed, released;
 int32_t left_x, left_y, right_x, right_y, touch_x, touch_y;
 uint8_t touch_down, should_exit, reserved[6];
} OptiInputSnapshot;
uint32_t opticraft_platform_abi_version(void);
int opticraft_platform_init(void);
void opticraft_platform_shutdown(void);
void opticraft_platform_poll_events(OptiInputSnapshot *out);
uint64_t opticraft_platform_ticks_ns(void);
int opticraft_platform_read_file(const char *relative_path, void *destination, size_t capacity, size_t *bytes_read);
int opticraft_platform_present_rgba8888(const uint32_t *pixels, int width, int height, int stride_pixels);
#ifdef __cplusplus
}
#endif
#endif
