#include "mat4c2gl.h"
#include <cstring>

mat4c2gl::mat4c2gl() { std::memset(data, 0, sizeof(data)); }

mat4c2gl::mat4c2gl(const float input) {
  std::memset(data, input, sizeof(data));
}

mat4c2gl::mat4c2gl(const float input[4][4]) {
  std::memcpy(data, input, sizeof(data));
}

mat4c2gl mat4c2gl::operator+(const mat4c2gl &other) const {
  mat4c2gl result;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      result.data[i][j] = data[i][j] + other.data[i][j];
  return result;
}

mat4c2gl mat4c2gl::operator*(const mat4c2gl &other) const {
  mat4c2gl result;
  std::memset(result.data, 0, sizeof(result.data));
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      for (int k = 0; k < 4; ++k)
        result.data[i][j] += data[i][k] * other.data[k][j];
  return result;
}

std::array<double, 4>
mat4c2gl::operator*(const std::array<double, 4> &vec) const {
  std::array<double, 4> result = {0, 0, 0, 0};
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      result[i] += data[i][j] * vec[j];
  return result;
}

void mat4c2gl::setViewMatrix(const std::array<double, 3> &eye,
                             const std::array<double, 3> &center,
                             const std::array<double, 3> &up) {
  std::array<double, 3> n, u, v;

  for (int i = 0; i < 3; ++i)
    n[i] = eye[i] - center[i];
  double norm = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
  for (int i = 0; i < 3; ++i)
    n[i] /= norm;

  u[0] = up[1] * n[2] - up[2] * n[1];
  u[1] = up[2] * n[0] - up[0] * n[2];
  u[2] = up[0] * n[1] - up[1] * n[0];
  norm = std::sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
  for (int i = 0; i < 3; ++i)
    u[i] /= norm;

  v[0] = n[1] * u[2] - n[2] * u[1];
  v[1] = n[2] * u[0] - n[0] * u[2];
  v[2] = n[0] * u[1] - n[1] * u[0];

  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      data[i][j] = 0.0f;

  data[0][0] = u[0];
  data[0][1] = u[1];
  data[0][2] = u[2];
  data[1][0] = v[0];
  data[1][1] = v[1];
  data[1][2] = v[2];
  data[2][0] = n[0];
  data[2][1] = n[1];
  data[2][2] = n[2];
  data[0][3] = -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]);
  data[1][3] = -(v[0] * eye[0] + v[1] * eye[1] + v[2] * eye[2]);
  data[2][3] = -(n[0] * eye[0] + n[1] * eye[1] + n[2] * eye[2]);
  data[3][3] = 1.0f;
}

void mat4c2gl::setProjectionMatrix(double fovY, double aspect, double near,
                                   double far) {
  float f = static_cast<float>(1.0 / std::tan(fovY * M_PI / 360.0));
  std::memset(data, 0, sizeof(data));
  data[0][0] = f / static_cast<float>(aspect);
  data[1][1] = f;
  data[2][2] = static_cast<float>((far + near) / (near - far));
  data[2][3] = static_cast<float>((2 * far * near) / (near - far));
  data[3][2] = -1.0f;
}

void mat4c2gl::setViewportMatrix(int x, int y, int width, int height) {
  float w = static_cast<float>(width);
  float h = static_cast<float>(height);
  std::memset(data, 0, sizeof(data));
  data[0][0] = w / 2.0f;
  data[0][3] = x + w / 2.0f;
  data[1][1] = h / 2.0f;
  data[1][3] = y + h / 2.0f;
  data[2][2] = 0.5f;
  data[2][3] = 0.5f;
  data[3][3] = 1.0f;
}

void mat4c2gl::print() const {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j)
      std::cout << data[i][j] << ' ';
    std::cout << '\n';
  }
}

std::array<GLfloat, 16> mat4c2gl::toGLMatrix() const {
  std::array<GLfloat, 16> result;
  for (int row = 0; row < 4; ++row)
    for (int col = 0; col < 4; ++col)
      result[col * 4 + row] = data[row][col]; // column-major
  return result;
}

float *mat4c2gl::operator[](int index) { return data[index]; }

const float *mat4c2gl::operator[](int index) const { return data[index]; }

glm::vec4 mat4c2gl::operator*(const glm::vec4 &v) const {
  glm::vec4 result(0.0f);
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      result[i] += data[i][j] * v[j];
  return result;
}

void mat4c2gl::setProjectionMatrixHV(double fovY_deg, double fovX_deg,
                                     double near, double far) {
  float fovY_rad = glm::radians(fovY_deg);
  float fovX_rad = glm::radians(fovX_deg);
  float f_y = static_cast<float>(1.0 / std::tan(fovY_rad / 2.0));
  float f_x = static_cast<float>(1.0 / std::tan(fovX_rad / 2.0));

  std::memset(data, 0, sizeof(data));
  data[0][0] = f_x;
  data[1][1] = f_y;
  data[2][2] = static_cast<float>((far + near) / (near - far));
  data[2][3] = static_cast<float>((2 * far * near) / (near - far));
  data[3][2] = -1.0f;
}