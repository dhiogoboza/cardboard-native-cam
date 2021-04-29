//
// Created by root on 18-8-10.
//

#include "nv_cam_background.h"

#define LOG_TAG "NVCameraBackground"

#include "nvrenderer.h"
#include "logger.h"
#include "util.h"

static auto kVertexShader =
        "attribute vec4 vPosition;\n"
                "attribute vec2 vUv;\n"
                "varying vec2 oUv;\n"
                "void main() {\n"
                "oUv = vUv;\n"
                "  gl_Position = vPosition;\n"
                "}\n";

static auto kFragmentShader =
        "#extension GL_OES_EGL_image_external:require\n"
        "precision mediump float;\n"
                "varying vec2 oUv;\n"
                "uniform samplerExternalOES uTexture;\n"
                "void main() {\n"
                "  vec4 color = texture2D(uTexture, oUv);\n"
                "  gl_FragColor = color;\n"
                "}\n";

//static const GLfloat kTriangleVertices[] = { -1.0f, -1.0f, -1.0f, 1.0f,
//                                      1.0f, 1.0f, 1.0f, -1.0f };

static const GLfloat kTriangleVertices[] = { // in counterclockwise order:
        -1.0f, -1.0f,   // 0.left - mid
        1.0f, -1.0f,   // 1. right - mid
        -1.0f, 1.0f,   // 2. left - top
        1.0f, 1.0f,   // 3. right - top
//
//    	 -1.0f, -1.0f, //4. left - bottom
//    	 1.0f , -1.0f, //5. right - bottom


//       -1.0f, -1.0f,  // 0. left-bottom
//        0.0f, -1.0f,   // 1. mid-bottom
//       -1.0f,  1.0f,   // 2. left-top
//        0.0f,  1.0f,   // 3. mid-top

        //1.0f, -1.0f,  // 4. right-bottom
        //1.0f, 1.0f,   // 5. right-top

};

//static const GLfloat kUvs[] = {1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0};
//static const GLfloat kUvs[] = {1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0};

// UVs of the quad vertices (S, T)
//const GLfloat kUvs[] = {
//        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
//};

// static const GLfloat kUvs_flip[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0}; //bkp



//static const GLfloat kUvs[] =      {1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0};
//
//static const GLfloat kUvs_flip[] = {1.0, 0.0, 1.0, 1.0,
//                                    0.0, 1.0, 0.0, 0.0};

//const GLfloat kUvs_flip[] = {
//        -1.0f, -1.0f, +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f
//};

//static const GLfloat kUvs[] = {1.0, 1.0, 0.0, 1.0,
//                               0.0, 0.0, 1.0, 0.0};

//static const GLfloat kUvs[] = {0.0, 1.0, 1.0, 1.0,
//                               1.0, 0.0, 0.0, 0.0};

static const GLfloat kUvs[] = {
        0.0f, 1.0f,  // A. left-bottom
        1.0f, 1.0f,  // B. right-bottom
        0.0f, 0.0f,  // C. left-top
        1.0f, 0.0f   // D. right-top
//
//        1.0f,  1.0f,
//        1.0f,  0.0f,
//        0.0f,  1.0f,
//        0.0f,  0.0f
};

//static const GLushort kIndices[] = {0, 3, 1, 1, 3, 2};

static const GLushort kIndices[] = {0, 1, 2, 2, 1, 3}; // order to draw vertices

namespace nv
{
    namespace render
    {
        NVCameraBackground::NVCameraBackground(NVRenderer *renderer):
            renderer_(renderer)
        {
            //bind data to gpu
            glGenBuffers(1, &vertex_id_);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_id_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*8, kTriangleVertices, GL_STATIC_DRAW);

            glGenBuffers(1, &uv_id_);
            glBindBuffer(GL_ARRAY_BUFFER, uv_id_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*8, kUvs, GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &indice_id_);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indice_id_);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*6, kIndices, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            //create program
            program_id_ = renderer->CreateProgram(kVertexShader, kFragmentShader);
            if (!program_id_) {
                LOG_ERROR("CameraBackground Could not create program.");
            }
            //Get variables from glsl
            position_handle_ = glGetAttribLocation(program_id_, "vPosition");

            uv_handle_ = glGetAttribLocation(program_id_, "vUv");
            texture_handle_ = glGetUniformLocation(program_id_, "uTexture");

        }

        NVCameraBackground::~NVCameraBackground() {

        }

        void NVCameraBackground::BeforeRender() {
            glUseProgram(program_id_);

            glBindTexture(GL_TEXTURE_EXTERNAL_OES, renderer_->GetSurfaceTextureId());
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(texture_handle_, 0);

            glVertexAttribPointer(position_handle_, 2, GL_FLOAT, GL_FALSE, 0, kTriangleVertices);
            glEnableVertexAttribArray(position_handle_);

            glVertexAttribPointer(uv_handle_, 2, GL_FLOAT, GL_FALSE, 0, kUvs);

            glEnableVertexAttribArray(uv_handle_);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indice_id_);
        }

        void NVCameraBackground::Render() {
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        }

        void NVCameraBackground::AfterRender() {
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
}