#ifndef VEC3_H
#define VEC3_H

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#define HPI 1.57079633

#include <math.h>
#include <ostream>
#include <string>
#include <stdexcept>
#include <iomanip>
#include "obj_loader.h"

using std::string;
using std::to_string;
using std::runtime_error;

inline float fclamp(float a, float min, float max) {
    return fmax(min, fmin(a, max));
}

struct vec3;

struct vec3i {
    int x, y, z;

    vec3i(int x, int y, int z) : x(x), y(y), z(z) {}
    vec3i() : x(0), y(0), z(0) {}
	
	#define BASE 16
	int hash() const {
		return x * (BASE*BASE) + y * BASE + z;
	}

	bool operator == (const vec3i& oth) const {
		return x == oth.x && y == oth.y && z == oth.z;
	}

    operator vec3() const;
};


inline vec3i imin(vec3i a, vec3i b) {
	return vec3i(fmin(a.x, b.x), fmin(a.y, b.y), fmin(a.z, b.z));
}

inline vec3i imax(vec3i a, vec3i b) {
	return vec3i(fmax(a.x, b.x), fmax(a.y, b.y), fmax(a.z, b.z));
}

struct vec3 {
    float x, y, z;

    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3() : x(0), y(0), z(0) {}
	vec3(int bin); // an odd constructor, meant to take binary inputs for directions
	vec3(glm::vec3 v3) : x(v3.x), y(v3.y), z(v3.z) {}
	vec3(objl::Vector3 v3) : x(v3.X), y(v3.Y), z(v3.Z) {}

    vec3 clamp(float min, float max) const { return vec3(fclamp(x, min, max), fclamp(y, min, max), fclamp(z, min, max)); }

    float mag() const { return sqrt(x * x + y * y + z * z); }
    float mag2() const { return x * x + y * y + z * z; }

    // Create a unit-length vector
    vec3 normalized() const {
        float len = mag();
        return vec3(x / len, y / len, z / len);
    }

    vec3& operator+=(vec3& a) {
        x += a.x;
        y += a.y;
		z += a.z;
        return *this;
    }

    vec3& operator-=(vec3& a) {
        x -= a.x;
        y -= a.y;
		z -= a.z;
        return *this;
    }

	vec3 operator*(vec3& rhs) {
		return vec3(x*rhs.x, y*rhs.y, z*rhs.z);
	}

	bool operator==(vec3& rhs) {
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}

	vec3 operator-() const {
		return vec3(-x, -y, -z);
	}


	string keyed_string(string prefix) {
		return prefix + to_string(x) + " " + to_string(y) + " " + to_string(z);
	}

	float pitch() {
		return asin(z);
	}
	float yaw() {
		return atan2(y, x);
	}

	float& operator[](int index) {
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;
		throw runtime_error("vec3[] out of bounds");
		return x;
	}

	operator glm::vec3() const {
		return glm::vec3(x, y, z);
	}

	static vec3 forward() { return vec3(0,1,0); }
	static vec3 back() { return vec3(0,-1,0); }
	static vec3 right() { return vec3(1,0,0); }
	static vec3 left() { return vec3(-1,0,0); }
	static vec3 up() { return vec3(0,0,1); }
	static vec3 down() { return vec3(0,0,-1); }
};

inline vec3 operator*(vec3 a, float f) {
    return vec3(a.x * f, a.y * f, a.z * f);
}

inline vec3 operator*(float f, vec3 a) {
    return vec3(a.x * f, a.y * f, a.z * f);
}

// Vector-vector dot product
inline float dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Vector-vector cross product
inline vec3 cross(vec3 a, vec3 b) {
    return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// Vector addition
inline vec3 operator+(vec3 a, vec3 b) {
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

// Vector subtraction
inline vec3 operator-(vec3 a, vec3 b) {
    return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline std::ostream& operator<<(std::ostream& os, vec3 v3) {
    return os << std::fixed << std::setprecision(2) << "{" << v3.z << ", " << v3.y << "}";
}

inline bool same_side(vec3 p1, vec3 p2, vec3 a, vec3 b) {
    vec3 cp1 = cross(b - a, p1 - a);
    vec3 cp2 = cross(b - a, p2 - a);
    return dot(cp1, cp2) >= 0;
}

inline vec3i::operator vec3() const {
    return vec3(x, y, z);
}

inline vec3 YawPitch(float yaw, float pitch) {
	pitch = fclamp(pitch, -HPI + 0.01, HPI - 0.01);
	float x = cos(pitch) * cos(yaw);
	float y = cos(pitch) * sin(yaw);
	float z = sin(pitch);
	return vec3(x,y,z);
}

inline vec3::vec3(int bin) {
	z = bin & 0b1 ? 0.5 : -0.5;
	y = bin & 0b10 ? 0.5 : -0.5;
	x = bin & 0b100 ? 0.5 : -0.5;
}

#endif