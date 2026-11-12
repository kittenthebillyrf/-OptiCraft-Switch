#include "opticraft_input_api.h"
#include "../../platform/include/opticraft_platform.h"
#include <jni.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {
jint init(JNIEnv*,jclass){return opticraft_platform_init();}
void shutdown(JNIEnv*,jclass){opticraft_platform_shutdown();}
jintArray array(JNIEnv* e,const std::vector<jint>& v){jintArray a=e->NewIntArray(static_cast<jsize>(v.size()));if(a&&!v.empty())e->SetIntArrayRegion(a,0,static_cast<jsize>(v.size()),v.data());return a;}
jintArray poll(JNIEnv* e,jclass,jint w,jint h){std::vector<jint> v(256);int n=opticraft_input_poll(w,h,v.data(),static_cast<int>(v.size()));if(n<0)return e->NewIntArray(0);v.resize(static_cast<std::size_t>(n));return array(e,v);}
jobjectArray byte_arrays(JNIEnv* e,const std::array<std::vector<jbyte>,4>& values){
 jclass byte_array_class=e->FindClass("[B");if(!byte_array_class)return nullptr;
 jobjectArray result=e->NewObjectArray(4,byte_array_class,nullptr);e->DeleteLocalRef(byte_array_class);if(!result)return nullptr;
 for(jsize i=0;i<4;++i){
  const auto& value=values[static_cast<std::size_t>(i)];
  jbyteArray part=e->NewByteArray(static_cast<jsize>(value.size()));if(!part)return nullptr;
  if(!value.empty())e->SetByteArrayRegion(part,0,static_cast<jsize>(value.size()),value.data());
  e->SetObjectArrayElement(result,i,part);e->DeleteLocalRef(part);
 }
 return result;
}
jobjectArray poll_lwjgl(JNIEnv* e,jclass,jint w,jint h){
 std::array<std::uint8_t,OPTI_LWJGL_MAX_EVENTS_PER_FRAME*OPTI_LWJGL_KEYBOARD_EVENT_SIZE> keyboard{};
 std::array<std::uint8_t,OPTI_LWJGL_MAX_EVENTS_PER_FRAME*OPTI_LWJGL_MOUSE_EVENT_SIZE> mouse{};
 std::array<std::uint8_t,OPTI_LWJGL_KEYBOARD_STATE_SIZE> keys{};
 std::array<std::uint8_t,OPTI_LWJGL_MOUSE_STATE_SIZE> buttons{};
 OptiLwjglInputBuffers output{keyboard.data(),keyboard.size(),0,mouse.data(),mouse.size(),0,keys.data(),keys.size(),buttons.data(),buttons.size(),0,0};
 if(opticraft_input_poll_lwjgl(w,h,&output)!=0)return nullptr;
 std::array<std::vector<jbyte>,4> values;
 values[0].assign(reinterpret_cast<const jbyte*>(keyboard.data()),reinterpret_cast<const jbyte*>(keyboard.data()+output.keyboard_bytes));
 values[1].assign(reinterpret_cast<const jbyte*>(mouse.data()),reinterpret_cast<const jbyte*>(mouse.data()+output.mouse_bytes));
 values[2].assign(reinterpret_cast<const jbyte*>(keys.data()),reinterpret_cast<const jbyte*>(keys.data()+keys.size()));
 values[3].assign(reinterpret_cast<const jbyte*>(buttons.data()),reinterpret_cast<const jbyte*>(buttons.data()+buttons.size()));
 return byte_arrays(e,values);
}
jintArray update(JNIEnv* e,jclass,jlong b,jint lx,jint ly,jint rx,jint ry,jint w,jint h,jint tx,jint ty,jboolean touch){
 std::vector<jint> v(256);int n=opticraft_input_update_snapshot(static_cast<uint64_t>(b),lx,ly,rx,ry,tx,ty,touch==JNI_TRUE,w,h,v.data(),static_cast<int>(v.size()));
 if(n<0)return e->NewIntArray(0);v.resize(static_cast<std::size_t>(n));return array(e,v);
}
jboolean key(JNIEnv*,jclass,jint k){return opticraft_input_key_down(k)?JNI_TRUE:JNI_FALSE;}
jboolean mouse(JNIEnv*,jclass,jint b){return opticraft_input_mouse_down(b)?JNI_TRUE:JNI_FALSE;}
jint mx(JNIEnv*,jclass){return opticraft_input_mouse_x();}jint my(JNIEnv*,jclass){return opticraft_input_mouse_y();}
void reset(JNIEnv*,jclass){opticraft_input_reset();}
JNINativeMethod methods[]={
 {const_cast<char*>("init"),const_cast<char*>("()I"),reinterpret_cast<void*>(init)},
 {const_cast<char*>("shutdown"),const_cast<char*>("()V"),reinterpret_cast<void*>(shutdown)},
 {const_cast<char*>("poll"),const_cast<char*>("(II)[I"),reinterpret_cast<void*>(poll)},
 {const_cast<char*>("pollLwjgl"),const_cast<char*>("(II)[[B"),reinterpret_cast<void*>(poll_lwjgl)},
 {const_cast<char*>("update"),const_cast<char*>("(JIIIIIIIIZ)[I"),reinterpret_cast<void*>(update)},
 {const_cast<char*>("isKeyDown"),const_cast<char*>("(I)Z"),reinterpret_cast<void*>(key)},
 {const_cast<char*>("isMouseDown"),const_cast<char*>("(I)Z"),reinterpret_cast<void*>(mouse)},
 {const_cast<char*>("getMouseX"),const_cast<char*>("()I"),reinterpret_cast<void*>(mx)},
 {const_cast<char*>("getMouseY"),const_cast<char*>("()I"),reinterpret_cast<void*>(my)},
 {const_cast<char*>("reset"),const_cast<char*>("()V"),reinterpret_cast<void*>(reset)}
};
}
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm,void*){
 JNIEnv* e=nullptr;if(vm->GetEnv(reinterpret_cast<void**>(&e),JNI_VERSION_1_6)!=JNI_OK)return JNI_ERR;
 jclass c=e->FindClass("org/lwjgl/switchport/SwitchInputBridge");if(!c)return JNI_ERR;
 jint rc=e->RegisterNatives(c,methods,sizeof(methods)/sizeof(methods[0]));e->DeleteLocalRef(c);return rc==0?JNI_VERSION_1_6:JNI_ERR;
}
