package org.lwjgl.switchport;

/**
 * Java-side native window contract for the custom OptiCraft runtime.
 * This class intentionally avoids java.awt so it can run on the Switch
 * without a desktop AWT implementation.
 *
 * Native methods are registered by the custom JVM's native-method table.
 * Framebuffer memory is owned by the native runtime and addressed by a
 * 64-bit pointer for the runtime's direct-buffer bridge.
 */
public final class SwitchWindow {
    private SwitchWindow() {}

    public static native boolean create(int width, int height);
    public static native void setPreferredSize(int width, int height);
    public static native void pack();
    public static native void setVisible(boolean visible);
    public static native boolean poll();
    public static native long buttons();
    public static native long pressed();
    public static native int leftX();
    public static native int leftY();
    public static native int rightX();
    public static native int rightY();
    public static native int touchX();
    public static native int touchY();
    public static native boolean touchDown();
    public static native long framebufferAddress();
    public static native int framebufferStride();
    public static native boolean present();
    public static native boolean shouldClose();
    public static native void destroy();
}
