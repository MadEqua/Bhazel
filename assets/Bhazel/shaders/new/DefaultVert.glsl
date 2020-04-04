#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;
layout(location = 1) in vec3 attrNormal;
layout(location = 2) in vec3 attrTangent;
layout(location = 3) in vec3 attrBitangent;
layout(location = 4) in vec2 attrTexCoord;

layout (set = 0, binding = 0, std140) uniform FrameConstants {
    vec2 timeAndDelta;
} uFrameConstants;

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    vec3 dirLightDirections[2];
    vec3 dirLightColors[2];
    int dirLightsCount;
} uSceneConstants;

layout (set = 2, binding = 0, std140) uniform ObjectConstants {
    mat4 modelMatrix;
    mat3 normalMatrix;
} uObjectConstants;

layout(location = 0) out struct {
    vec3 position;
    vec2 texCoord;
    mat3 tbnMatrix; //Tangent space to world space
} outData;

void main() {
    vec4 positionWorld = uObjectConstants.modelMatrix * vec4(attrPosition, 1.0);
    gl_Position = uSceneConstants.viewProjectionMatrix * positionWorld;

    outData.position = positionWorld.xyz;
    outData.texCoord = attrTexCoord;
    outData.tbnMatrix = uObjectConstants.normalMatrix * mat3(attrTangent, attrBitangent, attrNormal);
}