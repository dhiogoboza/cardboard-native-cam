#include "jni-bridge.h"

#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer
#include <android/log.h>
#include <jni.h>

#include "CardboardApp.h"

using namespace cardboardapp;
using namespace ndk_hello_cardboard;

JavaVM* javaVm;
static CardboardApp *cardboardApp = nullptr;
static ANativeWindow *nativeWindow = nullptr;

inline jlong jptr(CardboardApp* native_app)
{
    return reinterpret_cast<intptr_t>(native_app);
}

inline CardboardApp* native(jlong ptr)
{
    return reinterpret_cast<CardboardApp*>(ptr);
}

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    javaVm = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL NATIVE_METHOD(nativeCreateApp)(JNIEnv *jenv, jobject obj, jobject context)
{
    Cardboard_initializeAndroid(javaVm, context);

    VRConfigs configs("");
    configs.screenHeight = 720;
    configs.screenWidth = 1280;
    cardboardApp = new CardboardApp(configs);
}

JNIEXPORT void JNICALL NATIVE_METHOD(nativeOnDestroy)(JNIEnv *jenv, jobject obj)
{
    delete cardboardApp;
    cardboardApp = nullptr;
}

JNIEXPORT void JNICALL NATIVE_METHOD(nativeCreateSurface)(JNIEnv *jenv, jobject obj, jobject surface)
{
    cardboardApp->CreateSurface();
}

JNIEXPORT void JNICALL NATIVE_METHOD(nativeSetSurface)(JNIEnv *jenv, jobject obj, jobject surface) {
    if (surface) {
        nativeWindow = ANativeWindow_fromSurface(jenv, surface);
        cardboardApp->SetWindow(nativeWindow);
    } else {
        cardboardApp->SetWindow(0);
        ANativeWindow_release(nativeWindow);
        nativeWindow = nullptr;
    }
}

/*JNI_METHOD(void, nativeOnDrawFrame) (JNIEnv* env, jobject obj, jlong native_app, jfloatArray texMatrix) {
    native(native_app)->OnDrawFrame(texMatrix);
}*/

JNIEXPORT void JNICALL NATIVE_METHOD(nativePauseApp)(JNIEnv *jenv, jobject obj)
{
    cardboardApp->Pause();
}

JNIEXPORT void JNICALL NATIVE_METHOD(nativeResumeApp)(JNIEnv *jenv, jobject obj)
{
    cardboardApp->Resume();
}

//JNI_METHOD(void, nativeSetScreenParams) (JNIEnv* env, jobject obj, jlong native_app, jint width, jint height)
//{
//    native(native_app)->SetScreenParams(width, height);
//}

JNIEXPORT void JNICALL NATIVE_METHOD(nativeSwitchViewer)(JNIEnv *jenv, jobject obj)
{
    cardboardApp->SwitchViewer();
}

JNIEXPORT void JNICALL NATIVE_METHOD(nativeOnDrawFrame)(JNIEnv *jenv, jobject obj, jlong matAddr)
{
    cardboardApp->Render(matAddr);
}

}