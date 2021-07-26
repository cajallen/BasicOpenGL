#version 330 core

layout(location = 0)in vec3 vertexPosition_modelspace;
layout(location = 2)in vec3 inTexcoord;

out vec3 texcoord;

uniform mat4 model; // once per object per frame
uniform mat4 vp; // once per light direction change

void main() {
	texcoord = inTexcoord;
	gl_Position = vp * model * vec4(vertexPosition_modelspace, 1);
}