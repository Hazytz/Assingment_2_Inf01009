#include "camera.h"
#include <glm/gtc/constants.hpp> 
#include <iostream>

camera::camera()
    : position(0.0f, 0.0f, 0.0f),
      u(1.0f, 0.0f, 0.0f),
      v(0.0f, 1.0f, 0.0f),
      n(0.0f, 0.0f, -1.0f),
      hfov(60.0f),
      vfov(60.0f),
      nearClip(0.1f),
      farClip(1000.0f) {}

glm::vec3 camera::getPosition() const { return position; }
glm::vec3 camera::getU() const { return u; }
glm::vec3 camera::getV() const { return v; }
glm::vec3 camera::getN() const { return n; }

float camera::getHFOV() const { return hfov; }
float camera::getVFOV() const { return vfov; }
float camera::getNearClip() const { return nearClip; }
float camera::getFarClip() const { return farClip; }

void camera::setPosition(const glm::vec3& pos) {
    position = pos;
}

void camera::setOrientation(const glm::vec3& uVec, const glm::vec3& vVec, const glm::vec3& nVec) {
    u = uVec;
    v = vVec;
    n = nVec;
}

void camera::setFOV(float h, float v_) {
    hfov = h;
    vfov = v_;
}

void camera::setClippingPlanes(float near_, float far_) {
    nearClip = near_;
    farClip = far_;

}

void camera::print() const {
    std::cout << "Camera Info:\n";
    std::cout << "  Position: (" << position.x << ", " << position.y << ", " << position.z << ")\n";
    std::cout << "  U vector: (" << u.x << ", " << u.y << ", " << u.z << ")\n";
    std::cout << "  V vector: (" << v.x << ", " << v.y << ", " << v.z << ")\n";
    std::cout << "  N vector: (" << n.x << ", " << n.y << ", " << n.z << ")\n";
    std::cout << "  Horizontal FOV: " << hfov << " degrees\n";
    std::cout << "  Vertical FOV: " << vfov << " degrees\n";
    std::cout << "  Near Clip: " << nearClip << "\n";
    std::cout << "  Far Clip: " << farClip << "\n";
}
