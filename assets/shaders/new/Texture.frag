#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec3 tint;
} pushConstants;

layout(set = 2, binding = 0) uniform sampler texSampler;
layout(set = 2, binding = 1) uniform texture2D tex;

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

void main() {

    /*vec4 col = fragColor;
    for(int i = 0; i < 2048; ++i)  {
        col.g = sqrt(col.g);
    }*/

    outColor = texture(sampler2D(tex, texSampler), texCoord) * vec4(pushConstants.tint, 1.0);
    //outColor = vec4(tint, 1.0);
    //outColor = vec4(col, 1.0);
}