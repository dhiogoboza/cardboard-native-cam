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

//static auto kFragmentShader =
//        "#extension GL_OES_EGL_image_external:require\n"
//        "precision mediump float;\n"
//                "varying vec2 oUv;\n"
//                "uniform samplerExternalOES uTexture;\n"
//                "void main() {\n"
//                "  vec4 color = texture2D(uTexture, oUv);\n"
//                "  gl_FragColor = color;\n"
//                "}\n";

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

//static auto kVertexShader =
//        "attribute vec4 vPosition;\n"
//        "attribute vec2 vUv;\n"
//        "varying vec2 oUv;\n"
//        "void main() {\n"
//        "oUv = vUv;\n"
//        "  gl_Position = vPosition;\n"
//        "}\n";

//static const char glVertexShader[] =
//        "attribute vec2  vPosition;\n"
//        "attribute vec2  vTexCoord;\n"
//        "varying vec2    texCoord;\n"
//        "void main() {\n"
//        "    texCoord = vTexCoord;\n"
//        "    gl_Position = vec4 ( vPosition.x, vPosition.y, 0.0, 1.0 );\n"
//        "}\n\0";

static const char glVertexShader[] =
        "attribute vec2  vPosition;\n"
        "attribute vec2  vTexCoord;\n"
        "varying vec2    texCoord;\n"
        "void main() {\n"
        "    texCoord = vTexCoord;\n"
        "    gl_Position = vec4 ( vPosition.x, vPosition.y, 0.0, 1.0 );\n"
        "}\n";

//static auto kFragmentShader =
//        "#extension GL_OES_EGL_image_external:require\n"
//        "precision mediump float;\n"
//        "varying vec2 texCoord;\n"
//        "uniform samplerExternalOES uTexture;\n"
//        "void main() {\n"
//        "  vec4 color = texture2D(uTexture, texCoord);\n"
//        "  gl_FragColor = color;\n"
//        "}\n";

//static auto kFragmentShader =
//        "#extension GL_OES_EGL_image_external:require\n"
//        "precision mediump float;\n"
//                "varying vec2 oUv;\n"
//                "uniform samplerExternalOES uTexture;\n"
//                "void main() {\n"
//                "  vec4 color = texture2D(uTexture, oUv);\n"
//                "  gl_FragColor = color;\n"
//                "}\n";

// https://stackoverflow.com/questions/47381609/android-native-activity-gles-3-0-shader-compilations-fails
//static const char glFragmentShader[] =
//        //"#extension GL_OES_EGL_image_external:require\n"
//        "precision mediump float;\n"
//        //"uniform samplerExternalOES uTexture;\n"
//        "uniform vec3                iResolution;\n"
//        "uniform float               iGlobalTime;\n"
//        "uniform sampler2D           iChannel0;\n"
//        "varying vec2                texCoord;\n"
//        "void mainImage( out vec4 fragColor, in vec2 fragCoord )\n"
//        "{\n"
//        "    vec2 uv = fragCoord.xy;\n"
//        "    float amount = 0.0;\n"
//        "   amount = (1.0 + sin(iGlobalTime*6.0)) * 0.5;\n"
//        "   amount *= 1.0 + sin(iGlobalTime*16.0) * 0.5;\n"
//        "   amount *= 1.0 + sin(iGlobalTime*19.0) * 0.5;\n"
//        "   amount *= 1.0 + sin(iGlobalTime*27.0) * 0.5;\n"
//        "   amount = pow(amount, 3.0);\n"
//        "   amount *= 0.05;\n"
//        "   vec3 col;\n"
//        "   col.r = texture2D( iChannel0, vec2(uv.x+amount,uv.y) ).r;\n"
//        "   col.g = texture2D( iChannel0, uv ).g;\n"
//        "   col.b = texture2D( iChannel0, vec2(uv.x-amount,uv.y) ).b;\n"
//        "   col *= (1.0 - amount * 0.5);\n"
//        "   fragColor = vec4(col,1.0);\n"
//        "}\n"
//        "void main() {\n"
//        "    mainImage(gl_FragColor, texCoord);\n"
//        "}\n";

