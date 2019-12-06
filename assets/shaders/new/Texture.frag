#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(push_constant) uniform PushConstants {
//    vec4 tintAndAlpha;
//} pushConstants;

layout(set = 2, binding = 0) uniform sampler texSampler;
layout(set = 2, binding = 1) uniform texture2D tex;

layout(location = 0) in vec2 texCoord;
layout(location = 1) flat in uint colorPacked;
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
    //outColor = texture(sampler2D(tex, texSampler), texCoord) * pushConstants.tintAndAlpha;
    outColor = texture(sampler2D(tex, texSampler), texCoord) * unpackColorInt(colorPacked);
}