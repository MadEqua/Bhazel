#version 450 core
#pragma shader_stage(fragment)

layout(location = 0) in vec3 inCubeMapDirection;

layout(set = 3, binding = 1) uniform samplerCube uTexSampler;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outThresholdColor;


void main() {
    vec3 col = texture(uTexSampler, normalize(inCubeMapDirection)).rgb;
    outColor = vec4(col, 1.0);

    //Output threshold color to attachment 1.
    float brightness = dot(col, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        outThresholdColor = vec4(col, 1.0);
    else
        outThresholdColor = vec4(0.0, 0.0, 0.0, 1.0);
}