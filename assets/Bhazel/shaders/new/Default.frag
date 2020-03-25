#version 450 core

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;

layout(set = 3, binding = 0) uniform sampler uTexSampler;
layout(set = 3, binding = 1) uniform texture2D uTex;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = vec4(inTexCoord, 0.0, 1.0);
    //outColor = vec4(inNormal * 0.5 + 0.5, 1.0);
    outColor = texture(sampler2D(uTex, uTexSampler), inTexCoord);
}