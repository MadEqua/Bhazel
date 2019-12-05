#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec4 tintAndAlpha;
} pushConstants;

layout(set = 2, binding = 0) uniform sampler texSampler;
layout(set = 2, binding = 1) uniform texture2D tex;

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(sampler2D(tex, texSampler), texCoord) * pushConstants.tintAndAlpha;
}