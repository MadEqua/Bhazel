#version 450 core
#pragma shader_stage(fragment)

layout(location = 0) in vec3 inCubeMapDirection;

layout(set = 3, binding = 1) uniform samplerCube uTexSampler;

layout(location = 0) out vec4 outColor;


void main() {
    vec3 col = texture(uTexSampler, normalize(inCubeMapDirection)).rgb;
    outColor = vec4(col, 1.0);
}