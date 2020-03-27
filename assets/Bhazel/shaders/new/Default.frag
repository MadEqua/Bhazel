#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
} uSceneConstants;

layout(set = 3, binding = 0) uniform sampler2D uTexSampler;

layout(location = 0) out vec4 outColor;

vec3 LIGHT_DIR = vec3(-0.5, -1.0, 0.0);

void main() {

    vec3 L = -normalize(LIGHT_DIR);
    vec3 N = normalize(inNormal);
    vec3 V = normalize(uSceneConstants.cameraPosition - inPosition);
    vec3 H = normalize(L + V);

    vec3 amb = vec3(0.02);
    float diffuse = max(0.0, dot(L, N));
    float spec = pow(max(0.0, dot(H, N)), 100.0);

    vec3 col = amb + (diffuse * texture(uTexSampler, inTexCoord).rgb) + (spec * vec3(1.0));

    outColor = vec4(col, 1.0);
    //outColor = vec4(N, 1.0);
    //outColor = vec4(inTexCoord, 0.0, 1.0);
}