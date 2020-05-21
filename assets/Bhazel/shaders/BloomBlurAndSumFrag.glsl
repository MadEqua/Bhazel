#version 450 core
#pragma shader_stage(fragment)

#define BLOOM_TEXTURE_MIPS 5

layout(location = 0) in vec2 inTexCoord;

layout (set = 0, binding = 1, std140) uniform PostProcessConstants {
    vec4 cameraExposureAndBloomIntensity;
    float bloomBlurWeights[BLOOM_TEXTURE_MIPS];
} uPostProcessConstants;

layout(set = 1, binding = 0) uniform sampler2D uInputTexSampler;
layout(set = 1, binding = 1) uniform sampler2D uPreviousMipTexSampler;

layout(push_constant) uniform Data {
    uint direction;
    uint currentMip;
} uData;

layout(location = 0) out vec4 outColor;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


void main() {
    vec2 texelOffset = 1.0 / textureSize(uInputTexSampler, 0);

    vec3 result = texture(uInputTexSampler, inTexCoord).rgb * weight[0];

    //Horizontal
    if(uData.direction == 0) {
        for(int i = 1; i < 5; ++i) {
            result += texture(uInputTexSampler, inTexCoord + vec2(texelOffset.x * i, 0.0)).rgb * weight[i];
            result += texture(uInputTexSampler, inTexCoord - vec2(texelOffset.x * i, 0.0)).rgb * weight[i];
        }
    }
    //The vertical pass will simultaneously do a sum of the current mip with the previous one.
    else {
        for(int i = 1; i < 5; ++i) {
            result += texture(uInputTexSampler, inTexCoord + vec2(0.0, texelOffset.y * i)).rgb * weight[i];
            result += texture(uInputTexSampler, inTexCoord - vec2(0.0, texelOffset.y * i)).rgb * weight[i];
        }

        //Control the weight of the current blur mip.
        result *= uPostProcessConstants.bloomBlurWeights[uData.currentMip];

        //All mips except the last one should sum with the previous.
        if(uData.currentMip < BLOOM_TEXTURE_MIPS - 1) {
            result += texture(uPreviousMipTexSampler, inTexCoord).rgb;
        }
    }

    outColor = vec4(result, 1.0);
}