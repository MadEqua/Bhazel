#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec2 attrPos;
layout(location = 1) in vec2 attrTexCoord;
layout(location = 2) in vec4 attrColor;

layout (set = 0, binding = 0, std140) uniform Constants {
    mat4 projectionMatrix;
} uConstants;

layout(location = 0) out struct { 
    vec4 color;
    vec2 texCoord; 
} outData;

void main() {
    outData.color = attrColor;
    outData.texCoord = attrTexCoord;
    gl_Position = uConstants.projectionMatrix * vec4(attrPos, 0.0, 1.0);
}