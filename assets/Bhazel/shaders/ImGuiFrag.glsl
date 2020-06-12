#version 450 core
#pragma shader_stage(fragment)

layout(set = 0, binding = 0) uniform sampler2D uTextureSampler;

layout(location = 0) in struct { 
    vec4 color;
    vec2 texCoord; 
} inData;

layout(location = 0) out vec4 outColor;


void main() {
    outColor = inData.color * texture(uTextureSampler, inData.texCoord);
}