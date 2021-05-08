#version 330 core

const float ambient_comp = .3;

in VS_OUT {
    vec3 pos;
    vec3 texcoord;
    vec3 tangent_light_pos;
    vec3 tangent_view_pos;
    vec3 tangent_pos;
} fs_in;

layout(location = 0) out vec4 outColor;


uniform sampler2DArray tex;

uniform vec3 ambient_col;
uniform vec3 diffuse_col;
uniform vec3 specular_col;
uniform float phong;
uniform int do_special_specular;

vec4 calc_lighting_cont(vec3 amb_in, vec3 diff_in, vec3 spec_in, vec3 norm) {
	float light_dist = length(fs_in.tangent_light_pos - fs_in.tangent_pos);
	vec3 ambient = ambient_comp * amb_in;

    vec3 light_dir = normalize(fs_in.tangent_light_pos - fs_in.tangent_pos);
    float diff_comp = max(dot(light_dir, norm), 0.0);
    vec3 diffuse = diff_comp * diff_in / light_dist;

    vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_pos);
    vec3 halfway_dir = normalize(light_dir + view_dir);  
    float spec_comp = pow(max(dot(norm, halfway_dir), 0.0), phong);
    vec3 specular = spec_in * abs(cos(do_special_specular * spec_comp)) * spec_comp / light_dist;

    return vec4(ambient + diffuse + specular, 1.0);
}

void main() {
    // transform normal vector to range [-1,1]
	vec3 normal = texture(tex, fs_in.texcoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
	
	outColor = calc_lighting_cont(ambient_col, diffuse_col, specular_col, normal);
}