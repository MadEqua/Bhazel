#version 450 core
#pragma shader_stage(fragment)

#define MAX_DIR_LIGHTS_PER_SCENE 2
#define SHADOW_MAPPING_CASCADE_COUNT 4

layout(set = 0, binding = 0) uniform sampler2D uBrdfLookupTexture;

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 lightMatrices[MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT];
    vec4 dirLightDirectionsAndIntensities[MAX_DIR_LIGHTS_PER_SCENE];
    vec4 dirLightColors[MAX_DIR_LIGHTS_PER_SCENE];
    vec4 cascadeSplits; //View space
    vec2 dirLightCountAndRadianceMapMips;
} uSceneConstants;

layout(set = 1, binding = 1) uniform samplerCube uIrradianceMapTexSampler;
layout(set = 1, binding = 2) uniform samplerCube uRadianceMapTexSampler;
layout(set = 1, binding = 3) uniform sampler2DArrayShadow uShadowMapSamplers[MAX_DIR_LIGHTS_PER_SCENE];

layout (set = 3, binding = 0, std140) uniform MaterialConstants {
    vec4 normalMetallicRoughnessAndAO;
    vec4 heightAndUvScale;
} uMaterialConstants;

layout(set = 3, binding = 1) uniform sampler2D uAlbedoTexSampler;
layout(set = 3, binding = 2) uniform sampler2D uNormalTexSampler;
layout(set = 3, binding = 3) uniform sampler2D uMetallicTexSampler;
layout(set = 3, binding = 4) uniform sampler2D uRoughnessTexSampler;
layout(set = 3, binding = 5) uniform sampler2D uHeightTexSampler;
layout(set = 3, binding = 6) uniform sampler2D uAOTexSampler;

layout(location = 0) in struct {
    //TBN matrix goes from tangent space to world space
    mat3 TBN;
    vec2 texCoord;

    //In Light NDC space
    vec3 positionsLightNDC[MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT];

    //View space
    vec3 positionView;

    //From here, all in tangent space
    //vec3 positionTan;
    vec3 LTan[MAX_DIR_LIGHTS_PER_SCENE];
    vec3 VTan;
    vec3 NTan;
} inData;

layout(location = 0) out vec4 outColor;

#define PI 3.14159265359

bool hasNormalMap = uMaterialConstants.normalMetallicRoughnessAndAO.x > 0.0;
bool hasMetallicMap = uMaterialConstants.normalMetallicRoughnessAndAO.y < 0.0;
bool hasRoughnessMap = uMaterialConstants.normalMetallicRoughnessAndAO.z < 0.0;
bool hasAOMap = uMaterialConstants.normalMetallicRoughnessAndAO.w > 0.0;
bool hasHeightMap = uMaterialConstants.heightAndUvScale.x > 0.0;


