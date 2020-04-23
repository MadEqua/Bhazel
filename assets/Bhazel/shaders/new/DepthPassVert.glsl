#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;

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