// raw
//static auto kFragmentShader =
//        "#extension GL_OES_EGL_image_external:require\n"
//        "precision mediump float;\n"
//                "varying vec2 oUv;\n"
//                "uniform samplerExternalOES uTexture;\n"
//                "void main() {\n"
//                "  vec4 color = texture2D(uTexture, oUv);\n"
//                "  gl_FragColor = color;\n"
//                "}\n";
// OK WITH samplerExternalOES
//static const char glFragmentShader[] =
//        "#extension GL_OES_EGL_image_external : require\n"
//        "precision mediump float;\n"
//        "varying vec2                texCoord;\n"
//        "uniform samplerExternalOES  iChannel0;\n"
//        "void main() {\n"
//        //"    gl_FragColor = texture2D(iChannel0, texCoord);\n"
//        "  vec4 color = texture2D(iChannel0, texCoord);\n"
//        "  gl_FragColor = color;\n"
//        "}\n";
// USING sampler2D
//static const char glFragmentShader[] =
//        "#extension GL_OES_EGL_image_external:require\n"
//        "precision mediump float;\n"
//        "varying vec2                texCoord;\n"
//        "uniform samplerExternalOES  iChannel0;\n"
//        "void main() {\n"
//        "    gl_FragColor = texture2D(iChannel0, texCoord);\n"
//        //"  vec4 color = texture2D(iChannel0, texCoord);\n"
//        //"  gl_FragColor = color;\n"
//        "}\n";

// RAW
//static const char glFragmentShader[] =
//        "#extension GL_OES_EGL_image_external:require\n"
//        "precision mediump float;\n"
//        "varying vec2                texCoord;\n"
//        "uniform samplerExternalOES  iChannel0;\n"
//        "void main() {\n"
//        "    gl_FragColor = texture2D(iChannel0, texCoord);\n"
//        "}\n";

// PIXELIZE
static const char glFragmentShader[] =
        "#extension GL_OES_EGL_image_external:require\n"
        "precision mediump float;\n"
        "uniform vec3                iResolution;\n"
        "uniform samplerExternalOES           iChannel0;\n"
        "varying vec2                texCoord;\n"
        "#define S (1280.0f / 6e1) // The cell size.\n"
        "void mainImage(out vec4 c, vec2 p)\n"
        "{\n"
        "            c = texture2D(iChannel0, floor((p + .5) / S) * S / (1280.0f, 720.0f));\n"
        "}\n"
        "void main() {\n"
        "    mainImage(gl_FragColor, texCoord*(1280.0f, 720.0f));\n"
        "}\n";

// NOSTALGIA
//static const char glFragmentShader[] =
//        "#extension GL_OES_EGL_image_external:require\n"
//        "precision highp float;\n"
//        "uniform samplerExternalOES           iChannel0;\n"
//        "varying vec2                texCoord;\n"
//        "void mainImage( out vec4 fragColor, in vec2 fragCoord )\n"
//        "{\n"
//        "    vec4 mask = texture2D(iChannel0, fragCoord);\n"
//        "    vec4 tempColor = vec4(0.393 * mask.r + 0.769 * mask.g + 0.189 * mask.b,\n"
//        "                          0.349 * mask.r + 0.686 * mask.g + 0.168 * mask.b,\n"
//        "                          0.272 * mask.r + 0.534 * mask.g + 0.131 * mask.b, 1.0);\n"
//        "    fragColor = tempColor;\n"
//        "}\n"
//        "void main() {\n"
//        "    mainImage(gl_FragColor, texCoord);\n"
//        "}\n";

namespace nv
{
    namespace render
    {
        NVCameraBackground::NVCameraBackground(NVRenderer *renderer)
            : renderer_(renderer)
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
            program_id_ = renderer->CreateProgram(glVertexShader, glFragmentShader);
            if (!program_id_) {
                LOG_ERROR("CameraBackground Could not create program.");
            }
            //Get variables from glsl
            position_handle_ = glGetAttribLocation(program_id_, "vPosition");
            resolution_handle_ = glGetAttribLocation(program_id_, "iResolution");
            uv_handle_ = glGetAttribLocation(program_id_, "vTexCoord");
            texture_handle_ = glGetUniformLocation(program_id_, "iChannel0");
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

            float _iChannelResolutions[] = {1280.0f, 720.0f, 1.0f};
            //glVertexAttribPointer(resolution_handle_, 1, GL_FLOAT, GL_FALSE, 0, _iChannelResolutions);
            //glEnableVertexAttribArray(resolution_handle_);
            glUniform3fv(resolution_handle_, 1, _iChannelResolutions);

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
