#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files
#include "image_lib.h"

Color Color::operator+(const Color& rhs) const {
    return Color(r + rhs.r, g + rhs.g, b + rhs.b);
}
Color Color::operator*(const Color& rhs) const {
    return Color(r * rhs.r, g * rhs.g, b * rhs.b);
}
Color Color::operator*(const float& rhs) const {
    return Color(r * rhs, g * rhs, b * rhs);
}

bool Color::operator==(const Color& rhs) const {
    return r == rhs.r && g == rhs.g && b == rhs.b;
}

bool Color::operator<(const Color& rhs) const {
    return r < rhs.r && g < rhs.g && b < rhs.b;
}

Color Color::Lerp(const Color& rhs, float amt) const {
    float nr = LERP(r, rhs.r, amt);
    float ng = LERP(g, rhs.g, amt);
    float nb = LERP(b, rhs.b, amt);
    return Color(nr, ng, nb);
}

void Color::Clamp() {
    r = fmin(r, 1);
    g = fmin(g, 1);
    b = fmin(b, 1);
}

Image::Image(int w, int h) : width(w), height(h) {
    pixels = new Color[width * height];
}

// Copy constructor - Called on: Image img1 = img2; //Making new image
Image::Image(const Image& cpy) {
    width = cpy.width;
    height = cpy.height;
    memcpy(pixels, cpy.pixels, width * height * sizeof(Color));
}

// Assignment operator - Called on:  img1 = img2; //Overwriting existing data
Image& Image::operator=(const Image& rhs) {
    width = rhs.width;
    height = rhs.height;
    memcpy(pixels, rhs.pixels, width * height * sizeof(Color));
    return *this;
}

Image::Image(const char* fname) {
    int numComponents;  //(e.g., Y, YA, RGB, or RGBA)
    unsigned char* data = stbi_load(fname, &width, &height, &numComponents, 4);

    if (data == NULL) {
        printf("Error loading image: '%s'\n", fname);
        exit(-1);
    }

    pixels = new Color[width * height];

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            pixels[(i + j * width)] =
                Color(data[4 * (i + j * width) + 0] / 255.0,
                      data[4 * (i + j * width) + 1] / 255.0,
                      data[4 * (i + j * width) + 2] / 255.0);
        }
    }

    stbi_image_free(data);
}

void Image::setPixel(int x, int y, Color c) {
    pixels[x + y * width] = c;
}

Color& Image::getPixel(int x, int y) {
    return pixels[x + y * width];
}

uint8_t* Image::toBytes() {
    uint8_t* rawPixels = new uint8_t[width * height * 4];
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            Color col = getPixel(i, j);
            rawPixels[4 * (i + j * width) + 0] = uint8_t(fmin(col.r, 1) * 255);
            rawPixels[4 * (i + j * width) + 1] = uint8_t(fmin(col.g, 1) * 255);
            rawPixels[4 * (i + j * width) + 2] = uint8_t(fmin(col.b, 1) * 255);
            rawPixels[4 * (i + j * width) + 3] = 255;  // alpha
        }
    }
    return rawPixels;
}

void Image::write(const char* fname) {
    uint8_t* rawBytes = toBytes();

    int lastc = strlen(fname);

    switch (fname[lastc - 1]) {
        case 'g':  // jpeg (or jpg) or png
            if (fname[lastc - 2] == 'p' ||
                fname[lastc - 2] == 'e')  // jpeg or jpg
                stbi_write_jpg(fname, width, height, 4, rawBytes,
                               95);  // 95% jpeg quality
            else                     // png
                stbi_write_png(fname, width, height, 4, rawBytes, width * 4);
            break;
        case 'a':  // tga (targa)
            stbi_write_tga(fname, width, height, 4, rawBytes);
            break;
        case 'p':  // bmp
        default:
            stbi_write_bmp(fname, width, height, 4, rawBytes);
    }

    delete[] rawBytes;
}

Image::~Image() {
    delete[] pixels;
}

ostream& operator<<(ostream& os, const Color& col) {
    return os << "{" << col.r << "r, " << col.g << "g, " << col.b << "b}";
}
