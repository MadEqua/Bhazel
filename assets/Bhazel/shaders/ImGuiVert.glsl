#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec2 attrPos;
layout(location = 1) in vec2 attrTexCoord;
layout(location = 2) in vec4 attrColor;

layout (push_constant) uniform Constants {
    vec2 displaySize;
} uConstants;

layout(location = 0) out struct { 
    vec4 color;
    vec2 texCoord; 
} outData;


void main() {
    outData.color = attrColor;
    outData.texCoord = attrTexCoord;

    vec2 pos;
    pos.x = (attrPos.x * 2.0 / uConstants.displaySize.x) - 1.0;
    pos.y = (attrPos.y * -2.0 / uConstants.displaySize.y) + 1.0;

    gl_Position = vec4(pos, 0.0, 1.0);
}