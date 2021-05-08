#version 430 core

layout (location = 0) uniform sampler2D tex;
in vec2 texcoord;
out vec3 outColor;

void main() {
	outColor = texture(tex, texcoord).rgb;
}