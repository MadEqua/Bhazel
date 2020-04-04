#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;
layout(location = 1) in vec3 attrNormal;
layout(location = 2) in vec3 attrTangent;
layout(location = 3) in vec3 attrBitangent;
layout(location = 4) in vec2 attrTexCoord;

layout (set = 0, binding = 0, std140) uniform FrameConstants {
    vec2 timeAndDelta;
} uFrameConstants;

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    vec3 dirLightsDirectionsAndIntensities[2];
    vec3 dirLightColors[2];
    int dirLightsCount;
} uSceneConstants;

layout (set = 2, binding = 0, std140) uniform ObjectConstants {
    mat4 modelMatrix;
    mat3 normalMatrix;
} uObjectConstants;

layout(location = 0) out struct {
    //All in tangent space
    vec3 position;
    vec3 L[2];
    vec3 V;
    vec2 texCoord;
} outData;

void main() {
    vec4 positionWorld = uObjectConstants.modelMatrix * vec4(attrPosition, 1.0);
    gl_Position = uSceneConstants.viewProjectionMatrix * positionWorld;

    //TBN matrix goes from tangent space to world space
    mat3 TBN = uObjectConstants.normalMatrix * mat3(attrTangent, attrBitangent, attrNormal);

    //Multiply on the left is equal to multiply with the transpose (= inverse in this case). So transforming from world to tangent space.
    outData.position = positionWorld.xyz * TBN;

    for(int i = 0; i < uSceneConstants.dirLightsCount; ++i) {
        outData.L[i] = -normalize(uSceneConstants.dirLightsDirectionsAndIntensities[i].xyz * TBN);
    }

    outData.V = normalize((uSceneConstants.cameraPosition - positionWorld.xyz) * TBN);
    //outData.N = normalize((uObjectConstants.normalMatrix * attrNormal) * TBN);
    outData.texCoord = attrTexCoord;
}