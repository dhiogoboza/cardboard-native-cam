/*
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "util.h"

#include <android/log.h>
#include <string.h>  // Needed for strtok_r and strstr
#include <time.h>
#include <unistd.h>

#include <array>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <random>
#include <sstream>
#include <string>

#include <GLES3/gl32.h>

namespace ndk_hello_cardboard {

namespace {

class RunAtEndOfScope {
 public:
  RunAtEndOfScope(std::function<void()> function) : function_(function) {}

  ~RunAtEndOfScope() { function_(); }

 private:
  std::function<void()> function_;
};

/**
 * Loads a png file from assets folder and then assigns it to the OpenGL target.
 * This method must be called from the renderer thread since it will result in
 * OpenGL calls to assign the image to the texture target.
 *
 * @param env The JNIEnv to use.
 * @param java_asset_mgr The asset manager object.
 * @param target OpenGL texture target to load the image into.
 * @param path Path to the file, relative to the assets folder.
 * @return true if png is loaded correctly, otherwise false.
 */
bool LoadPngFromAssetManager(JNIEnv* env, jobject java_asset_mgr, int target,
                             const std::string& path) {
  jclass bitmap_factory_class =
      env->FindClass("android/graphics/BitmapFactory");
  jclass asset_manager_class =
      env->FindClass("android/content/res/AssetManager");
  jclass gl_utils_class = env->FindClass("android/opengl/GLUtils");
  jmethodID decode_stream_method = env->GetStaticMethodID(
      bitmap_factory_class, "decodeStream",
      "(Ljava/io/InputStream;)Landroid/graphics/Bitmap;");
  jmethodID open_method = env->GetMethodID(
      asset_manager_class, "open", "(Ljava/lang/String;)Ljava/io/InputStream;");
  jmethodID tex_image_2d_method = env->GetStaticMethodID(
      gl_utils_class, "texImage2D", "(IILandroid/graphics/Bitmap;I)V");

  jstring j_path = env->NewStringUTF(path.c_str());
  RunAtEndOfScope cleanup_j_path([&] {
    if (j_path) {
      env->DeleteLocalRef(j_path);
    }
  });

  jobject image_stream =
      env->CallObjectMethod(java_asset_mgr, open_method, j_path);
  jobject image_obj = env->CallStaticObjectMethod(
      bitmap_factory_class, decode_stream_method, image_stream);
  if (env->ExceptionOccurred() != nullptr) {
    LOGE("Java exception while loading image");
    env->ExceptionClear();
    image_obj = nullptr;
    return false;
  }

  env->CallStaticVoidMethod(gl_utils_class, tex_image_2d_method, target, 0,
                            image_obj, 0);
  return true;
}

/**
 * Calculates vector norm
 *
 * @param vec Vector
 * @return Norm value
 */
float VectorNorm(const std::array<float, 4>& vec) {
  return std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

/**
 * Calculates dot product of two vectors
 *
 * @param vec1 First vector
 * @param vec2 Second vector
 * @return Dot product value.
 */
float VectorDotProduct(const std::array<float, 4>& vec1,
                       const std::array<float, 4>& vec2) {
  float product = 0;
  for (int i = 0; i < 3; i++) {
    product += vec1[i] * vec2[i];
  }
  return product;
}

}  // anonymous namespace

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& right) {
  Matrix4x4 result;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result.m[i][j] = 0.0f;
      for (int k = 0; k < 4; ++k) {
        result.m[i][j] += this->m[k][j] * right.m[i][k];
      }
    }
  }
  return result;
}

std::array<float, 4> Matrix4x4::operator*(const std::array<float, 4>& vec) {
  std::array<float, 4> result;
  for (int i = 0; i < 4; ++i) {
    result[i] = 0;
    for (int k = 0; k < 4; ++k) {
      result[i] += this->m[k][i] * vec[k];
    }
  }
  return result;
}

std::array<float, 16> Matrix4x4::ToGlArray() {
  std::array<float, 16> result;
  memcpy(&result[0], m, 16 * sizeof(float));
  return result;
}

Matrix4x4 Quatf::ToMatrix() {
  // Based on ion::math::RotationMatrix3x3
  const float xx = 2 * x * x;
  const float yy = 2 * y * y;
  const float zz = 2 * z * z;

  const float xy = 2 * x * y;
  const float xz = 2 * x * z;
  const float yz = 2 * y * z;

  const float xw = 2 * x * w;
  const float yw = 2 * y * w;
  const float zw = 2 * z * w;

  Matrix4x4 m;
  m.m[0][0] = 1 - yy - zz;
  m.m[0][1] = xy + zw;
  m.m[0][2] = xz - yw;
  m.m[0][3] = 0;
  m.m[1][0] = xy - zw;
  m.m[1][1] = 1 - xx - zz;
  m.m[1][2] = yz + xw;
  m.m[1][3] = 0;
  m.m[2][0] = xz + yw;
  m.m[2][1] = yz - xw;
  m.m[2][2] = 1 - xx - yy;
  m.m[2][3] = 0;
  m.m[3][0] = 0;
  m.m[3][1] = 0;
  m.m[3][2] = 0;
  m.m[3][3] = 1;

  return m;
}

