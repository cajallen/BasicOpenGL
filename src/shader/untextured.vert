#version 150 core

in vec3 inPosition;
in vec3 inNormal;
in vec3 inTexcoord;
in vec3 inTangent;
in vec3 inBitangent;

out VS_OUT {
    vec3 pos;
    vec3 texcoord;
    vec3 tangent_light_pos;
    vec3 tangent_view_pos;
    vec3 tangent_pos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 view_pos;
uniform vec3 light_pos;

void main() {
    vs_out.pos = vec3(model * vec4(inPosition, 1.0));   
    vs_out.texcoord = inTexcoord;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * inTangent);
    vec3 N = normalize(normalMatrix * inNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    
    vs_out.tangent_light_pos = TBN * light_pos;
    vs_out.tangent_view_pos  = TBN * view_pos;
    vs_out.tangent_pos  = TBN * vs_out.pos;
        
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
}