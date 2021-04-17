#ifndef NV_RENDERER_H_
#define NV_RENDERER_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <condition_variable>

#include "cardboard.h"
#include "util.h"

namespace nv
{
    namespace render
    {
        class NVCameraBackground;
        class NVRenderer
        {
        public:
            NVRenderer();
            ~NVRenderer();

            void Resume();

            void Pause();

            void Destroy();

            void CreateSurface(jobject surface);

            void SetWindow(ANativeWindow* window);

            void FlipBackground(bool flip);

            GLuint GetSurfaceTextureId(){return surface_texture_id_;}

            void CheckGlError(const char *op);

            GLuint LoaderShader(GLenum shader_type, const char* source);
            GLuint CreateProgram(const char* vertex_source, const char* fragment_source);

            void _Run();

            bool UpdateDeviceParams();

            void SwitchViewer();

        protected:
            bool Initialise();

            bool WindowRestore();

            bool WindowChanged();

            void CreateSurfaceTextureId();

            void ShutDown();

            ndk_hello_cardboard::Matrix4x4 GetPose();

            void DrawFrame();

            void RenderCamera();

            void SwapBuffers();

            void InitCardboard();

        private:
            void _renderLoop();

            void GlSetup();

            void GlTeardown();


        private:
            enum RenderMessage{
                MSG_NONE = 0,
                MSG_WINDOW_CREATE,
                MSG_WINDOW_UPDATE,
                MSG_WINDOW_DESTROY,
                MSG_LOOP_EXIT
            };

            enum RenderMessage msg_;

            //Android Window
            ANativeWindow* window_;

            //EGL Resources
            EGLDisplay display_;
            EGLSurface surface_;
            EGLContext context_;
            EGLConfig  config_;

            NVCameraBackground *cam_background_;

            std::mutex mut_;
            std::condition_variable cond_;

            int width_;
            int height_;

            GLuint surface_texture_id_;
            bool flip_background_;

            bool window_init_;
            bool pause_;


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

            GLuint depthRenderBuffer_;  // depth buffer
            GLuint framebuffer_;        // framebuffer object
            GLuint texture_;            // distortion texture

            ndk_hello_cardboard::Matrix4x4 head_view_;
            ndk_hello_cardboard::Matrix4x4 model_target_;
        };
    }
}


#endif