#pragma once

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"

#include "globals.h"

namespace caj {

struct DirectionalLight {
	vec3 direction = vec3(1,1,-1).normalized(); // manually normalize after setting
	glm::mat4 vp;

	vec3 depth_proj_min = vec3(-10.f, -10.f, -10.f);
	vec3 depth_proj_max = vec3(10.f, 10.f, 10.f);

	void set_direction(vec3 dir);
};

}  // namespace caj
