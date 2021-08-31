#pragma once
#include <jni.h>
#include <string>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cardboard.h>
#include "util.h"
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer

namespace cardboardapp {

class CardboardApp {
public:
    CardboardApp(ndk_hello_cardboard::VRConfigs configs);

    ~CardboardApp();

    ndk_hello_cardboard::Matrix4x4 GetPose();

    void Render(jlong matAddr);

    void SetConfigs(const ndk_hello_cardboard::VRConfigs &configs);

    void Resume();

    void Pause();

    void Destroy();

    void CreateSurface();

    bool UpdateDeviceParams();

    void SwitchViewer();

    void SetWindow(ANativeWindow *window);

private:
    GLuint vertex_id_;
    GLuint uv_id_;
    GLuint index_id_;
    GLuint program_id_;
    GLuint position_handle_;
    GLuint resolution_handle_;
    GLuint uv_handle_;
    GLuint texture_handle_;

    int width_;
    int height_;
    ndk_hello_cardboard::VRConfigs vr_configs_;

    // Cardboard related variables
    CardboardHeadTracker* head_tracker_;
    CardboardLensDistortion* lens_distortion_;
    CardboardDistortionRenderer* distortion_renderer_;

    CardboardEyeTextureDescription left_eye_texture_description_;
    CardboardEyeTextureDescription right_eye_texture_description_;

    bool screen_params_changed_;
    bool device_params_changed_;

    float projection_matrices_[2][16];
    float eye_matrices_[2][16];

    GLuint depthRenderBuffer_;
    GLuint framebuffer_;
    GLuint texture_;

    ndk_hello_cardboard::Matrix4x4 head_view_;
    ndk_hello_cardboard::Matrix4x4 model_target_;

    //EGL Resources
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLConfig  config_;

    GLuint surface_texture_id_;

    void GlSetup();

    void GlTeardown();

    void CreateSurfaceTextureId();
};

} // namespace cardboardapp