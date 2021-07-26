#version 330 core

in vec3 texcoord;

layout(location = 0)out float fragmentdepth;

uniform sampler2DArray tex;
uniform float base_tex_offset;

void main() {
    // Not really needed, OpenGL does it anyway
    vec4 base_in = base_tex_offset != -1 ? texture(tex, texcoord + vec3(0,0,base_tex_offset)) : vec4(1,1,1,1);
    if (base_in.a < 0.5) discard;

    fragmentdepth = gl_FragCoord.z;
}