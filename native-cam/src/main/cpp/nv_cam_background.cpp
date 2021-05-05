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
static const char glFragmentShader[] =
        "#extension GL_OES_EGL_image_external:require\n"
        "precision mediump float;\n"
        "varying vec2                texCoord;\n"
        "uniform samplerExternalOES  iChannel0;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(iChannel0, texCoord);\n"
        //"  vec4 color = texture2D(iChannel0, texCoord);\n"
        //"  gl_FragColor = color;\n"
        "}\n";

// basic deform
//static const char glBasicDeformFragmentShader[] =
//        "precision highp float;\n"
//        "uniform vec3                iResolution;\n"
//        "uniform float               iGlobalTime;\n"
//        "uniform sampler2D           iChannel0;\n"
//        "varying vec2                texCoord;\n"
//        "void mainImage( out vec4 fragColor, in vec2 fragCoord )\n"
//        "{\n"
//        "	float stongth = 0.3;\n"
//        "	vec2 uv = fragCoord.xy;\n"
//        "	float waveu = sin((uv.y + iGlobalTime) * 20.0) * 0.5 * 0.05 * stongth;\n"
//        "	fragColor = texture2D(iChannel0, uv + vec2(waveu, 0));\n"
//        "}\n"
//        "void main() {\n"
//        "	mainImage(gl_FragColor, texCoord);\n"
//        "}\n";

static const char glNostalgiaFragmentShader[] =
        "precision highp float;\n"
        "uniform sampler2D           iChannel0;\n"
        "varying vec2                texCoord;\n"
        "void mainImage( out vec4 fragColor, in vec2 fragCoord )\n"
        "{\n"
        "    vec4 mask = texture2D(iChannel0, fragCoord);\n"
        "    vec4 tempColor = vec4(0.393 * mask.r + 0.769 * mask.g + 0.189 * mask.b,\n"
        "                          0.349 * mask.r + 0.686 * mask.g + 0.168 * mask.b,\n"
        "                          0.272 * mask.r + 0.534 * mask.g + 0.131 * mask.b, 1.0);\n"
        "    fragColor = tempColor;\n"
        "}\n"
        "void main() {\n"
        "    mainImage(gl_FragColor, texCoord);\n"
        "}\n";

//static const char glFragmentShader[] =
//        "precision highp float;\n"
//        "uniform vec3                iResolution;\n"
//        "uniform sampler2D           iChannel1;\n"
//        "varying vec2                texCoord;\n"
//        "float c = 0.02; //amout of blocks = c*iResolution.x\n"
//        "void mainImage( out vec4 fragColor, in vec2 fragCoord ){\n"
//        "    //blocked pixel coordinate\n"
//        "    vec2 middle = floor(fragCoord*c+.5)/c;\n"
//        "    vec3 color = texture2D(iChannel1, middle/iResolution.xy).rgb;\n"
//        "    //lego block effects\n"
//        "        //stud\n"
//        "        float dis = distance(fragCoord,middle)*c*2.;\n"
//        "        if(dis<.65&&dis>.55){\n"
//        "            color *= dot(vec2(0.707),normalize(fragCoord-middle))*.5+1.;\n"
//        "        }\n"
//        "        //side shadow\n"
//        "        vec2 delta = abs(fragCoord-middle)*c*2.;\n"
//        "        float sdis = max(delta.x,delta.y);\n"
//        "        if(sdis>.9){\n"
//        "            color *= .8;\n"
//        "        }\n"
//        "	fragColor = vec4(color,1.0);\n"
//        "}\n"
//        "void main() {\n"
//        "	mainImage(gl_FragColor, texCoord*iResolution.xy);\n"
//        "}\n";

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
            program_id_ = renderer->CreateProgram(glVertexShader, glFragmentShader);
            if (!program_id_) {
                LOG_ERROR("CameraBackground Could not create program.");
            }
            //Get variables from glsl
            position_handle_ = glGetAttribLocation(program_id_, "vPosition");