Matrix4x4 GetMatrixFromGlArray(float* vec) {
  Matrix4x4 result;
  memcpy(result.m, vec, 16 * sizeof(float));
  return result;
}

Matrix4x4 GetTranslationMatrix(const std::array<float, 3>& translation) {
  return {{{1.0f, 0.0f, 0.0f, 0.0f},
           {0.0f, 1.0f, 0.0f, 0.0f},
           {0.0f, 0.0f, 1.0f, 0.0f},
           {translation.at(0), translation.at(1), translation.at(2), 1.0f}}};
}

float AngleBetweenVectors(const std::array<float, 4>& vec1,
                          const std::array<float, 4>& vec2) {
  return std::acos(
      std::max(-1.f, std::min(1.f, VectorDotProduct(vec1, vec2) /
                                       (VectorNorm(vec1) * VectorNorm(vec2)))));
}

static constexpr uint64_t kNanosInSeconds = 1000000000;

long GetMonotonicTimeNano() {
  struct timespec res;
  clock_gettime(CLOCK_MONOTONIC, &res);
  return (res.tv_sec * kNanosInSeconds) + res.tv_nsec;
}

float RandomUniformFloat(float min, float max) {
  static std::random_device random_device;
  static std::mt19937 random_generator(random_device());
  static std::uniform_real_distribution<float> random_distribution(0, 1);
  return random_distribution(random_generator) * (max - min) + min;
}

int RandomUniformInt(int max_val) {
  static std::random_device random_device;
  static std::mt19937 random_generator(random_device());
  std::uniform_int_distribution<int> random_distribution(0, max_val - 1);
  return random_distribution(random_generator);
}

void CheckGlError(const char* file, int line, const char* label) {
  int gl_error = glGetError();
  if (gl_error != GL_NO_ERROR) {
    LOGE("%s : %d > GL error @ %s: %d", file, line, label, gl_error);
    // Crash immediately to make OpenGL errors obvious.
    abort();
  }
}

GLuint LoadGLShader(GLenum type, const char* shader_source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &shader_source, nullptr);
  glCompileShader(shader);

  // Get the compilation status.
  GLint compile_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

  // If the compilation failed, delete the shader and show an error.
  if (compile_status == 0) {
    GLint info_len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len == 0) {
      return 0;
    }

    std::vector<char> info_string(info_len);
    glGetShaderInfoLog(shader, info_string.size(), nullptr, info_string.data());
    LOGE("Could not compile shader of type %d: %s", type, info_string.data());
    glDeleteShader(shader);
    return 0;
  } else {
    return shader;
  }
}

Texture::Texture() : texture_id_(0) {}

Texture::~Texture() {
  if (texture_id_ != 0) {
    glDeleteTextures(1, &texture_id_);
  }
}

bool Texture::Initialize(JNIEnv* env, jobject java_asset_mgr,
                         const std::string& texture_path) {
  glGenTextures(1, &texture_id_);
  Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  if (!LoadPngFromAssetManager(env, java_asset_mgr, GL_TEXTURE_2D,
                               texture_path)) {
    LOGE("Couldn't load texture.");
    return false;
  }
  glGenerateMipmap(GL_TEXTURE_2D);
  return true;
}

void Texture::Bind() const {
  HELLOCARDBOARD_CHECK(texture_id_ != 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
}

cv::Mat getCcvImgFromGlImg(GLuint ogl_texture_id, GLint texWidth, GLint texHeight)
{
//  glBindTexture(GL_TEXTURE_2D, ogl_texture_id);
//  GLenum gl_texture_width, gl_texture_height;
//
//  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)&gl_texture_width);
//  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&gl_texture_height);
//
//  unsigned char* gl_texture_bytes = (unsigned char*) malloc(sizeof(unsigned char)*gl_texture_width*gl_texture_height*3);
//  glGetTexImage(GL_TEXTURE_2D, 0 /* mipmap level */, GL_RGB, GL_UNSIGNED_BYTE, gl_texture_bytes);
//  //glGetTexImage(GL_TEXTURE_2D, 0 /* mipmap level */, GL_BGR, GL_UNSIGNED_BYTE, gl_texture_bytes);
//
//  return cv::Mat(gl_texture_height, gl_texture_width, CV_8UC3, gl_texture_bytes);
    //int data_size = texWidth * texHeight * 4;
    static int data_size = texWidth * texHeight * 4;
    GLubyte* pixels = new GLubyte[data_size];

//    GLuint fbo;
//    glGenFramebuffers(1, &fbo);
//    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ogl_texture_id, 0);

    //glReadPixels(0, 0, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    glDeleteFramebuffers(1, &fbo);

    cv::Mat result(texWidth, texHeight, CV_8UC4, pixels);

    return result;
}

}  // namespace ndk_hello_cardboard
