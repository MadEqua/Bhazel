#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 attrPosition;
layout(location = 1) in vec3 attrNormal;
layout(location = 2) in vec3 attrTangent;
layout(location = 3) in vec3 attrBitangent;
layout(location = 4) in vec2 attrTexCoord;

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPositionAndDirLightCount;
    vec4 dirLightsDirectionsAndIntensities[2];
    vec4 dirLightColors[2];
    float radianceMapMips;
} uSceneConstants;

layout (set = 2, binding = 0, std140) uniform EntityConstants {
    mat4 modelMatrix;
    mat4 normalMatrix;
} uEntityConstants;

layout(location = 0) out struct {
    mat3 TBN; //TBN matrix goes from tangent space to world space
    vec2 texCoord;

    //From here, all in tangent space
    vec3 position;
    vec3 L[2];
    vec3 V;
} outData;

void main() {
    vec4 positionWorld = uEntityConstants.modelMatrix * vec4(attrPosition, 1.0);
    gl_Position = uSceneConstants.viewProjectionMatrix * positionWorld;

    outData.TBN = mat3(uEntityConstants.normalMatrix) * mat3(attrTangent, attrBitangent, attrNormal);
    outData.texCoord = attrTexCoord;

    //Multiply on the left is equal to multiply with the transpose (= inverse in this case). So transforming from world to tangent space.
    outData.position = positionWorld.xyz * outData.TBN;

    for(int i = 0; i < uSceneConstants.cameraPositionAndDirLightCount.w; ++i) {
        outData.L[i] = -normalize(uSceneConstants.dirLightsDirectionsAndIntensities[i].xyz * outData.TBN);
    }

    outData.V = normalize((uSceneConstants.cameraPositionAndDirLightCount.xyz - positionWorld.xyz) * outData.TBN);
}