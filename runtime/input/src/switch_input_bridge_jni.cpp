#include "opticraft_input_api.h"
#include "../../platform/include/opticraft_platform.h"
#include <jni.h>
#include <vector>
namespace {
jint init(JNIEnv*,jclass){return opticraft_platform_init();}
void shutdown(JNIEnv*,jclass){opticraft_platform_shutdown();}
jintArray array(JNIEnv* e,const std::vector<jint>& v){jintArray a=e->NewIntArray(static_cast<jsize>(v.size()));if(a&&!v.empty())e->SetIntArrayRegion(a,0,static_cast<jsize>(v.size()),v.data());return a;}
jintArray poll(JNIEnv* e,jclass,jint w,jint h){std::vector<jint> v(256);int n=opticraft_input_poll(w,h,v.data(),static_cast<int>(v.size()));if(n<0)return e->NewIntArray(0);v.resize(static_cast<std::size_t>(n));return array(e,v);}
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
