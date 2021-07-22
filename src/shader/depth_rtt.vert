#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 model; // once per object per frame
uniform mat4 vp; // once per light direction change

void main() {
	gl_Position = vp * model * vec4(vertexPosition_modelspace,1);
}