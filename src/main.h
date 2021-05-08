#pragma once

#include <SDL.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <cpp/imgui_stdlib.h>
#include <glad/glad.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <stb_image.h>
#include <sstream>
#include <fstream>
#include <fmt/format.h>

#include "globals.h"
#include "log.h"
#include "level.h"
#include "model_loader.h"
#include "player.h"
#include "key.h"
#include "obj_loader.h"

using namespace std;
using namespace fmt;

GLuint LoadVertexSource(string s);
GLuint LoadFragmentSource(string s);
GLuint JoinShaders(GLuint vert, GLuint frag);
void UpdateShaderCameraUniforms(GLuint shader_program);