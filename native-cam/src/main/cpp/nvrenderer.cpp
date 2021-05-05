#include <thread>
#include <stdlib.h>
#include <android/native_window.h>

#include "nvrenderer.h"
#include "logger.h"
#include "global_interface.h"
#include "nv_cam_background.h"

#define LOG_TAG "NVRenderer"

// Default near clip plane z-axis coordinate.
static constexpr float kZNear = 0.1f;

// Default far clip plane z-axis coordinate.
static constexpr float kZFar = 100.f;

constexpr uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;

constexpr float kDefaultFloorHeight = -1.7f;

// The objects are about 1 meter in radius, so the min/max target distance are
// set so that the objects are always within the room (which is about 5 meters
// across) and the reticle is always closer than any objects.
constexpr float kMinTargetDistance = 2.5f;
constexpr float kMaxTargetDistance = 3.5f;
constexpr float kMinTargetHeight = 0.5f;
constexpr float kMaxTargetHeight = kMinTargetHeight + 3.0f;

using namespace ndk_hello_cardboard;

namespace nv
{
    namespace render
    {
        NVRenderer::NVRenderer():
                msg_(MSG_NONE),
                display_(0),
                surface_(0),
                context_(0),
                config_(0),
                cam_background_(0),
                width_(0),
                height_(0),
                surface_texture_id_(0),
                flip_background_(false),
                window_init_(false),
                pause_(false),
                // cardboard
                head_tracker_(nullptr),
                lens_distortion_(nullptr),
                distortion_renderer_(nullptr),
                screen_params_changed_(false),
                device_params_changed_(false),
                depthRenderBuffer_(0),
                framebuffer_(0),
                texture_(0)
        {
            InitCardboard();
        }

        NVRenderer::~NVRenderer() {

        }

        bool NVRenderer::UpdateDeviceParams() {
            // Checks if screen or device parameters changed
            if (!screen_params_changed_ && !device_params_changed_) {
                return true;
            }

            // Get saved device parameters
            uint8_t* buffer;
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

        void NVRenderer::GlSetup() {
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
                         GL_RGB, GL_UNSIGNED_BYTE, 0);

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

            //...

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

        void NVRenderer::GlTeardown() {
            if (framebuffer_ == 0) {
                return;
            }
            glDeleteRenderbuffers(1, &depthRenderBuffer_);
            depthRenderBuffer_ = 0;
            glDeleteFramebuffers(1, &framebuffer_);
            framebuffer_ = 0;
            glDeleteTextures(1, &texture_);
            texture_ = 0;
        }

        void NVRenderer::Resume() {
            pause_ = false;
            CardboardHeadTracker_resume(head_tracker_);
            LOGD("MEULOG resumiu head_tracker_: %d\n", head_tracker_);

            // Parameters may have changed.
            device_params_changed_ = true;

            // Check for device parameters existence in external storage. If they're
            // missing, we must scan a Cardboard QR code and save the obtained parameters.
            uint8_t* buffer;
            int size;
            CardboardQrCode_getSavedDeviceParams(&buffer, &size);
            if (size == 0) {
                SwitchViewer();
            }
            CardboardQrCode_destroy(buffer);
        }


        void NVRenderer::Pause() {
            pause_ = true;
            CardboardHeadTracker_pause(head_tracker_);
            LOGD("MEULOG pausou head_tracker_: %d\n", head_tracker_);
        }

        void NVRenderer::Destroy() {
            msg_ = MSG_LOOP_EXIT;
            CardboardHeadTracker_destroy(head_tracker_);
            head_tracker_ = nullptr;
            LOGD("MEULOG destruiu head_tracker_: %d\n", head_tracker_);
        }

        void NVRenderer::CreateSurface(jobject surface) {
            // TODO:
            // Target object first appears directly in front of user.
            model_target_ = GetTranslationMatrix({0.0f, 1.5f, kMinTargetDistance});
        }

        void NVRenderer::SetWindow(ANativeWindow *window) {
            if(window)
            {
                window_ = window;
                if(window_init_)
                {
                    msg_ = MSG_WINDOW_UPDATE;
                    return;
                }

                LOG_INFO("nv log renderer SetWindow Create");
                msg_ = MSG_WINDOW_CREATE;

                //Block the UI thread
                std::unique_lock<std::mutex> lk(mut_);
                cond_.wait(lk);
                lk.unlock();
                LOG_INFO("nv log renderer unblock ui thread");
            }else{
                msg_ = MSG_WINDOW_DESTROY;
            }

        }

        //Run on the gl thread
        void NVRenderer::_Run() {
            //LOG_INFO("nv log renderer run loop");
            _renderLoop();
        }

        //Create a gl context and surface
        bool NVRenderer::Initialise() {
            LOG_INFO("nv log renderer initialise");
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
            EGLint contextAtt[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
            EGLDisplay display;
            EGLConfig config;
            EGLint numConfigs;
            EGLint format;
            EGLSurface surface;
            EGLContext context;
            GLfloat ratio;

            if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
                LOG_ERROR("eglGetDisplay() returned error %d", eglGetError());
                return false;
            }
            if (!eglInitialize(display, 0, 0)) {
                LOG_ERROR("eglInitialize() returned error %d", eglGetError());
                return false;
            }

            if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
                LOG_ERROR("eglChooseConfig() returned error %d", eglGetError());
                ShutDown();
                return false;
            }

            if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
                LOG_ERROR("eglGetConfigAttrib() returned error %d", eglGetError());
                ShutDown();
                return false;
            }

