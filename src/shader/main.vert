#version 330 core

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inTexcoord;
layout(location=3) in vec3 inTangent;

out VS_OUT {
    vec3 pos;
    vec3 texcoord;
    vec3 tangent_light_dir;
    vec3 tangent_view_pos;
    vec3 tangent_pos;
	vec4 shadowcoord;
} vs_out;

uniform mat4 model; // once per frame per object
uniform mat4 VP; // once per frame
uniform mat4 DepthBiasVP; // once per light direction change

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
    vs_out.tangent_view_pos  = TBN * view_pos;
    vs_out.tangent_pos  = TBN * vs_out.pos;
        
	vs_out.shadowcoord = DepthBiasVP * model * vec4(inPosition, 1.0);

    gl_Position = VP * model * vec4(inPosition, 1.0);
}