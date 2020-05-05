#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec2 attrPosition;
layout(location = 1) in vec2 attrTexCoord;
layout(location = 2) in uint attrColorPacked;

layout (set = 0, binding = 0, std140) uniform Constants {
    mat4 viewProjectionMatrix;
} uConstants;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) flat out uint outColorPacked;


void main() {
    gl_Position = uConstants.viewProjectionMatrix * vec4(attrPosition, 0.0, 1.0);
    outTexCoord = attrTexCoord;
    outColorPacked = attrColorPacked;
}