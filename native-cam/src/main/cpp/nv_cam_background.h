#pragma once

#include <GLES3/gl32.h>

namespace nv::render {

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
    GLuint  index_id_;
    GLuint  program_id_;
    GLuint  position_handle_;
    GLuint  resolution_handle_;
    GLuint  uv_handle_;
    GLuint  texture_handle_;
};

} // namespace nv::render
