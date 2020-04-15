#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;
layout(location = 1) in vec3 attrNormal;
layout(location = 2) in vec3 attrTangent;
layout(location = 3) in vec3 attrBitangent;
layout(location = 4) in vec2 attrTexCoord;

layout (set = 1, binding = 0, std140) uniform PassConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
} uPassConstants;

layout (set = 3, binding = 0, std140) uniform EntityConstants {
    mat4 modelMatrix;
    mat4 normalMatrix;
} uEntityConstants;

void main() {
    gl_Position = uPassConstants.viewProjectionMatrix * uEntityConstants.modelMatrix * vec4(attrPosition, 1.0);
}