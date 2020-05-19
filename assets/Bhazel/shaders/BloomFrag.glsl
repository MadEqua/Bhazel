#version 450 core
#pragma shader_stage(fragment)

layout(location = 0) in vec2 inTexCoord;

layout (set = 0, binding = 1, std140) uniform PostProcessConstants {
    vec4 cameraExposureAndBloomIntensity;
} uPostProcessConstants;

layout(set = 1, binding = 0) uniform sampler2D uInputTexSampler;

layout(location = 0) out vec4 outColor;


void main() {
    vec3 col = texture(uInputTexSampler, inTexCoord).rgb;
    outColor = vec4(col, uPostProcessConstants.cameraExposureAndBloomIntensity.y);
}