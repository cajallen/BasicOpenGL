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

struct Entity {
	string model_name;
	vec3 position = vec3(0,0,0);
	vec3 amb_tint = vec3(1,1,1);
	vec3 diff_tint = vec3(1,1,1);
	vec3 spec_tint = vec3(1,1,1);
	float phong = 8;

	glm::mat4 model = glm::mat4(1);

	float lifetime = 0.0;

	Entity(vec3 pos) : position(pos) {}

	void Update(float delta);
	void UpdateUniforms(DrawSet ds);
	void Draw(DrawSet ds);
};


struct Key : Entity {
	KEY_COLOR color;
	float width;
	float rotation_speed;
	float rotation = 0.0;

	Key(vec3 pos, KEY_COLOR col, float rot_spd, float wid);
	void Update(float delta);
};

struct Door : Entity {
	KEY_COLOR color;

	float opened_at = 0.0;

	Door(vec3 pos, KEY_COLOR col);
	void Update(float delta);
	void Open();
	bool IsOpen();
};

struct Goal : Entity{
	float rotation_speed = 0.0;
	float rotation = 0.0;

	Goal(vec3 pos, float rot_spd);
	void Update(float delta);
};