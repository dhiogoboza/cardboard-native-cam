#include "CardboardApp.h"
#include <array>
#include <cardboard.h>
#include <cstdlib>
#include <utility>

static constexpr float kZNear = 0.1f;
static constexpr float kZFar = 100.f;

constexpr float kMinTargetDistance = 2.5f;
constexpr float kMinTargetHeight = 0.5f;

constexpr uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;
static constexpr uint64_t kNanosInSeconds = 1000000000;
constexpr float kDefaultFloorHeight = -1.7f;

using namespace ndk_hello_cardboard;

namespace cardboardapp {

CardboardApp::CardboardApp(ndk_hello_cardboard::VRConfigs configs)
    : vr_configs_(std::move(configs))
{
    head_tracker_ = CardboardHeadTracker_create();
}

CardboardApp::~CardboardApp()
{
    CardboardHeadTracker_destroy(head_tracker_);
    CardboardLensDistortion_destroy(lens_distortion_);
    CardboardDistortionRenderer_destroy(distortion_renderer_);
}

Matrix4x4 CardboardApp::GetPose() {
    std::array<float, 4> out_orientation{};
    std::array<float, 3> out_position{};

    long monotonic_time_nano = GetMonotonicTimeNano();
    monotonic_time_nano += kPredictionTimeWithoutVsyncNanos;
    CardboardHeadTracker_getPose(head_tracker_, monotonic_time_nano,
                                 &out_position[0], &out_orientation[0]);
    return GetTranslationMatrix(out_position) *
           Quatf::FromXYZW(&out_orientation[0]).ToMatrix();
}

void CardboardApp::Render(jlong matAddr)
{

    if (!UpdateDeviceParams()) {
        return;
    }

    // Update Head Pose.
    head_view_ = GetPose();

    // Incorporate the floor height into the head_view
    head_view_ =
            head_view_ * GetTranslationMatrix({0.0f, kDefaultFloorHeight, 0.0f});

    // Bind buffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    android_app_update_tex_image();
//
//    if (cam_background_) {
//        cam_background_->BeforeRender();
//
//        // Draw camera on eye 1
//        glViewport(0, 0, width_ / 2, height_);
//        cam_background_->Render();
//
//        // Draw camera on eye 2
//        glViewport(width_ / 2, 0, width_ / 2, height_);
//        cam_background_->Render();
//
//        cam_background_->AfterRender();
//    }
    // aqui


    // Render
    glViewport(0, 0, width_, height_);
    CardboardDistortionRenderer_renderEyeToDisplay(
            distortion_renderer_, /* target_display = */ 0, /* x = */ 0, /* y = */ 0,
            width_, height_, &left_eye_texture_description_,
            &right_eye_texture_description_);
}

void CardboardApp::SetConfigs(const VRConfigs &configs)
{

}

bool CardboardApp::UpdateDeviceParams() {
    // Checks if screen or device parameters changed
    if (!screen_params_changed_ && !device_params_changed_) {
        return true;
    }

    // Get saved device parameters
    uint8_t *buffer;
    int size;
    CardboardQrCode_getSavedDeviceParams(&buffer, &size);

    // If there are no parameters saved yet, returns false.
    if (size == 0) {
        return false;
    }

    CardboardLensDistortion_destroy(lens_distortion_);
    lens_distortion_ = CardboardLensDistortion_create(
            buffer, size, width_, height_);

    CardboardQrCode_destroy(buffer);

    GlSetup();

    CardboardDistortionRenderer_destroy(distortion_renderer_);
    distortion_renderer_ = CardboardOpenGlEs2DistortionRenderer_create();

    // Get the distortion meshes for each of the eyes and pass it to the distortion renderer:
    // FIXME: Reverse order?
    CardboardMesh left_mesh;
    CardboardMesh right_mesh;
    CardboardLensDistortion_getDistortionMesh(lens_distortion_, kLeft,
                                              &left_mesh);
    CardboardLensDistortion_getDistortionMesh(lens_distortion_, kRight,
                                              &right_mesh);

    CardboardDistortionRenderer_setMesh(distortion_renderer_, &left_mesh, kLeft);
    CardboardDistortionRenderer_setMesh(distortion_renderer_, &right_mesh,
                                        kRight);

    // Get eye matrices
    // Get view and projection matrices for left and right eye
    // FIXME: Reverse order?
    CardboardLensDistortion_getEyeFromHeadMatrix(
            lens_distortion_, kLeft, eye_matrices_[0]);
    CardboardLensDistortion_getEyeFromHeadMatrix(
            lens_distortion_, kRight, eye_matrices_[1]);
    CardboardLensDistortion_getProjectionMatrix(
            lens_distortion_, kLeft, kZNear, kZFar, projection_matrices_[0]);
    CardboardLensDistortion_getProjectionMatrix(
            lens_distortion_, kRight, kZNear, kZFar, projection_matrices_[1]);

    screen_params_changed_ = false;
    device_params_changed_ = false;

    return true;
}

void CardboardApp::GlSetup() {
    if (framebuffer_ != 0) {
        GlTeardown();
    }

    // Create render texture.
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    left_eye_texture_description_.texture = texture_;
    left_eye_texture_description_.left_u = 0;
    left_eye_texture_description_.right_u = 0.5;
    left_eye_texture_description_.top_v = 1;
    left_eye_texture_description_.bottom_v = 0;

    right_eye_texture_description_.texture = texture_;
    right_eye_texture_description_.left_u = 0.5;
    right_eye_texture_description_.right_u = 1;
    right_eye_texture_description_.top_v = 1;
    right_eye_texture_description_.bottom_v = 0;

    // FIXME: Is the code below necessary?
    // Generate depth buffer to perform depth test.
    glGenRenderbuffers(1, &depthRenderBuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width_,
                          height_);

    // Create render target.
    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depthRenderBuffer_);
}