            ANativeWindow_setBuffersGeometry(window_, 0, 0, format);

            if (!(surface = eglCreateWindowSurface(display, config, window_, 0))) {
                LOG_ERROR("eglCreateWindowSurface() returned error %d", eglGetError());
                ShutDown();
                return false;
            }

            if (!(context = eglCreateContext(display, config, 0, contextAtt))) {
                LOG_ERROR("eglCreateContext() returned error %d", eglGetError());
                ShutDown();
                return false;
            }

            if (!eglMakeCurrent(display, surface, surface, context)) {
                LOG_ERROR("eglMakeCurrent() returned error %d", eglGetError());
                ShutDown();
                return false;
            }

            if (!eglQuerySurface(display, surface, EGL_WIDTH, &width_) ||
                !eglQuerySurface(display, surface, EGL_HEIGHT, &height_)) {
                LOG_ERROR("eglQuerySurface() returned error %d", eglGetError());
                ShutDown();
                return false;
            }

            display_ = display;
            surface_ = surface;
            context_ = context;
            config_ = config;

            window_init_ = true;


            CheckGlError("glBindFramebuffer");
            glClearColor(0.0, 0.0, 0.0, 1.0);
            CheckGlError("glClearColor");
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            CheckGlError("glClear");
            glViewport(0, 0, width_, height_);
            CheckGlError("glViewport");

            //surfaceTexture Id Used for rendering camera preview
            CreateSurfaceTextureId();
            //Create camera background
            cam_background_ = new NVCameraBackground(this);

            msg_ = MSG_NONE;

            std::lock_guard<std::mutex> lk(mut_);
            cond_.notify_one();
            return true;
        }

        void NVRenderer::InitCardboard() {
            head_tracker_ = CardboardHeadTracker_create();
            LOGD("MEULOG criou head_tracker_: %d\n", head_tracker_);
        }

        void NVRenderer::CreateSurfaceTextureId() {
            glGenTextures(1, &surface_texture_id_);
            if(surface_texture_id_ >0)
            {
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, surface_texture_id_);
                CheckGlError("surfaceTexture glBindTexture");
                //Linear filter type without mipmap
                glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                //Wrap clmap to edge
                glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
        }

        void NVRenderer::SwitchViewer() {
            CardboardQrCode_scanQrCodeAndSaveDeviceParams();
        }

        void NVRenderer::FlipBackground(bool flip) {
            flip_background_ = flip;
        }

        bool NVRenderer::WindowRestore() {
            EGLContext  context;
            EGLSurface  surface;


            if (!(surface = eglCreateWindowSurface(display_, config_, window_, 0))) {
                LOG_ERROR("eglCreateWindowSurface() returned error %d", eglGetError());
                ShutDown();
                return false;
            }

            if (!eglMakeCurrent(display_, surface, surface, context)) {
                LOG_ERROR("eglMakeCurrent() returned error %d", eglGetError());
                ShutDown();
                return false;
            }

            surface_ = surface;
            context_ = context;
            msg_ = MSG_NONE;

            return true;
        }

        bool NVRenderer::WindowChanged() {
            LOG_INFO("Renderer update Window");

            if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
                LOG_ERROR("eglMakeCurrent() returned error %d", eglGetError());
                ShutDown();
                return false;
            }


            if (!eglQuerySurface(display_, surface_, EGL_WIDTH, &width_) ||
                !eglQuerySurface(display_, surface_, EGL_HEIGHT, &height_)) {
                LOG_ERROR("eglQuerySurface() returned error %d", eglGetError());
                ShutDown();
                return false;
            }

