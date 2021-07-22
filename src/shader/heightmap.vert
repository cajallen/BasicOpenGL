#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;

out vec2 UV;
out vec4 ShadowCoord;

uniform sampler2D heightmap;
uniform mat4 MVP;
uniform mat4 DepthBiasMVP;


void main() {
    vec3 corrected_pos = vertexPosition_modelspace + texture(heightmap, UV);
	gl_Position =  MVP * vec4(corrected_pos,1);
	
	ShadowCoord = DepthBiasMVP * vec4(vertexPosition_modelspace,1);

	UV = vertexUV;
}