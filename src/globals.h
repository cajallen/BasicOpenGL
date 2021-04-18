#pragma once

#define PI 3.141592f
#define DEG2RAD (PI / 180)
#define RAD2DEG (180 / PI)

#include <SDL.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <glad/glad.h>
#include <map>

#include "obj_loader.h"
#include "vec3.h"

using namespace std;

struct Vertex {
	vec3 pos;
	vec3 norm;
	vec3 texcoord;
	vec3 tangent;
	vec3 bitangent;
};

struct ModelRange {
	int start_pos = 0;
	int size = 0;
};

struct DrawSet {
	GLuint fragment;
	GLuint vertex;
	GLuint shader;
	GLuint vao;
	GLuint vbo;
};

struct Level;

extern bool fullscreen;
extern float screen_width;
extern float screen_height;
extern SDL_Window* window;
extern ImGuiIO* io;
extern SDL_Cursor* cursor;
extern Level* level;
extern DrawSet ds_world;
extern DrawSet ds_models;
extern ModelRange model_world;
extern map<string, ModelRange> models;


inline void SetTriTangents(Vertex& v1, Vertex& v2, Vertex& v3) {
	// calculate tangent/bitangent vectors of both triangles
	glm::vec3 tangent1, bitangent1;
	glm::vec3 edge1 = v2.pos - v1.pos;
	glm::vec3 edge2 = v3.pos - v1.pos;
	glm::vec2 deltaUV1 = glm::vec2(v2.texcoord.x, v2.texcoord.y) - glm::vec2(v1.texcoord.x, v1.texcoord.y);
	glm::vec2 deltaUV2 = glm::vec2(v3.texcoord.x, v3.texcoord.y) - glm::vec2(v1.texcoord.x, v1.texcoord.y);

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	v1.tangent = tangent1;
	v2.tangent = tangent1;
	v3.tangent = tangent1;
	v1.bitangent = bitangent1;
	v2.bitangent = bitangent1;
	v3.bitangent = bitangent1;
}

enum KEY_COLOR {
	BRONZE,
	ADAMANT,
	COBALT,
	OBSIDIAN,
	TUNGSTEN,
	KEY_COLOR_COUNT
};

static inline char* KEY_COLOR_STR[] = {
	"BRONZE",
	"ADAMANT",
	"COBALT",
	"OBSIDIAN",
	"TUNGSTEN",
	"KEY_COLOR_COUNT"
};