vec2 parallaxOcclusionMap(vec2 texCoord, vec3 viewDirTangentSpace) {
    if(!hasHeightMap)
        return texCoord;

    const float LAYERS = 10;
    float layerDepth = 1.0 / LAYERS;
    float currentLayerDepth = 0.0;

    vec2 P = viewDirTangentSpace.xy * uMaterialConstants.heightAndUvScale.x;
    vec2 deltaTexCoords = P / LAYERS;

    vec2  currentTexCoords = texCoord;
    float currentDepthMapValue = 1.0 - texture(uHeightTexSampler, currentTexCoords).r;
    
    while(currentLayerDepth < currentDepthMapValue) {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = 1.0 - texture(uHeightTexSampler, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = (1.0 - texture(uHeightTexSampler, prevTexCoords).r) - currentLayerDepth + layerDepth;
    
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 ret = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    const vec2 uvScale = uMaterialConstants.heightAndUvScale.yz;
    if(ret.x > uvScale.x || ret.y > uvScale.y || ret.x < 0.0 || ret.y < 0.0)
        discard;

    return ret;
}

vec3 fresnelSchlick(float HdotV, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);
}

float distributionGGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
    
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float geometrySmith(float NdotV, float NdotL, float roughness) {
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

//Cook-Torrance
vec3 directLight(vec3 N, vec3 V, vec3 L, vec3 albedo, vec3 F0, float roughness, vec3 lightRadiance, vec2 texCoord) {
    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 F = fresnelSchlick(HdotV, F0);

    float G = geometrySmith(NdotV, NdotL, roughness);
    float NDF = distributionGGX(NdotH, roughness);

    vec3 specular = (NDF * G * F) / max((4.0 * NdotV * NdotL), 0.001);

    vec3 kDiffuse = 1.0 - F;
    return (kDiffuse * albedo / PI + specular) * lightRadiance * NdotL;
}

vec3 indirectLight(vec3 N, vec3 V, vec3 F0, vec3 albedo, float roughness, vec2 texCoord) {
    float NdotV = max(dot(N, V), 0.0);

    vec3 F = fresnelSchlick(NdotV, F0);
    vec3 kSpecular = F;
    vec3 kDiffuse = 1.0 - kSpecular;

    vec3 cubeDirection = normalize(inData.TBN * N);
    cubeDirection.x = -cubeDirection.x;

    vec3 irradiance = texture(uIrradianceMapTexSampler, cubeDirection).rgb;
    vec3 diffuse = albedo * irradiance;

    vec3 R = reflect(-V, N);
    vec3 worldR = normalize(inData.TBN * R);
    worldR.x = -worldR.x;

    vec3 radiance = textureLod(uRadianceMapTexSampler, worldR, roughness * uSceneConstants.dirLightCountAndRadianceMapMips.y).rgb;
    vec2 envBRDF = texture(uBrdfLookupTexture, vec2(NdotV, roughness)).rg;
    vec3 specular = radiance * (F * envBRDF.x + envBRDF.y);

    float ao = hasAOMap ? texture(uAOTexSampler, texCoord).r : 1.0f;
    return (kDiffuse * diffuse + specular) * ao; 
}

int findShadowMapCascade() {
    for(int cascadeIdx = 0; cascadeIdx < SHADOW_MAPPING_CASCADE_COUNT; ++cascadeIdx) {
        if(inData.positionView.z > uSceneConstants.cascadeSplits[cascadeIdx])
            return cascadeIdx;
    }
}

float shadowMapping(int lightIdx, int cascadeIdx) {
    //Try to use cascade i - 1 even if we are on cascade i. Possible because the interceptions between cascades.
    int previousCascade = max(0, cascadeIdx - 1);
    for(int i = previousCascade; i <= cascadeIdx; ++i) {
        vec3 posLightNDC = inData.positionsLightNDC[i + lightIdx * SHADOW_MAPPING_CASCADE_COUNT];

        if(posLightNDC.x < -1.0 || posLightNDC.x > 1.0 ||
           posLightNDC.y < -1.0 || posLightNDC.y > 1.0 ||
           posLightNDC.z < 0.0 || posLightNDC.z > 1.0) {
            continue;
        }
        else {
            vec2 shadowMapTexCoord = posLightNDC.xy * 0.5 + 0.5;
            //In Vulkan texCoord y=0 is the top line. This texture was not flipped by Bhazel like the ones loaded from disk, so flip it here.
            shadowMapTexCoord.y = 1.0 - shadowMapTexCoord.y;

            vec4 coordLayerAndCompare = vec4(shadowMapTexCoord, float(i), posLightNDC.z);
            return texture(uShadowMapSamplers[lightIdx], coordLayerAndCompare);
        }
    }
    return 1.0;
}

//float shadowMapping2(int lightIdx, int cascadeIdx) {
//    vec3 posLightNDC = inData.positionsLightNDC[0];
//
//    vec2 shadowMapTexCoord = posLightNDC.xy * 0.5 + 0.5;
//    //In Vulkan texCoord y=0 is the top line. This texture was not flipped by Bhazel like the ones loaded from disk, so flip it here.
//    shadowMapTexCoord.y = 1.0 - shadowMapTexCoord.y;
//
//    vec4 coordLayerAndCompare = vec4(shadowMapTexCoord, 0.0, posLightNDC.z);
//    return texture(uShadowMapSamplers[lightIdx], coordLayerAndCompare);
//}

vec3 lighting(vec3 N, vec3 V, vec2 texCoord) {
    vec3 albedo = texture(uAlbedoTexSampler, texCoord).rgb;
    float metallic = hasMetallicMap ? texture(uMetallicTexSampler, texCoord).r : uMaterialConstants.normalMetallicRoughnessAndAO.y;
    float roughness = hasRoughnessMap ? texture(uRoughnessTexSampler, texCoord).r : uMaterialConstants.normalMetallicRoughnessAndAO.z;

    //Hardcoded reflectance for dielectrics
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 col = indirectLight(N, V, F0, albedo, roughness, texCoord);
    int cascadeIdx = findShadowMapCascade();

    for(int lightIdx = 0; lightIdx < int(uSceneConstants.dirLightCountAndRadianceMapMips.x); ++lightIdx) {
        vec3 L = normalize(inData.LTan[lightIdx]);

        float shadow = shadowMapping(lightIdx, cascadeIdx);
        col += shadow * directLight(N, V, L, albedo, F0, roughness, uSceneConstants.dirLightColors[lightIdx].xyz * uSceneConstants.dirLightDirectionsAndIntensities[lightIdx].w, texCoord);
    }
    return col;
}

/*vec3 cascadeColor(int cascade) {
    if(cascade==0) return vec3(1,0,0);
    if(cascade==1) return vec3(1,1,0);
    if(cascade==2) return vec3(0,1,0);
    if(cascade==3) return vec3(0.5,0.5,0.5);
}

vec3 debugCascades() {
    for(int cascade = 0; cascade < SHADOW_MAPPING_CASCADE_COUNT; ++cascade) {
        if(inData.positionView.z > uSceneConstants.cascadeSplits[cascade]) {
            return cascadeColor(cascade);
        }
    }
}*/

void main() {
    vec3 V = normalize(inData.VTan);

    const vec2 uvScale = uMaterialConstants.heightAndUvScale.yz;
    vec2 texCoord = parallaxOcclusionMap(inData.texCoord * uvScale, V);

    vec3 N = normalize(hasNormalMap ? (texture(uNormalTexSampler, texCoord).rgb * 2.0 - 1.0) : inData.NTan);

    vec3 col = lighting(N, V, texCoord);
    outColor = vec4(col, 1.0);
}