//            int vPositionLocation = GLES20.glGetAttribLocation(PROGRAM, "vPosition");
//            GLES20.glEnableVertexAttribArray(vPositionLocation);
//            GLES20.glVertexAttribPointer(vPositionLocation, 2, GLES20.GL_FLOAT, false, 4 * 2, VERTEX_BUF);

            uv_handle_ = glGetAttribLocation(program_id_, "vTexCoord");
            texture_handle_ = glGetUniformLocation(program_id_, "iChannel0");



            // Effect texture
            effect_program_id = renderer->CreateProgram(glVertexShader, glNostalgiaFragmentShader);
            glGenTextures(1, &texture_effect);
            glBindTexture(GL_TEXTURE_2D, texture_effect);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
////            glGenFramebuffersOES(1, &indice_id_);
////            glBindFramebufferOES(GL_FRAMEBUFFER_OES, ResultFBO);
////            glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES,
////                                      GL_COLOR_ATTACHMENT0_OES,
////                                      GL_TEXTURE_2D,
////                                      ResultTexture,
////                                      0);
//
//            simpleTriangleProgram = ndk_hello_cardboard::createProgram(glVertexShader, glFragmentShader);
//            if (!simpleTriangleProgram) {
//                LOG_ERROR("Could not create program");
//            } else {
//                vPosition = glGetAttribLocation(simpleTriangleProgram, "vPosition");
//            }
        }

        NVCameraBackground::~NVCameraBackground() {

        }

        void NVCameraBackground::BeforeRender() {
            glUseProgram(program_id_);
            //glUseProgram(simpleTriangleProgram);

            texture_handle_ = glGetUniformLocation(program_id_, "iChannel0");

            // OK WITH samplerExternalOES
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, renderer_->GetSurfaceTextureId());
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(texture_handle_, 0);

            // USING Sampler2D
//            glUniform1i(texture_handle_, 0);
//            glActiveTexture(GL_TEXTURE0);
//            glBindTexture(GL_TEXTURE_2D, renderer_->GetSurfaceTextureId());

//            glUniform1i(texture_handle_, 0);
//            glActiveTexture(GL_TEXTURE0);
//            glBindTexture(GL_TEXTURE_2D, renderer_->GetSurfaceTextureId());

//            auto iResolutionLocation = glGetUniformLocation(program_id_, "iResolution");
//            auto *iResolution = new GLfloat[1280.0f, 720.0f, 1.0f];
//            glUniform3fv(iResolutionLocation, 1, iResolution);
//            delete[] iResolution;

            position_handle_ = glGetAttribLocation(program_id_, "vPosition");
            glEnableVertexAttribArray(position_handle_);
            glVertexAttribPointer(position_handle_, 2, GL_FLOAT, GL_FALSE, 4 * 2, kTriangleVertices);

            uv_handle_ = glGetAttribLocation(program_id_, "vTexCoord");
            glEnableVertexAttribArray(uv_handle_);
            glVertexAttribPointer(uv_handle_, 2, GL_FLOAT, GL_FALSE, 4 * 2, kUvs);

            //glEnableVertexAttribArray(vTexCoordLocation);
            //glVertexAttribPointer(vTexCoordLocation, 2, GLES20.GL_FLOAT, false, 4 * 2, ROATED_TEXTURE_COORD_BUF);




