#version 330 core

in vec3 texcoord;

out vec4 out_color;

uniform samplerCube skybox;
uniform vec3 light_dir;

const float interior_angle = 0.1; // cos(ang)
const float exterior_angle = 0.15; // cos(ang)

void main() {
    float sun_comp = (exterior_angle - acos(dot(normalize(-light_dir), normalize(texcoord.xyz))))/(exterior_angle - interior_angle);
    sun_comp = max(sun_comp, 0.0);
    out_color = mix(texture(skybox, texcoord.xzy), vec4(1,1,1,1), sun_comp);
}