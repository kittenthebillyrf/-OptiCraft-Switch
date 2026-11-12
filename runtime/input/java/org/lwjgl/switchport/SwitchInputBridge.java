package org.lwjgl.switchport;
public final class SwitchInputBridge {
 private SwitchInputBridge() {}
 public static native int init();
 public static native void shutdown();
 public static native int[] poll(int width,int height);
 public static native int[] update(long buttons,int lx,int ly,int rx,int ry,int width,int height,int tx,int ty,boolean touch);
 public static native boolean isKeyDown(int key);
 public static native boolean isMouseDown(int button);
 public static native int getMouseX();
 public static native int getMouseY();
 public static native void reset();
}
