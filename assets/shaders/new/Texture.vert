#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout (set = 0, binding = 0, std140) uniform FrameConstants {
    vec2 timeAndDelta;
} frameConstants;

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
} sceneConstants;

layout (set = 2, binding = 0, std140) uniform ObjectConstants {
    mat4 modelMatrix;
    vec3 tint;
} objectConstants;

layout(location = 0) out vec2 texCoord;

void main() {
    gl_Position = sceneConstants.projectionMatrix * sceneConstants.viewMatrix * objectConstants.modelMatrix * vec4(inPosition, 1.0);
    texCoord = inTexCoord;
}