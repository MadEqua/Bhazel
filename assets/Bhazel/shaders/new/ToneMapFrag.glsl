#version 450 core
#pragma shader_stage(fragment)

layout(set = 0, binding = 0) uniform sampler2D uInputTexSampler;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    vec3 hdrColor = texture(uInputTexSampler, inTexCoord).rgb;
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    //outColor = vec4(mapped, 1.0);
    outColor = vec4(hdrColor, 1.0);
}