#version 450 core

layout(location = 0) in vec3 attrPosition;
layout(location = 1) in vec3 attrNormal;
layout(location = 2) in vec3 attrTangent;
layout(location = 3) in vec2 attrTexCoord;

layout (set = 0, binding = 0, std140) uniform FrameConstants {
    vec2 timeAndDelta;
} uFrameConstants;

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
} uSceneConstants;

layout(location = 0) out vec2 outTexCoord;

void main() {
    gl_Position = uSceneConstants.viewProjectionMatrix * vec4(attrPosition, 1.0);
    outTexCoord = attrTexCoord;
}