#ifndef OPTICRAFT_AWT_BRIDGE_H
#define OPTICRAFT_AWT_BRIDGE_H

#include <stdint.h>
#include "opticraft_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal AWT Frame/Canvas surface contract for the custom Java runtime.
 * This is deliberately not a java.awt implementation: the JVM method
 * dispatcher must map the corresponding Frame/Canvas calls to these APIs.
 */
typedef struct OptiAwtWindow {
    int32_t width;
    int32_t height;
    int32_t visible;
    int32_t packed;
    int32_t closed;
} OptiAwtWindow;

int opticraft_awt_window_init(OptiAwtWindow *window, int32_t width, int32_t height);
void opticraft_awt_window_set_preferred_size(OptiAwtWindow *window, int32_t width, int32_t height);
void opticraft_awt_window_pack(OptiAwtWindow *window);
void opticraft_awt_window_set_visible(OptiAwtWindow *window, int visible);
int opticraft_awt_window_poll(OptiAwtWindow *window, OptiInputSnapshot *input);
uint32_t *opticraft_awt_window_pixels(OptiAwtWindow *window, int32_t *stride_pixels);
int opticraft_awt_window_present(OptiAwtWindow *window);
void opticraft_awt_window_shutdown(OptiAwtWindow *window);

#ifdef __cplusplus
}
#endif
#endif
