#include "lights.h"

namespace caj {

void DirectionalLight::set_direction(vec3 dir) {
    direction = dir;
    glm::vec3 light_inv_dir = glm::vec3(-dir.x, -dir.y, -dir.z);
    glm::mat4 proj = glm::ortho(
        depth_proj_min.x, depth_proj_max.x, 
        depth_proj_min.y, depth_proj_max.y, 
        depth_proj_min.z, depth_proj_max.z
    );
    glm::mat4 view = glm::lookAt(light_inv_dir, glm::vec3(0,0,0), glm::vec3(0,0,1));
    vp = proj * view;
}

}  // namespace caj