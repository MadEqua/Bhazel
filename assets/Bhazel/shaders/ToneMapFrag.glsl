#version 450 core
#pragma shader_stage(fragment)

layout(location = 0) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform sampler2D uInputTexSampler;
layout(set = 0, binding = 1) uniform sampler2D uBlurTexSampler;

//TODO: All of this just to get the camera exposure.
layout (set = 0, binding = 2, std140) uniform PassConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPositionAndExposure;
} uPassConstants;

layout(location = 0) out vec4 outColor;


void main() {
    //Add the color pass with the blurred image coming from the bloom pass.
    vec3 hdrColor = texture(uInputTexSampler, inTexCoord).rgb + texture(uBlurTexSampler, inTexCoord).rgb;

    vec3 mapped = vec3(1.0) - exp(-hdrColor * uPassConstants.cameraPositionAndExposure.w);

    outColor = vec4(mapped, 1.0);
}