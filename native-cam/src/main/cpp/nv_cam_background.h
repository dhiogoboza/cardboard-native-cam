#pragma once

#include <GLES3/gl32.h>
#include <string>
#include "util.h"

namespace nv::render {

class NVRenderer;

class NVCameraBackground {
public:
    NVCameraBackground(NVRenderer *renderer, const ndk_hello_cardboard::VRConfigs &configs);

    ~NVCameraBackground();

    void BeforeRender();

    void Render();

    void AfterRender();

    void SetConfigs(const ndk_hello_cardboard::VRConfigs &configs);

private:
    NVRenderer *renderer_;
    GLuint vertex_id_;
    GLuint uv_id_;
    GLuint index_id_;
    GLuint program_id_;
    GLuint position_handle_;
    GLuint resolution_handle_;
    GLuint uv_handle_;
    GLuint texture_handle_;
};

} // namespace nv::render
