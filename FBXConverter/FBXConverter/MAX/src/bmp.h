#pragma once
#include <string>

void generateBitmapImage(unsigned char* image, int height, int width, const char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

class BMP {
 public:
    BMP(int width, int height) {
        width_ = width;
        height_ = height;
        buffer_BGR.resize(width * height * 3);
    }

    char& operator()(int x, int y) { return buffer_BGR[(y * width_ + x) * 3]; }

    const char& operator()(int x, int y) const { return buffer_BGR[(y * width_ + x) * 3]; }

    char& R(int x, int y) { return (&(operator()(x, y)))[2]; }
    char& G(int x, int y) { return (&(operator()(x, y)))[1]; }
    char& B(int x, int y) { return (&(operator()(x, y)))[0]; }
    const char& R(int x, int y) const { return (&(operator()(x, y)))[2]; }
    const char& G(int x, int y) const { return (&(operator()(x, y)))[1]; }
    const char& B(int x, int y) const { return (&(operator()(x, y)))[0]; }

    void save(const std::string& path) {
        generateBitmapImage((unsigned char*)buffer_BGR.data(), height_, width_, path.c_str());
    }

    void FromRGBA(int width, int height, const char* mem) {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                R(j, i) = mem[(width * i + j) * 4 + 0];
                G(j, i) = mem[(width * i + j) * 4 + 1];
                B(j, i) = mem[(width * i + j) * 4 + 2];
            }
        }
    }

    void FromBGRA(int width, int height, const char* mem) {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                R(j, i) = mem[(width * i + j) * 4 + 2];
                G(j, i) = mem[(width * i + j) * 4 + 1];
                B(j, i) = mem[(width * i + j) * 4 + 0];
            }
        }
    }

    void FromRGB(int width, int height, const char* mem) {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                R(j, i) = mem[(width * i + j) * 3 + 0];
                G(j, i) = mem[(width * i + j) * 3 + 1];
                B(j, i) = mem[(width * i + j) * 3 + 2];
            }
        }
    }

 private:
    int width_;
    int height_;
    std::string buffer_BGR;
};