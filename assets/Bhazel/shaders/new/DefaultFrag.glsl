#version 450 core
#pragma shader_stage(fragment)

layout (set = 0, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
    vec4 dirLightsDirectionsAndIntensities[2];
    vec4 dirLightColors[2];
    int dirLightsCount;
} uSceneConstants;

layout(set = 0, binding = 1) uniform samplerCube uIrradianceMapTexSampler;
layout(set = 0, binding = 2) uniform samplerCube uRadianceMapTexSampler;

layout (set = 2, binding = 0, std140) uniform MaterialConstants {
     float parallaxOcclusionScale;
} uMaterialConstants;

layout(set = 2, binding = 1) uniform sampler2D uAlbedoTexSampler;
layout(set = 2, binding = 2) uniform sampler2D uNormalTexSampler;
layout(set = 2, binding = 3) uniform sampler2D uMetallicTexSampler;
layout(set = 2, binding = 4) uniform sampler2D uRoughnessTexSampler;
layout(set = 2, binding = 5) uniform sampler2D uHeightTexSampler;
layout(set = 2, binding = 6) uniform sampler2D uBrdfLookupTexture; //TODO: this doesn't belong to the Material DescriptorSet

layout(location = 0) in struct {
    mat3 TBN; //TBN matrix goes from tangent space to world space
    vec2 texCoord;

    //From here, all in tangent space
    vec3 position;
    vec3 L[2];
    vec3 V;
} inData;

layout(location = 0) out vec4 outColor;

#define PI 3.14159265359


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

vec3 indirectLight(vec3 N, vec3 V, vec3 F0, vec3 albedo, float roughness) {
    float NdotV = max(dot(N, V), 0.0);

    vec3 F = fresnelSchlick(NdotV, F0);
    vec3 kSpecular = F;
    vec3 kDiffuse = 1.0 - kSpecular;

    vec3 cubeDirection = normalize(inData.TBN * N);
    cubeDirection.x = -cubeDirection.x;

    vec3 irradiance = texture(uIrradianceMapTexSampler, cubeDirection).rgb;
    vec3 diffuse = albedo * irradiance;

    const float MAX_REFLECTION_LOD = 7.0; //TODO: not harcoded
    vec3 R = reflect(-V, N);
    vec3 worldR = normalize(inData.TBN * R);
    worldR.x = -worldR.x;

    vec3 prefilteredColor = textureLod(uRadianceMapTexSampler, worldR, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF = texture(uBrdfLookupTexture, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    return (kDiffuse * diffuse + specular);// * ao; 
}

vec3 lighting(vec3 N, vec3 V, vec2 texCoord) {
    vec3 albedo = texture(uAlbedoTexSampler, texCoord).rgb;
    float metallic = texture(uMetallicTexSampler, texCoord).r;
    float roughness = texture(uRoughnessTexSampler, texCoord).r;

    vec3 F0 = mix(vec3(0.04), albedo, metallic); //Hardcoded reflectance for dielectrics

    vec3 col = indirectLight(N, V, F0, albedo, roughness);
    for(int i = 0; i < uSceneConstants.dirLightsCount; ++i) {
        vec3 L = normalize(inData.L[i]);
        col += directLight(N, V, L, albedo, F0, roughness, uSceneConstants.dirLightColors[i].xyz * uSceneConstants.dirLightsDirectionsAndIntensities[i].w, texCoord);
    }
    return col;
}

vec2 parallaxOcclusionMap(vec2 texCoord, vec3 viewDirTangentSpace) {
    const float LAYERS = 10;
    float layerDepth = 1.0 / LAYERS;
    float currentLayerDepth = 0.0;

    vec2 P = viewDirTangentSpace.xy * uMaterialConstants.parallaxOcclusionScale; 
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
    return prevTexCoords * weight + currentTexCoords * (1.0 - weight);
}

void main() {
    vec3 V = normalize(inData.V);

    vec2 texCoord = parallaxOcclusionMap(inData.texCoord, V);
    if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
        discard;

    vec3 N = normalize(texture(uNormalTexSampler, texCoord).rgb * 2.0 - 1.0);

    vec3 col = lighting(N, V, texCoord);
    outColor = vec4(col, 1.0);
}