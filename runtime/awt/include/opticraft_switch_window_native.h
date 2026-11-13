#ifndef OPTICRAFT_SWITCH_WINDOW_NATIVE_H
#define OPTICRAFT_SWITCH_WINDOW_NATIVE_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* C ABI targets for java/org/lwjgl/switchport/SwitchWindow native methods.
 * The custom JVM binds these names/descriptors in its native-method table.
 */
int32_t opticraft_switch_window_create(int32_t width, int32_t height);
void opticraft_switch_window_set_preferred_size(int32_t width, int32_t height);
void opticraft_switch_window_pack(void);
void opticraft_switch_window_set_visible(int32_t visible);
int32_t opticraft_switch_window_poll(void);
uint64_t opticraft_switch_window_buttons(void);
uint64_t opticraft_switch_window_pressed(void);
int32_t opticraft_switch_window_left_x(void);
int32_t opticraft_switch_window_left_y(void);
int32_t opticraft_switch_window_right_x(void);
int32_t opticraft_switch_window_right_y(void);
int32_t opticraft_switch_window_touch_x(void);
int32_t opticraft_switch_window_touch_y(void);
int32_t opticraft_switch_window_touch_down(void);
uint64_t opticraft_switch_window_framebuffer_address(void);
int32_t opticraft_switch_window_framebuffer_stride(void);
int32_t opticraft_switch_window_present(void);
int32_t opticraft_switch_window_should_close(void);
void opticraft_switch_window_destroy(void);
#ifdef __cplusplus
}
#endif
#endif
