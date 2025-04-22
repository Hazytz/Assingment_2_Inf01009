#version 330 core

in vec3 mobileColor;

out vec4 gpuColor;

void main() {
    gpuColor = vec4(mobileColor, 1.0);
}