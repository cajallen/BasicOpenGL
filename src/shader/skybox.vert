#version 330 core

layout(location = 0)in vec3 in_pos;

out vec3 texcoord;

uniform mat4 vp;

void main() {
    texcoord = in_pos;
    gl_Position = vp * vec4(in_pos, 1.0);
}