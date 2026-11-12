package org.lwjgl.switchport;
public final class SwitchInputBridgeSmokeTest {
 public static void main(String[] args){
  System.load(args[0]);if(SwitchInputBridge.init()!=0)throw new AssertionError("init");
  if(SwitchInputBridge.poll(960,544).length!=0)throw new AssertionError("unexpected host input");
  int[] e=SwitchInputBridge.update(1L<<1,0,0,0,0,960,544,0,0,false);
  if(e.length!=4||e[0]!=1||e[1]!=1||!SwitchInputBridge.isKeyDown(1))throw new AssertionError("B to Escape");
  SwitchInputBridge.shutdown();System.out.println("PASS JNI input bridge smoke test");
 }
}
