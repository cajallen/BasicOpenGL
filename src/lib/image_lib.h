// CSCI 5607 Image Library

#ifndef IMAGE_LIB_H
#define IMAGE_LIB_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>

#define LERP(a, b, r) ((1.0 - r) * a + r * b)

using namespace std;

struct Color {
    float r, g, b;

    Color(float r, float g, float b) : r(r), g(g), b(b) {}
    Color() : r(0), g(0), b(0) {}

    Color Lerp(const Color& rhs, float amt) const;
    void Clamp();

    Color operator+(const Color& rhs) const;
    Color operator*(const Color& rhs) const;
    Color operator*(const float& rhs) const;
    bool operator==(const Color& rhs) const;
	bool operator<(const Color& rhs) const;
};

struct Image {
    int width, height;
    Color* pixels;

    Image(int w, int h);
    Image(const Image& cpy);
    Image(const char* fname);
    ~Image();

    void setPixel(int i, int j, Color c);
    Color& getPixel(int i, int j);
    uint8_t* toBytes();
    void write(const char* fname);

    Image& operator=(const Image& rhs);
};
   
ostream& operator<<(ostream& os, const Color& col);

#endif