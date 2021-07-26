#version 330 core

layout(location = 0)in vec3 inPosition;
layout(location = 1)in vec3 inNormal;
layout(location = 2)in vec3 inTexcoord;
layout(location = 3)in vec3 inTangent;

out VS_OUT {
    vec3 pos;
    vec3 texcoord;
    vec3 tangent_light_dir;
    vec3 tangent_view_pos;
    vec3 tangent_pos;
    vec4 shadowcoord;
} vs_out;

uniform mat4 model; // once per frame per object
uniform mat4 vp; // once per frame
uniform mat4 db_vp; // once per light direction change
uniform sampler2DArray tex; // once

uniform float height_tex_offset;

uniform vec3 view_pos; // once per frame
uniform vec3 light_dir; // once per light direction change


void main() {
    vs_out.pos = vec3(model * vec4(inPosition, 1.0));
    vs_out.texcoord = inTexcoord;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * inTangent);
    vec3 N = normalize(normalMatrix * inNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.tangent_light_dir = TBN * light_dir;
    vs_out.tangent_view_pos = TBN * view_pos;
    vs_out.tangent_pos = TBN * vs_out.pos;
    
    // ternary is to avoid read
    float norm_offset = height_tex_offset != -1.0 ? 2.0 * texture(tex, inTexcoord + vec3(0,0,height_tex_offset)).r: 0.0;
    vec3 corrected_pos = inPosition + inNormal * norm_offset;
    
    vs_out.shadowcoord = db_vp * model * vec4(corrected_pos, 1.0);
    
    gl_Position = vp * model * vec4(corrected_pos, 1.0);
}