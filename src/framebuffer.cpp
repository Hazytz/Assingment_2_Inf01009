#include "framebuffer.h"
#include <stdexcept>




framebuffer::framebuffer(int width, int height)
    : width(width), height(height),
      colorBuffer(width * height, {0, 0, 0, 255}),
      depthBuffer(width * height, 1.0f) {}

framebuffer::framebuffer() : framebuffer(0, 0) {} 

void framebuffer::resize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    colorBuffer.resize(width * height, {0, 0, 0, 255});
    depthBuffer.resize(width * height, 1.0f);
}

void framebuffer::clearColorBuffer(const std::array<uint8_t, 4>& clearColor) {
    std::fill(colorBuffer.begin(), colorBuffer.end(), clearColor);
}

void framebuffer::clearDepthBuffer(float clearDepth) {
    std::fill(depthBuffer.begin(), depthBuffer.end(), clearDepth);
}

std::array<uint8_t, 4>& framebuffer::getPixel(int x, int y) {
    return colorBuffer[getIndex(x, y)];
}

float& framebuffer::getDepth(int x, int y) {
    return depthBuffer[getIndex(x, y)];
}

void framebuffer::setPixel(int x, int y, const std::array<uint8_t, 4>& color) {
    colorBuffer[getIndex(x, y)] = color;
}

void framebuffer::setDepth(int x, int y, float depth) {
    depthBuffer[getIndex(x, y)] = depth;
}

int framebuffer::getWidth() const {
    return width;
}

int framebuffer::getHeight() const {
    return height;
}

int framebuffer::getIndex(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height)
        throw std::out_of_range("framebuffer: Pixel coordinates out of range");
    return y * width + x;
}

void framebuffer::writeColorBufferToTxt(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto& pixel = colorBuffer[getIndex(x, y)];
            outFile << "("
                    << std::setw(3) << static_cast<int>(pixel[0]) << ", "
                    << std::setw(3) << static_cast<int>(pixel[1]) << ", "
                    << std::setw(3) << static_cast<int>(pixel[2]) << ", "
                    << std::setw(3) << static_cast<int>(pixel[3]) << ")";
            if (x < width - 1)
                outFile << " ";
        }
        outFile << "\n";
    }
}

void framebuffer::writePPM(const std::string& filename) const {
    FILE* f = std::fopen(filename.c_str(), "wb");
    if (!f) {
        perror("fopen");
        return;
    }

    // P6 = binary RGB
    std::fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto& c = colorBuffer[getIndex(x, y)];
            std::fputc(c[0], f); // R
            std::fputc(c[1], f); // G
            std::fputc(c[2], f); // B
        }
    }

    std::fclose(f);
}