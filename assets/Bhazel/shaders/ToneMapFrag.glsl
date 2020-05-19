#version 450 core
#pragma shader_stage(fragment)

layout(location = 0) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform sampler2D uInputTexSampler;

layout (set = 0, binding = 1, std140) uniform PostProcessConstants {
    vec4 cameraExposureAndBloomIntensity;
} uPostProcessConstants;

layout(location = 0) out vec4 outColor;


void main() {
    vec3 hdrColor = texture(uInputTexSampler, inTexCoord).rgb;

    vec3 mapped = vec3(1.0) - exp(-hdrColor * uPostProcessConstants.cameraExposureAndBloomIntensity.x);
    outColor = vec4(mapped, 1.0);
}