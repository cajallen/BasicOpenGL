#pragma once

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "globals.h"
#include "vec3.h"

using namespace std;

#define DIFF_ADAMANT	vec3(0.3, 0.6, 0.4)
#define DIFF_BRONZE		vec3(1.0, 0.7, 0.2)
#define DIFF_COBALT		vec3(0.2, 0.3, 0.9)
#define DIFF_OBSIDIAN	vec3(0.15, 0.12, 0.18)

enum LIGHTING_COMPONENT {
	AMBIENT,
	DIFFUSE,
	SPECULAR
};

inline vec3 GetKeyColor(KEY_COLOR col, LIGHTING_COMPONENT comp) {
	switch (col) {
	case (ADAMANT): {
		if (comp == DIFFUSE)	return 1.0 * DIFF_ADAMANT;
		if (comp == AMBIENT)	return 0.6 * DIFF_ADAMANT;
		if (comp == SPECULAR)	return 0.5 * DIFF_ADAMANT + 0.2 * vec3(1, 1, 1);
		break;
	}
	case (BRONZE): {
		if (comp == DIFFUSE)	return 1.0 * DIFF_BRONZE;
		if (comp == AMBIENT)	return 0.4 * DIFF_BRONZE;
		if (comp == SPECULAR)	return 0.5 * DIFF_BRONZE + 0.2 * vec3(1, 1, 1);
		break;
	}
	case (COBALT): {
		if (comp == DIFFUSE)	return 1.0 * DIFF_COBALT;
		if (comp == AMBIENT)	return 0.3 * DIFF_COBALT;
		if (comp == SPECULAR)	return 0.5 * DIFF_COBALT + 0.1 * vec3(1, 1, 1);
		break;
	}
	case (OBSIDIAN): {
		if (comp == DIFFUSE)	return 1.0 * DIFF_OBSIDIAN;
		if (comp == AMBIENT)	return 0.5 * DIFF_OBSIDIAN;
		if (comp == SPECULAR)	return 0.0 * DIFF_OBSIDIAN + 0.5 * vec3(1, 1, 1);
		break;
	}
	case (TUNGSTEN): {
		if (comp == DIFFUSE)	return 1.0 * vec3(0.6, 0.7, 0.6);
		if (comp == AMBIENT)	return 0.5 * vec3(1, 1, 1);
		if (comp == SPECULAR)	return 0.4 * vec3(1, 0, 1);
		break;
	}
	}
	return vec3(0, 0, 0);
}

inline float GetKeyPhong(KEY_COLOR col) {
	switch(col) {
	case (ADAMANT): return 4;
	case (BRONZE): return 4;
	case (COBALT): return 8;
	case (OBSIDIAN): return 32;
	case (TUNGSTEN): return 4;
	}
}

inline int DoKeySpecial(KEY_COLOR col) {
	switch (col) {
	case (ADAMANT): return 0;
	case (BRONZE): return 0;
	case (COBALT): return 5;
	case (OBSIDIAN): return 0;
	case (TUNGSTEN): return 12;
	}
}


struct Key {
	vec3 pos;
	KEY_COLOR col = BRONZE;
	float width;
	float rotation_speed;
	float rotation = 0.0;


	void Update(float delta);
	void Draw();
};

struct Door {
	vec3 pos;
	KEY_COLOR col = BRONZE;

	float seconds_alive = 0.0;
	float opened_at = 0.0;

	void Update(float delta);
	void Draw();
	void Open();

	bool IsOpen();
};

struct Goal {
	vec3 pos;
	float seconds_alive =  0.0;

	float rotation_speed = 0.5;

	void Update(float delta);
	void Draw();
};