            msg_ = MSG_NONE;
            return true;
        }



        void NVRenderer::ShutDown() {

            if(cam_background_ != 0)
            {
                delete cam_background_;
                cam_background_ = 0;
            }

            LOG_INFO("Destroying context");

            eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroyContext(display_, context_);
            eglDestroySurface(display_, surface_);
            eglTerminate(display_);

            display_ = EGL_NO_DISPLAY;
            surface_ = EGL_NO_SURFACE;
            context_ = EGL_NO_CONTEXT;
            config_ = 0;
            window_ = 0;
            window_init_ = false;
            msg_ = MSG_NONE;

            return;
        }

        Matrix4x4 NVRenderer::GetPose() {
            std::array<float, 4> out_orientation;
            std::array<float, 3> out_position;
            long monotonic_time_nano = GetMonotonicTimeNano();
            monotonic_time_nano += kPredictionTimeWithoutVsyncNanos;
            CardboardHeadTracker_getPose(head_tracker_, monotonic_time_nano,
                                         &out_position[0], &out_orientation[0]);
            return GetTranslationMatrix(out_position) *
                   Quatf::FromXYZW(&out_orientation[0]).ToMatrix();
        }

        void NVRenderer::DrawFrame() {
            //LOG_INFO("nv log renderer drawframe");

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

            android_app_update_tex_image();

            if (cam_background_) {
                cam_background_->BeforeRender();

                // Draw camera on eye 1
                glViewport(0, 0, width_ / 2, height_);
                cam_background_->Render();

                // Draw camera on eye 2
                glViewport(width_ / 2, 0, width_ / 2, height_);
                cam_background_->Render();

                glViewport(0, 0, width_, height_);
                cam_background_->AfterRender();
            }

            // Render
            glViewport(0, 0, width_, height_);
            CardboardDistortionRenderer_renderEyeToDisplay(
                    distortion_renderer_, /* target_display = */ 0, /* x = */ 0, /* y = */ 0,
                    width_, height_, &left_eye_texture_description_,
                    &right_eye_texture_description_);
        }

        void NVRenderer::SwapBuffers() {
            if (!eglSwapBuffers(display_, surface_)) {
                LOG_ERROR("eglSwapBuffers() returned error %d", eglGetError());
            }
        }

        void NVRenderer::CheckGlError(const char *op) {
            for (GLint error = glGetError(); error; error
                                                            = glGetError()) {
                LOG_INFO("nv log renderer after %s() glError (0x%x)\n", op, error);
            }
        }

        GLuint NVRenderer::LoaderShader(GLenum shader_type, const char *source) {
            GLuint shader = glCreateShader(shader_type);
            if (shader) {
                glShaderSource(shader, 1, &source, NULL);
                glCompileShader(shader);
                GLint compiled = 0;
                glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
                if (!compiled) {
                    GLint infoLen = 0;
                    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                    if (infoLen) {
                        char* buf = (char*) malloc(infoLen);
                        if (buf) {
                            glGetShaderInfoLog(shader, infoLen, NULL, buf);
                            LOG_ERROR("Could not compile shader %d:\n%s\n",
                                      shader_type, buf);
                            free(buf);
                        }
                        glDeleteShader(shader);
                        shader = 0;
                    }
                }
            }
            return shader;
        }

        GLuint NVRenderer::CreateProgram(const char *vertex_source, const char *fragment_source) {
            GLuint vertex_shader = LoaderShader(GL_VERTEX_SHADER, vertex_source);
            if (!vertex_shader) {
                return 0;
            }

            GLuint pixel_shader = LoaderShader(GL_FRAGMENT_SHADER, fragment_source);
            if (!pixel_shader) {
                return 0;
            }

            GLuint program = glCreateProgram();
            if (program) {
                glAttachShader(program, vertex_shader);
                CheckGlError("glAttachShader");
                glAttachShader(program, pixel_shader);
                CheckGlError("glAttachShader");
                glLinkProgram(program);
                GLint linkStatus = GL_FALSE;
                glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
                if (linkStatus != GL_TRUE) {
                    GLint bufLength = 0;
                    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                    if (bufLength) {
                        char* buf = (char*) malloc(bufLength);
                        if (buf) {
                            glGetProgramInfoLog(program, bufLength, NULL, buf);
                            LOG_ERROR("Could not link program:\n%s\n", buf);
                            free(buf);
                        }
                    }
                    glDeleteProgram(program);
                    program = 0;
                }
            }




            return program;
        }


        void NVRenderer::_renderLoop() {
            bool run = true;
            while(run)
            {
                switch (msg_)
                {
                    case MSG_WINDOW_CREATE:
                        if(!window_init_)
                        {
                            if(!Initialise())
                            {
                                run = false;
                            }

                        }else{
                            if(WindowRestore())
                            {
                                run = false;
                            }
                        }
                        break;
                    case MSG_WINDOW_UPDATE:
                        WindowChanged();
                        break;
                    case MSG_WINDOW_DESTROY:
                        ShutDown();
                        break;
                    case MSG_LOOP_EXIT:
                        run = false;
                        break;
                    default:
                        break;
                }

                if(window_init_ && !pause_)
                {
                    DrawFrame();
                    SwapBuffers();
                }
            }
        }
    }//render
}
