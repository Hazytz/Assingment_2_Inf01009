#ifndef MAT4C2GL_H
#define MAT4C2GL_H

#include <vector>
#include <iostream>
#include <array>
#include <cmath>
#include <GL/glew.h>
#include "glm/glm.hpp"

class mat4c2gl {
    public:
        float data[4][4];
    
        mat4c2gl();  // default constructor
        mat4c2gl(const float input);
        mat4c2gl(const float input[4][4]);
    
        mat4c2gl operator+(const mat4c2gl& other) const;
        mat4c2gl operator*(const mat4c2gl& other) const;
        std::array<double, 4> operator*(const std::array<double, 4>& vec) const;
    
        void setViewMatrix(const std::array<double, 3>& eye,
                           const std::array<double, 3>& center,
                           const std::array<double, 3>& up);
    
        void setProjectionMatrix(double fov, double aspect, double near, double far);
        void setViewportMatrix(int x, int y, int w, int h);
    
        std::array<GLfloat, 16> toGLMatrix() const;
    
        void print() const;
        float* operator[](int index);
        const float* operator[](int index) const ;
        glm::vec4 operator*(const glm::vec4& v) const;
        void setProjectionMatrixHV(double fovY_deg, double fovX_deg,
            double near, double far);
    };
    

#endif