void CardboardApp::GlTeardown() {
    if (!framebuffer_) {
        return;
    }

    glDeleteRenderbuffers(1, &depthRenderBuffer_);
    depthRenderBuffer_ = 0;
    glDeleteFramebuffers(1, &framebuffer_);
    framebuffer_ = 0;
    glDeleteTextures(1, &texture_);
    texture_ = 0;
}

void CardboardApp::Resume() {
    CardboardHeadTracker_resume(head_tracker_);

    // Parameters may have changed.
    device_params_changed_ = true;

    // Check for device parameters existence in external storage. If they're
    // missing, we must scan a Cardboard QR code and save the obtained parameters.
    uint8_t *buffer;
    int size;
    CardboardQrCode_getSavedDeviceParams(&buffer, &size);
    if (size == 0) {
        SwitchViewer();
    }
    CardboardQrCode_destroy(buffer);
}


void CardboardApp::Pause() {
    CardboardHeadTracker_pause(head_tracker_);
}

void CardboardApp::Destroy() {
    CardboardHeadTracker_destroy(head_tracker_);
    head_tracker_ = nullptr;
}

void CardboardApp::CreateSurface() {
    // TODO:
    // Target object first appears directly in front of user.
    model_target_ = GetTranslationMatrix({0.0f, 1.5f, kMinTargetDistance});
}

void CardboardApp::SwitchViewer() {
    CardboardQrCode_scanQrCodeAndSaveDeviceParams();
}

void CardboardApp::SetWindow(ANativeWindow *window)
{
    if (!window) {
        return;
    }

    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
    };
    //opengl es2.0 context
    EGLint contextAtt[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE};
    EGLDisplay display;
    EGLConfig config;
    EGLint numConfigs;
    EGLint format;
    EGLSurface surface;
    EGLContext context;
    GLfloat ratio;

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        return;
    }
    if (!eglInitialize(display, nullptr, nullptr)) {
        return;
    }

    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
        // ShutDown();
        return;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        // ShutDown();
        return;
    }

    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    if (!(surface = eglCreateWindowSurface(display, config, window, 0))) {
        // ShutDown();
        return;
    }

    if (!(context = eglCreateContext(display, config, nullptr, contextAtt))) {
        // ShutDown();
        return;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        // ShutDown();
        return;
    }

    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width_) ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &height_)) {
        // ShutDown();
        return;
    }

    display_ = display;
    surface_ = surface;
    context_ = context;
    config_ = config;

    //CheckGlError("glBindFramebuffer");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    //CheckGlError("glClearColor");
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    //CheckGlError("glClear");
    glViewport(0, 0, width_, height_);
    //CheckGlError("glViewport");

    //surfaceTexture Id Used for rendering camera preview
    //CreateSurfaceTextureId();
}

void CardboardApp::CreateSurfaceTextureId() {
    glGenTextures(1, &surface_texture_id_);
    if (surface_texture_id_ > 0) {
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, surface_texture_id_);
        //CheckGlError("surfaceTexture glBindTexture");
        //Linear filter type without mipmap
        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Wrap clmap to edge
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

} // namespace cardboardapp