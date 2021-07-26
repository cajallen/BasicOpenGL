#version 330 core

in VS_OUT {
    vec3 pos;
    vec3 texcoord;
    vec3 tangent_light_dir;
    vec3 tangent_view_pos;
    vec3 tangent_pos;
    vec4 shadowcoord;
} fs_in;

layout(location = 0)out vec4 out_color;

// Updated in shader callback funcs
uniform sampler2DArray tex;
uniform sampler2DShadow depthmap_tex;
uniform vec3 ambient_tint;
uniform vec3 diffuse_tint;
uniform vec3 specular_tint;
uniform float phong;

uniform float base_tex_offset;
uniform float specular_tex_offset;
uniform float normal_tex_offset;
uniform float height_tex_offset;

// the reason why ambient lighting is baked here is because it is generally scene static, not per-material
const float ambient_comp = 0.4;

vec4 calc_lighting_cont(vec3 base_in, vec3 spec_in, vec3 norm) {
    vec3 ambient = ambient_comp * ambient_tint * diffuse_tint * base_in;
    
    vec3 light_dir = -normalize(fs_in.tangent_light_dir);
    float diff_comp = max(dot(light_dir, norm), 0.0);
    vec3 diffuse = diffuse_tint * diff_comp * base_in;
    
    vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_pos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec_comp = pow(max(dot(norm, halfway_dir), 0.0), phong);
    vec3 specular = specular_tint * spec_in * spec_comp;

    float bias = clamp(0.001 * tan(acos(diff_comp)), 0, 1.0);
    float visibility = diff_comp > 0 ? texture(depthmap_tex, vec3(fs_in.shadowcoord.xy, (fs_in.shadowcoord.z - bias) / fs_in.shadowcoord.w)) : 1.0;
    vec4 reg = vec4(ambient, 1.0) + visibility * vec4(diffuse + specular, 1.0);

    vec4 debug = vec4(vec3(phong), 1.0);
    return reg;
}

void main() {
    vec4 base_in = base_tex_offset != -1 ? texture(tex, fs_in.texcoord + vec3(0,0,base_tex_offset)) : vec4(1,1,1,1);
    vec3 spec_in = specular_tex_offset != -1 ? texture(tex, fs_in.texcoord + vec3(0,0,specular_tex_offset)).rgb : vec3(1,1,1);
    vec3 normal = normal_tex_offset != -1 ? texture(tex, fs_in.texcoord + vec3(0,0,normal_tex_offset)).rgb : vec3(.5,.5,1);
    normal = normalize(normal * 2.0 - 1.0); // transform normal vector to range [-1,1]
    
    if (base_in.a < 0.5) discard;

    out_color = calc_lighting_cont(base_in.rgb, spec_in, normal);
}