#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 model; // once per object per frame
uniform mat4 VP; // once per light direction change

void main() {
	gl_Position = VP * model * vec4(vertexPosition_modelspace,1);
}