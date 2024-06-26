#pragma once

#include <jni.h>

#define NATIVE_METHOD(METHOD_NAME) Java_com_nvision_facetracker_CameraRenderView_##METHOD_NAME

extern "C" {
JNIEXPORT void JNICALL NATIVE_METHOD(nativeCreateApp)(JNIEnv *jenv, jobject obj, jobject context, jstring shader);
JNIEXPORT void JNICALL NATIVE_METHOD(nativeResumeApp)(JNIEnv *jenv, jobject obj);
JNIEXPORT void JNICALL NATIVE_METHOD(nativePauseApp)(JNIEnv *jenv, jobject obj);
JNIEXPORT void JNICALL NATIVE_METHOD(nativeDestroyApp)(JNIEnv *jenv, jobject obj);

JNIEXPORT void JNICALL NATIVE_METHOD(nativeSetShader)(JNIEnv *jenv, jobject obj, jstring shader);
JNIEXPORT void JNICALL NATIVE_METHOD(nativeSetSurface)(JNIEnv *jenv, jobject obj, jobject surface);
JNIEXPORT jobject JNICALL NATIVE_METHOD(nativeSurfaceTexture)(JNIEnv *jenv, jobject obj, jboolean flip);
JNIEXPORT void JNICALL NATIVE_METHOD(nativeDestroyTexture)(JNIEnv *jenv, jobject obj);
JNIEXPORT void JNICALL NATIVE_METHOD(nativeRequestUpdateTexture)(JNIEnv *jenv, jobject obj);

// Cardboard methods
JNIEXPORT void JNICALL NATIVE_METHOD(nativeCreateSurface)(JNIEnv *jenv, jobject obj, jobject surface);
JNIEXPORT void JNICALL NATIVE_METHOD(nativeSwitchViewer)(JNIEnv *env, jobject obj);
}
