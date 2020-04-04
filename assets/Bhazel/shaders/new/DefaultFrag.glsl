#version 450 core
#pragma shader_stage(fragment)

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    vec3 dirLightDirections[2];
    vec3 dirLightColors[2];
    int dirLightsCount;
} uSceneConstants;

layout(location = 0) in struct {
    vec3 position;
    vec2 texCoord;
    mat3 tbnMatrix;
} inData;

layout(set = 3, binding = 0) uniform sampler2D uAlbedoTexSampler;
layout(set = 3, binding = 1) uniform sampler2D uNormalTexSampler;
layout(set = 3, binding = 2) uniform sampler2D uMetallicTexSampler;
layout(set = 3, binding = 3) uniform sampler2D uRoughnessTexSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 col = vec3(0.0);
    mat3 TBN = mat3(normalize(inData.tbnMatrix[0]), normalize(inData.tbnMatrix[1]), normalize(inData.tbnMatrix[2]));

    for(int i = 0; i < uSceneConstants.dirLightsCount; ++i) {
        vec3 L = -normalize(uSceneConstants.dirLightDirections[i]);
        vec3 N = TBN * normalize((texture(uNormalTexSampler, inData.texCoord).rgb * 2.0 - 1.0));
        vec3 V = normalize(uSceneConstants.cameraPosition - inData.position);
        vec3 H = normalize(L + V);

        vec3 amb = vec3(0.01);
        float diffuse = max(0.0, dot(L, N));
        float spec = pow(max(0.0, dot(H, N)), 10.0);

        col += amb + (diffuse * texture(uAlbedoTexSampler, inData.texCoord).rgb * uSceneConstants.dirLightColors[i]) + (spec * vec3(1.0));
        //col = vec3(diffuse, diffuse, diffuse);
        //col = vec3(spec, spec, spec);
        //col = inData.tbnMatrix[2] *0.5+0.5;
        //col = N *0.5+0.5;
        //col = vec3(inData.texCoord.x, 0.0, 0.0);
        //col = texture(uNormalTexSampler, inData.texCoord).rgb;
    }

    outColor = vec4(col, 1.0);
    //outColor = vec4(normalize(inNormal), 1.0);
    //outColor = vec4(inTexCoord, 0.0, 1.0).yyyy;
    //outColor = texture(uTexSampler, inTexCoord);
}