//            int iResolutionLocation = glGetUniformLocation(program_id_, "iResolution");
//            GLfloat *iResolution = new GLfloat[1280.0f, 720.0f, 1.0f];
//            glUniform3fv(iResolutionLocation, 1, iResolution);
//
//            //float time = ((float) (System.currentTimeMillis() - START_TIME)) / 1000.0f;
//            struct timeval time_now{};
//            gettimeofday(&time_now, nullptr);
//            time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
//
//            int iGlobalTimeLocation = glGetUniformLocation(program_id_, "iGlobalTime");
//            glUniform1f(iGlobalTimeLocation, msecs_time);
//
////            int iFrameLocation = glGetUniformLocation(program_id_, "iFrame");
////            glUniform1i(iFrameLocation, iFrame);
//
//            int vPositionLocation = glGetAttribLocation(program_id_, "vPosition");
//            glEnableVertexAttribArray(vPositionLocation);
//            glVertexAttribPointer(vPositionLocation, 2, GL_FLOAT, false, 4 * 2, kTriangleVertices);
//
//            int vTexCoordLocation = glGetAttribLocation(program_id_, "vTexCoord");
//            glEnableVertexAttribArray(vTexCoordLocation);
//            glVertexAttribPointer(vTexCoordLocation, 2, GL_FLOAT, false, 4 * 2, kUvs);
//
//            //for (int i = 0; i < iChannels.length; i++) {
//                int sTextureLocation = glGetUniformLocation(program_id_, "iChannel0");
//                glActiveTexture(GL_TEXTURE0);
//                glBindTexture(GL_TEXTURE_2D, renderer_->GetSurfaceTextureId());
//                glUniform1i(sTextureLocation, 0);
            //}

//            float _iChannelResolutions[] = new float[iChannelResolutions.length * 3];
//            for (int i = 0; i < iChannelResolutions.length; i++) {
//                _iChannelResolutions[i * 3] = iChannelResolutions[i][0];
//                _iChannelResolutions[i * 3 + 1] = iChannelResolutions[i][1];
//                _iChannelResolutions[i * 3 + 2] = 1.0f;
//            }
//
//            int iChannelResolutionLocation = glGetUniformLocation(program_id_, "iChannelResolution");
//            glUniform3fv(iChannelResolutionLocation,
//                         _iChannelResolutions.length, FloatBuffer.wrap(_iChannelResolutions));

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indice_id_);





            //glUseProgram(program_id_);



//            glUseProgram(effect_program_id);
//            int texLocation = glGetUniformLocation(effect_program_id, "iChannel1");
//            glUniform1i(texLocation, 1);
//            glActiveTexture(GL_TEXTURE1);
//            glBindTexture(GL_TEXTURE_2D, texture_effect);
//
//            position_handle_ = glGetAttribLocation(effect_program_id, "vPosition");
//            glEnableVertexAttribArray(position_handle_);
//            glVertexAttribPointer(position_handle_, 2, GL_FLOAT, GL_FALSE, 4 * 2, kTriangleVertices);
//
//            uv_handle_ = glGetAttribLocation(effect_program_id, "vTexCoord");
//            glEnableVertexAttribArray(uv_handle_);
//            glVertexAttribPointer(uv_handle_, 2, GL_FLOAT, GL_FALSE, 4 * 2, kUvs);

            //---------------------------------------------------
            //glUseProgram(simpleTriangleProgram);
//            glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, kTriangleVertices);
//            glEnableVertexAttribArray(vPosition);
//            glDrawArrays(GL_TRIANGLES, 0, 3);
            //---------------------------------------------------
        }

        void NVCameraBackground::Render() {
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            glUseProgram(effect_program_id);
            int texLocation = glGetUniformLocation(effect_program_id, "iChannel1");
            glUniform1i(texLocation, 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture_effect);

            position_handle_ = glGetAttribLocation(effect_program_id, "vPosition");
            glEnableVertexAttribArray(position_handle_);
            glVertexAttribPointer(position_handle_, 2, GL_FLOAT, GL_FALSE, 4 * 2, kTriangleVertices);

            uv_handle_ = glGetAttribLocation(effect_program_id, "vTexCoord");
            glEnableVertexAttribArray(uv_handle_);
            glVertexAttribPointer(uv_handle_, 2, GL_FLOAT, GL_FALSE, 4 * 2, kUvs);
        }

        void NVCameraBackground::AfterRender() {


            // OK WITH samplerExternalOES
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            // USING Sampler2D
//            glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
//            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
}