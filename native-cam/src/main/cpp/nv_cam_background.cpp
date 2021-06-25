#include "nv_cam_background.h"

#include "nvrenderer.h"
#include "logger.h"
#include "util.h"

static const GLfloat kTriangleVertices[] = { // in counterclockwise order:
        -1.0f, -1.0f,   // 0.left - mid
        1.0f, -1.0f,   // 1. right - mid
        -1.0f, 1.0f,   // 2. left - top
        1.0f, 1.0f,   // 3. right - top
};

static const GLfloat kUvs[] = {
        0.0f, 1.0f,  // A. left-bottom
        1.0f, 1.0f,  // B. right-bottom
        0.0f, 0.0f,  // C. left-top
        1.0f, 0.0f   // D. right-top
};

static const GLushort kIndices[] = {0, 1, 2, 2, 1, 3}; // order to draw vertices

static const char glVertexShader[] =
        "attribute vec2  vPosition;\n"
        "attribute vec2  vTexCoord;\n"
        "varying vec2    texCoord;\n"
        "void main() {\n"
        "    texCoord = vTexCoord;\n"
        "    gl_Position = vec4 ( vPosition.x, vPosition.y, 0.0, 1.0 );\n"
        "}\n";

namespace nv::render {

NVCameraBackground::NVCameraBackground(NVRenderer *renderer, const ndk_hello_cardboard::VRConfigs &configs)
        : renderer_(renderer)
        , program_id_(-1)
        , vertex_id_(-1)
        , uv_id_(-1)
        , index_id_(-1)
    {
    //bind data to gpu
    glGenBuffers(1, &vertex_id_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_id_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, kTriangleVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &uv_id_);
    glBindBuffer(GL_ARRAY_BUFFER, uv_id_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, kUvs, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &index_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_id_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, kIndices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //create program
    program_id_ = renderer_->CreateProgram(glVertexShader, configs.shader.c_str());
    if (!program_id_) {
        LOG_ERROR("CameraBackground Could not create program.");
    }

    //Get variables from glsl
    position_handle_ = glGetAttribLocation(program_id_, "vPosition");
    resolution_handle_ = glGetAttribLocation(program_id_, "iResolution");
    uv_handle_ = glGetAttribLocation(program_id_, "vTexCoord");
    texture_handle_ = glGetUniformLocation(program_id_, "iChannel0");
}

NVCameraBackground::~NVCameraBackground() = default;

void NVCameraBackground::BeforeRender() {
    glUseProgram(program_id_);

    glBindTexture(GL_TEXTURE_EXTERNAL_OES, renderer_->GetSurfaceTextureId());
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(texture_handle_, 0);

    glVertexAttribPointer(position_handle_, 2, GL_FLOAT, GL_FALSE, 0, kTriangleVertices);
    glEnableVertexAttribArray(position_handle_);

    glVertexAttribPointer(uv_handle_, 2, GL_FLOAT, GL_FALSE, 0, kUvs);

    float _iChannelResolutions[] = {1280.0f, 720.0f, 1.0f};
    glUniform3fv(resolution_handle_, 1, _iChannelResolutions);

    glEnableVertexAttribArray(uv_handle_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_id_);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"
void NVCameraBackground::Render() {
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void NVCameraBackground::AfterRender() {
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
#pragma clang diagnostic pop

void NVCameraBackground::SetConfigs(const ndk_hello_cardboard::VRConfigs &configs) {
    glDeleteProgram(program_id_);

    // create program
    program_id_ = renderer_->CreateProgram(glVertexShader, configs.shader.c_str());
    if (!program_id_) {
        LOG_ERROR("CameraBackground Could not create program.");
    }
}

} // namespace nv::render
