#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <iomanip>
#include <fstream>



class framebuffer {
public:
    framebuffer();
    framebuffer(int width, int height);

    void resize(int newWidth, int newHeight);
    void clearColorBuffer(const std::array<uint8_t, 4>& clearColor);
    void clearDepthBuffer(float clearDepth);

    std::array<uint8_t, 4>& getPixel(int x, int y);
    float& getDepth(int x, int y);

    int getWidth() const;
    int getHeight() const;
    void setPixel(int x, int y, const std::array<uint8_t, 4>& color);
    void setDepth(int x, int y, float depth);
    void writeColorBufferToTxt(const std::string& filename) const;
    void writePPM(const std::string& filename) const;
    

    int width, height;
    std::vector<std::array<uint8_t, 4>> colorBuffer;
    std::vector<float> depthBuffer;

    int getIndex(int x, int y) const;
};

#endif