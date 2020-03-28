#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    vec3 dirLightDirections[2];
    vec3 dirLightColors[2];
    int dirLightsCount;
} uSceneConstants;

layout(set = 3, binding = 0) uniform sampler2D uTexSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 col = vec3(0.0);
    for(int i = 0; i < uSceneConstants.dirLightsCount; ++i) {
        vec3 L = -normalize(uSceneConstants.dirLightDirections[i]);
        vec3 N = normalize(inNormal);
        vec3 V = normalize(uSceneConstants.cameraPosition - inPosition);
        vec3 H = normalize(L + V);

        vec3 amb = vec3(0.02);
        float diffuse = max(0.0, dot(L, N));
        float spec = pow(max(0.0, dot(H, N)), 100.0);

        col += amb + (diffuse * texture(uTexSampler, inTexCoord).rgb * uSceneConstants.dirLightColors[i]) + (spec * vec3(1.0));
    }

    outColor = vec4(col, 1.0);
    //outColor = vec4(N, 1.0);
    //outColor = vec4(inTexCoord, 0.0, 1.0);
}