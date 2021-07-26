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

uniform sampler2DArray tex;
uniform sampler2DShadow depthmap_tex;

uniform float base_tex_offset;
uniform float base_normal_tex_offset;
uniform float warp1_tex_offset; // larger
uniform float warp2_tex_offset; // smaller
uniform vec3 specular_tint; // .4
uniform float phong; // 64?

uniform float intensity1; // .2
uniform float intensity2; // .06
uniform vec2 base_velocity;  //  .04,  .03
uniform vec2 warp1_velocity; // -.04, -.03
uniform vec2 warp2_velocity; //  .03, -.04
uniform vec3 scale; // .2, .2, .2

uniform vec3 col_min; // 14, 62, 110
uniform vec3 col_mid; // 32, 160, 230
uniform vec3 col_max; // 150, 228, 250

const float ambient_comp = 0.4;

vec4 calc_lighting_cont(vec3 base_in, vec3 norm) {
    vec3 ambient = ambient_comp * base_in;
    
    vec3 light_dir = -normalize(fs_in.tangent_light_dir);
    float diff_comp = max(dot(light_dir, norm), 0.0);
    vec3 diffuse = diff_comp * base_in;
    
    vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_pos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec_comp = pow(max(dot(norm, halfway_dir), 0.0), phong);
    vec3 specular = specular_tint * spec_comp;

    float bias = clamp(0.001 * tan(acos(diff_comp)), 0, 1.0);
    float visibility = diff_comp > 0 ? texture(depthmap_tex, vec3(fs_in.shadowcoord.xy, (fs_in.shadowcoord.z - bias) / fs_in.shadowcoord.w)) : 1.0;
    vec4 reg = vec4(ambient, 1.0) + visibility * vec4(diffuse + specular, 1.0);

    return reg;
}

void main() {
	vec2 tex_size = vec2(textureSize(tex, 0).xy);
	
	vec2 offset1 = vec2(0.5, 0.5) - texture(tex, vec3(uv.xy + TIME * warp1_velocity, warp1_tex_offset)).xy;
	offset1 *= intensity1;
	vec2 offset2 = vec2(0.5, 0.5) - texture(tex, vec3(uv.xy + TIME * warp2_velocity, warp2_tex_offset)).xy;
	offset2 *= intensity2;
	
	vec2 coord = uv.xz + offset1 + offset2 + TIME * base_velocity;
	float height = texture(tex, vec3(coord, base_tex_offset)).r;
    vec3 normal = texture(tex, vec3(coord, base_normal_offset));
	
	float min_comp = max(1-height*2, 0.0);
    float max_comp = max(height*2-1, 0.0);
    float mid_comp = 1-min_comp-max_comp;
    vec3 base_col = col_min * min_comp + col_mid * mid_comp + col_max * max_comp;
    
    out_color = calc_lighting_cont(base_col, normal);
}