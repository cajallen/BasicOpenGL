#pragma once

#define PI 3.141592f
#define DEG2RAD (PI / 180)
#define RAD2DEG (180 / PI)

#include <SDL.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <glad/glad.h>
#include <map>
#include <fmt/format.h>

#include "obj_loader.h"
#include "vec3.h"

using namespace std;

struct Vertex {
	vec3 pos;
	vec3 norm;
	vec3 texcoord;
	vec3 tangent;

	// TODO: violates only pointers set members
	static inline void set_tangents(Vertex& v1, Vertex& v2, Vertex& v3) {
		glm::vec3 tangent;
		glm::vec3 edge1 = v2.pos - v1.pos;
		glm::vec3 edge2 = v3.pos - v1.pos;
		glm::vec2 delta_UV1 = glm::vec2(v2.texcoord.x, v2.texcoord.y) - glm::vec2(v1.texcoord.x, v1.texcoord.y);
		glm::vec2 delta_UV2 = glm::vec2(v3.texcoord.x, v3.texcoord.y) - glm::vec2(v1.texcoord.x, v1.texcoord.y);

		float f = 1.0f / (delta_UV1.x * delta_UV2.y - delta_UV2.x * delta_UV1.y);

		tangent.x = f * (delta_UV2.y * edge1.x - delta_UV1.y * edge2.x);
		tangent.y = f * (delta_UV2.y * edge1.y - delta_UV1.y * edge2.y);
		tangent.z = f * (delta_UV2.y * edge1.z - delta_UV1.y * edge2.z);

		v1.tangent = tangent;
		v2.tangent = tangent;
		v3.tangent = tangent;
	}
};

struct Material {
	string name;
	vec3 ambient_color;
	vec3 diffuse_color;
	vec3 specular_color;
	float phong;

	GLuint atlas_tex;
	int layer_offset; // z coord is layer_offset * layer_width

	// TODO: these are weird values... They will only be used temporarily...
	// Once we get all of the geometry, we load the atlas, and reference the
	// textures with the above...
	string base_color_tex_file;
	string specular_tex_file;
	string normal_tex_file;
	string height_tex_file;
};

struct ModelRange {
	int start_pos = 0;
	int size = 0;
};

struct Mesh { // This mesh does not yet support custom shaders.
	Material material;
	ModelRange model_range;
	glm::mat4 transform = glm::mat4(1);
};

struct Shader {
	GLuint program;
	GLuint vao;

	map<string, GLuint> uniforms;

	// oddly, this doesn't need to be a part of the struct, but helps uniformity without much cost
	void (*initialization)(Shader& shader);
	// per frame uniform update callback, array bound before and unbound after
	void (*pre_render)(Shader& shader);
	// per object per frame uniform update callback, array bound before and unbound after
	void (*pre_mesh_render)(Shader& shader, Mesh& mesh);
};
