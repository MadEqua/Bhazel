#version 450 core
#pragma shader_stage(vertex)

#define MAX_DIR_LIGHTS_PER_SCENE 2

layout(location = 0) in vec3 attrPosition;
layout(location = 1) in vec3 attrNormal;
layout(location = 2) in vec3 attrTangent;
layout(location = 3) in vec3 attrBitangent;
layout(location = 4) in vec2 attrTexCoord;

layout (set = 1, binding = 0, std140) uniform PassConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
} uPassConstants;

layout (set = 2, binding = 0, std140) uniform SceneConstants {
    mat4 lightMatrices[MAX_DIR_LIGHTS_PER_SCENE];
    vec4 dirLightDirectionsAndIntensities[MAX_DIR_LIGHTS_PER_SCENE];
    vec4 dirLightColors[MAX_DIR_LIGHTS_PER_SCENE];
    vec2 dirLightCountAndRadianceMapMips;
} uSceneConstants;

layout (set = 3, binding = 0, std140) uniform EntityConstants {
    mat4 modelMatrix;
    mat4 normalMatrix;
} uEntityConstants;

layout(location = 0) out struct {
    mat3 TBN; //TBN matrix goes from tangent space to world space
    vec2 texCoord;

    //Light NDC space
    vec3 positionsLightNDC[MAX_DIR_LIGHTS_PER_SCENE];

    //From here, all in tangent space
    //vec3 positionTan;
    vec3 LTan[MAX_DIR_LIGHTS_PER_SCENE];
    vec3 VTan;
} outData;


void main() {
    vec4 positionWorld = uEntityConstants.modelMatrix * vec4(attrPosition, 1.0);
    gl_Position = uPassConstants.viewProjectionMatrix * positionWorld;

    outData.TBN = mat3(uEntityConstants.normalMatrix) * mat3(attrTangent, attrBitangent, attrNormal);
    outData.texCoord = attrTexCoord;

    //Multiply on the left is equal to multiply with the transpose (= inverse in this case). So transforming from world to tangent space.
    //outData.positionTan = positionWorld.xyz * outData.TBN;

    for(int i = 0; i < uSceneConstants.dirLightCountAndRadianceMapMips.x; ++i) {
        vec4 posLightClip = uSceneConstants.lightMatrices[i] * positionWorld;
        outData.positionsLightNDC[i] = posLightClip.xyz / posLightClip.w;
        outData.LTan[i] = -normalize(uSceneConstants.dirLightDirectionsAndIntensities[i].xyz * outData.TBN);
    }

    outData.VTan = normalize((uPassConstants.cameraPosition.xyz - positionWorld.xyz) * outData.TBN);
}