//
// Created by root on 18-8-10.
//

#ifndef FACE_TRACKER_ANDROID_NV_CAM_BACKGROUND_H
#define FACE_TRACKER_ANDROID_NV_CAM_BACKGROUND_H
#include <GLES2/gl2.h>
//#include <GLES3/gl32.h>

namespace nv
{
    namespace render
    {
        class NVRenderer;
        class NVCameraBackground {
        public:
            NVCameraBackground(NVRenderer *renderer);
            ~NVCameraBackground();

            void BeforeRender();
            void Render();
            void AfterRender();

        private:
            NVRenderer *renderer_;
            GLuint  vertex_id_;
            GLuint  uv_id_;
            GLuint  indice_id_;
            GLuint  program_id_;
            GLuint  position_handle_;
            GLuint  uv_handle_;
            GLuint  texture_handle_;

            GLuint  effect_program_id;
            GLuint  texture_effect;
        };
    }

}

#endif //FACE_TRACKER_ANDROID_NV_CAM_BACKGROUND_H
