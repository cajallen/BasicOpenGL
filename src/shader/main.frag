#version 330 core

const float ambient_comp = .3;

in VS_OUT {
    vec3 pos;
    vec3 texcoord;
    vec3 tangent_light_dir;
    vec3 tangent_view_pos;
    vec3 tangent_pos;
	vec4 shadowcoord;
} fs_in;

layout(location = 0) out vec4 outColor;

uniform sampler2DArray tex; // once
uniform sampler2DShadow shadow_tex; // once
uniform vec3 ambient_col; // once per frame per object
uniform vec3 diffuse_col; // once per frame per object
uniform vec3 specular_col; // once per frame per object
uniform float phong; // once per frame per object

vec2 poissonDisk[4] = vec2[](
  vec2(-1,-.5),
  vec2( 1, .5),
  vec2(-.5, 1),
  vec2( .5,-1)
);

vec4 calc_lighting_cont(vec3 diff_in, vec3 spec_in, vec3 norm) {
	vec3 ambient = ambient_col * ambient_comp * diff_in;

    vec3 light_dir = -normalize(fs_in.tangent_light_dir);
    float diff_comp = max(dot(light_dir, norm), 0.0);
    vec3 diffuse = diffuse_col * diff_comp * diff_in;

    vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_pos);
    vec3 halfway_dir = normalize(light_dir + view_dir);  
    float spec_comp = pow(max(dot(norm, halfway_dir), 0.0), phong);
    vec3 specular = specular_col * spec_in * spec_comp;

	float bias = 0.0007*tan(acos(diff_comp));
	bias = clamp(bias, 0, 0.01);
	float visibility = 1.0;
	if (diff_comp > 0) {
		for (int i =0; i < 4; i++) {
			visibility -= 0.25*(1.0-texture(shadow_tex, vec3(fs_in.shadowcoord.xy + poissonDisk[i]/4500.0,  (fs_in.shadowcoord.z-bias)/fs_in.shadowcoord.w)));
		}
	}
    return vec4(ambient, 1.0) + visibility * vec4(diffuse + specular, 1.0);
}

void main() {
	// transform layer to 0-1 range
	vec3 texcoord = fs_in.texcoord;
	float layer_offset = 1 / float(textureSize(tex, 0).z);
    vec3 diff_in = texture(tex, texcoord).rgb;
	texcoord.z += 1.0;
	vec3 spec_in = texture(tex, texcoord).rgb;
	texcoord.z += 1.0;
	vec3 normal = texture(tex, texcoord).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
	
	outColor = calc_lighting_cont(diff_in, spec_in, normal);
}