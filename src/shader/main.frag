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

uniform sampler2DArray tex; // once
uniform sampler2DShadow depthmap_tex; // once
uniform vec3 ambient_col; // once per frame per object
uniform vec3 diffuse_col; // once per frame per object
uniform vec3 specular_col; // once per frame per object
uniform float phong; // once per frame per object

vec4 calc_lighting_cont(vec3 diff_in, vec3 spec_in, vec3 norm) {
    vec3 ambient = ambient_col * diff_in;
    
    vec3 light_dir = -normalize(fs_in.tangent_light_dir);
    float diff_comp = max(dot(light_dir, norm), 0.0);
    vec3 diffuse = diffuse_col * diff_comp * diff_in;
    
    vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_pos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec_comp = pow(max(dot(norm, halfway_dir), 0.0), phong);
    vec3 specular = specular_col * spec_in * spec_comp;

    float bias = clamp(0.001 * tan(acos(diff_comp)), 0, 0.1);
    float visibility = diff_comp > 0 ? texture(depthmap_tex, vec3(fs_in.shadowcoord.xy, (fs_in.shadowcoord.z - bias) / fs_in.shadowcoord.w)) : 1.0;
    //vec4 debug = vec4(vec3(visibility * 0.1 + 0.1), 1.0);
    vec4 debug = vec4(norm * 0.5 + vec3(0.5), 1.0);
    vec4 reg = vec4(ambient, 1.0) + visibility * vec4(diffuse + specular, 1.0);

    return debug*0. + reg*1.;
}

void main() {
    vec3 diff_in = texture(tex, fs_in.texcoord).rgb;
    vec3 spec_in = texture(tex, fs_in.texcoord + vec3(0,0,1)).rgb;
    vec3 normal = texture(tex, fs_in.texcoord + vec3(0,0,2)).rgb;
    normal = normalize(normal * 2.0 - 1.0); // transform normal vector to range [-1,1]
    
    out_color = calc_lighting_cont(diff_in, spec_in, normal);
}