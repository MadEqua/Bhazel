#version 450 core
#pragma shader_stage(fragment)

//layout(push_constant) uniform PushConstants {
//    vec4 tintAndAlpha;
//} pushConstants;

layout(set = 3, binding = 0) uniform sampler2D uTexSampler;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) flat in uint inColorPacked;

layout(location = 0) out vec4 outColor;

vec4 unpackColorInt(uint color) {
    vec4 vec;
    vec.a = ((color >> 24) & 255) / 255.0;
    vec.r = ((color >> 16) & 255) / 255.0;
    vec.g = ((color >> 8) & 255) / 255.0;
    vec.b = (color & 255) / 255.0;
    return vec;
}

void main() {
    //outColor = texture(uTexSampler, inTexCoord) * pushConstants.tintAndAlpha;
    outColor = texture(uTexSampler, inTexCoord) * unpackColorInt(inColorPacked);
}