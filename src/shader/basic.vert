#version 430 core

in vec2 position;
in vec2 inTexcoord;
out vec2 texcoord;


void main() {
	gl_Position = vec4(position, 1.0, 1.0);
	texcoord = inTexcoord;
}