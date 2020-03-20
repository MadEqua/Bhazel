#version 450 core

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(inTexCoord, 0.0, 1.0);
}