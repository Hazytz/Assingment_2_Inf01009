#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class camera {
public:
    camera();
    glm::vec3 getPosition() const;
    glm::vec3 getU() const;
    glm::vec3 getV() const;
    glm::vec3 getN() const;
    float getHFOV() const;
    float getVFOV() const;
    float getNearClip() const;
    float getFarClip() const;
    void setPosition(const glm::vec3& pos);
    void setOrientation(const glm::vec3& u, const glm::vec3& v, const glm::vec3& n);
    void setFOV(float hfov, float vfov);
    void setClippingPlanes(float nearClip, float farClip);
    void print() const;

private:
    glm::vec3 position;
    glm::vec3 u, v, n; 
    float hfov, vfov;  
    float nearClip, farClip;
};

#endif 