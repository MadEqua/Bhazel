#version 450 core
#pragma shader_stage(fragment)

#define MAX_DIR_LIGHTS_PER_SCENE 2

layout(set = 0, binding = 0) uniform sampler2D uBrdfLookupTexture;

layout (set = 2, binding = 0, std140) uniform SceneConstants {
    mat4 lightMatrices[MAX_DIR_LIGHTS_PER_SCENE];
    vec4 dirLightDirectionsAndIntensities[MAX_DIR_LIGHTS_PER_SCENE];
    vec4 dirLightColors[MAX_DIR_LIGHTS_PER_SCENE];
    vec2 dirLightCountAndRadianceMapMips;
} uSceneConstants;

layout(set = 2, binding = 1) uniform samplerCube uIrradianceMapTexSampler;
layout(set = 2, binding = 2) uniform samplerCube uRadianceMapTexSampler;
layout(set = 2, binding = 3) uniform sampler2DShadow uShadowMapSamplers[MAX_DIR_LIGHTS_PER_SCENE];

layout (set = 4, binding = 0, std140) uniform MaterialConstants {
    vec4 normalMetallicRoughnessAndAO;
    vec4 heightAndUvScale;
} uMaterialConstants;

layout(set = 4, binding = 1) uniform sampler2D uAlbedoTexSampler;
layout(set = 4, binding = 2) uniform sampler2D uNormalTexSampler;
layout(set = 4, binding = 3) uniform sampler2D uMetallicTexSampler;
layout(set = 4, binding = 4) uniform sampler2D uRoughnessTexSampler;
layout(set = 4, binding = 5) uniform sampler2D uHeightTexSampler;
layout(set = 4, binding = 6) uniform sampler2D uAOTexSampler;

layout(location = 0) in struct {
    //TBN matrix goes from tangent space to world space
    mat3 TBN;
    vec2 texCoord;

    //Light NDC space
    vec3 positionsLightNDC[MAX_DIR_LIGHTS_PER_SCENE];

    //From here, all in tangent space
    //vec3 positionTan;
    vec3 LTan[MAX_DIR_LIGHTS_PER_SCENE];
    vec3 VTan;
    vec3 NTan;
} inData;

layout(location = 0) out vec4 outColor;

#define PI 3.14159265359


vec2 parallaxOcclusionMap(vec2 texCoord, vec3 viewDirTangentSpace) {
    //If no Height Texture is bound
    if(uMaterialConstants.heightAndUvScale.x < 0.0)
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
    //vec2 envBRDF = texture(uBrdfLookupTexture, vec2(NdotV, roughness)).rg;
    vec2 envBRDF = texture(uBrdfLookupTexture, vec2(NdotV, roughness)).rg;
    vec3 specular = radiance * (F * envBRDF.x + envBRDF.y);

    float ao = uMaterialConstants.normalMetallicRoughnessAndAO.w > 0.0 ? texture(uAOTexSampler, texCoord).r : 1.0f;
    return (kDiffuse * diffuse + specular) * ao; 
}

float shadowMapping(int lightIdx, vec3 N, vec3 L) {
    if(inData.positionsLightNDC[lightIdx].z > 1.0)
        return 1.0;

    vec2 shadowMapTexCoord = inData.positionsLightNDC[lightIdx].xy * 0.5 + 0.5;
    //In Vulkan texCoord y=0 is the top line. This texture was not flipped by Bhazel like the ones loaded from disk, so flip it here.
    shadowMapTexCoord.y = 1.0 - shadowMapTexCoord.y;
    
    vec3 coordAndCompare = vec3(shadowMapTexCoord, inData.positionsLightNDC[lightIdx].z);
    return texture(uShadowMapSamplers[lightIdx], coordAndCompare);
}

vec3 lighting(vec3 N, vec3 V, vec2 texCoord) {
    vec3 albedo = texture(uAlbedoTexSampler, texCoord).rgb;
    float metallic = uMaterialConstants.normalMetallicRoughnessAndAO.y < 0.0 ? texture(uMetallicTexSampler, texCoord).r : uMaterialConstants.normalMetallicRoughnessAndAO.y;
    float roughness = uMaterialConstants.normalMetallicRoughnessAndAO.z < 0.0 ? texture(uRoughnessTexSampler, texCoord).r : uMaterialConstants.normalMetallicRoughnessAndAO.z;

    //Hardcoded reflectance for dielectrics
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 col = indirectLight(N, V, F0, albedo, roughness, texCoord);
    for(int i = 0; i < int(uSceneConstants.dirLightCountAndRadianceMapMips.x); ++i) {
        vec3 L = normalize(inData.LTan[i]);

        float shadow = shadowMapping(i, N, L);
        col += shadow * directLight(N, V, L, albedo, F0, roughness, uSceneConstants.dirLightColors[i].xyz * uSceneConstants.dirLightDirectionsAndIntensities[i].w, texCoord);
    }
    return col;
}

void main() {
    vec3 V = normalize(inData.VTan);

    const vec2 uvScale = uMaterialConstants.heightAndUvScale.yz;
    vec2 texCoord = parallaxOcclusionMap(inData.texCoord * uvScale, V);

    vec3 N = normalize(uMaterialConstants.normalMetallicRoughnessAndAO.x > 0.0 ? (texture(uNormalTexSampler, texCoord).rgb * 2.0 - 1.0) : inData.NTan);

    vec3 col = lighting(N, V, texCoord);
    //col =  texture(uAlbedoTexSampler, texCoord).rgb;
    outColor = vec4(col, 1.0);
}