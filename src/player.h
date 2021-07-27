#pragma once

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <array>

#include "imgui_ex.h"
#include "vec3.h"
#include "globals.h"
#include "log.h"

#define FORWARD_INDEX 0
#define BACKWARD_INDEX 1
#define RIGHT_INDEX 2
#define LEFT_INDEX 3
#define UP_INDEX 4
#define DOWN_INDEX 5

using namespace std;

struct Player {
	// state
	vec3 logic_pos = vec3(1, 0, 1);
	vec3 camera_pos = vec3(1, 0, 1);
	vec3 des_pos = vec3(1, 0, 1);
	vec3 look_dir = vec3(-1, 0, -1); 

	// settings
	float noclip_speed = 4.0;
	float pan_speed = 0.075;
	float width = 0.2;
	float height = 0.4;

	// input state
	bool using_mouse = true;
	bool warped = false;
	int mouse_diff_x = 0, mouse_diff_y = 0;
	array<bool, 6> wasd_input = {false, false, false, false, false, false};

	SDL_Window* window;

	glm::mat4 get_view_matrix() const;

	void handle_input(SDL_Event* event);
	void update(float delta);
	void move(float delta);
};