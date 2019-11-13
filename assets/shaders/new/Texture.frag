#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 2, binding = 0, std140) uniform ObjectConstants {
    mat4 modelMatrix;
    vec3 tint;
} objectConstants;

layout(set = 3, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() {

    /*vec4 col = fragColor;
    for(int i = 0; i < 2048; ++i)  {
        col.g = sqrt(col.g);
    }*/

    outColor = texture(texSampler, texCoord) * vec4(objectConstants.tint, 1.0);
    //outColor = vec4(col, 1.0);
}