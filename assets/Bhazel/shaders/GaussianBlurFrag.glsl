#version 450 core
#pragma shader_stage(fragment)

layout(location = 0) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform sampler2D uInputTexSampler;
layout(push_constant) uniform Direction {
    uint horizontal;
} direction;

layout(location = 0) out vec4 outColor;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


void main() {
    vec2 texelOffset = 1.0 / textureSize(uInputTexSampler, 0);

    vec3 result = texture(uInputTexSampler, inTexCoord).rgb * weight[0];

    if(direction.horizontal == 1) {
        for(int i = 1; i < 5; ++i) {
            result += texture(uInputTexSampler, inTexCoord + vec2(texelOffset.x * i, 0.0)).rgb * weight[i];
            result += texture(uInputTexSampler, inTexCoord - vec2(texelOffset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else {
        for(int i = 1; i < 5; ++i) {
            result += texture(uInputTexSampler, inTexCoord + vec2(0.0, texelOffset.y * i)).rgb * weight[i];
            result += texture(uInputTexSampler, inTexCoord - vec2(0.0, texelOffset.y * i)).rgb * weight[i];
        }
    }

    outColor = vec4(result, 1.0);
}