#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;
layout(location = 1) in vec3 attrNormal;
layout(location = 2) in vec3 attrTangent;
layout(location = 3) in vec3 attrBitangent;
layout(location = 4) in vec2 attrTexCoord;

layout (set = 0, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
    vec4 dirLightsDirectionsAndIntensities[2];
    vec4 dirLightColors[2];
    int dirLightsCount;
} uSceneConstants;

layout (set = 1, binding = 0, std140) uniform EntityConstants {
    mat4 modelMatrix;
    mat4 normalMatrix;
} uEntityConstants;

layout(location = 0) out struct {
    vec3 worldN;
    vec2 texCoord;

    //From here, all in tangent space
    vec3 position;
    vec3 L[2];
    vec3 V;
} outData;

void main() {
    vec4 positionWorld = uEntityConstants.modelMatrix * vec4(attrPosition, 1.0);
    gl_Position = uSceneConstants.viewProjectionMatrix * positionWorld;

    //TBN matrix goes from tangent space to world space
    mat3 TBN = mat3(uEntityConstants.normalMatrix) * mat3(attrTangent, attrBitangent, attrNormal);

    outData.worldN = normalize(mat3(uEntityConstants.normalMatrix) * attrNormal);
    outData.texCoord = attrTexCoord;

    //Multiply on the left is equal to multiply with the transpose (= inverse in this case). So transforming from world to tangent space.
    outData.position = positionWorld.xyz * TBN;

    for(int i = 0; i < uSceneConstants.dirLightsCount; ++i) {
        outData.L[i] = -normalize(uSceneConstants.dirLightsDirectionsAndIntensities[i].xyz * TBN);
    }

    outData.V = normalize((uSceneConstants.cameraPosition.xyz - positionWorld.xyz) * TBN);
}