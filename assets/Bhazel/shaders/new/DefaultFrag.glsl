#version 450 core
#pragma shader_stage(fragment)

layout (set = 1, binding = 0, std140) uniform SceneConstants {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    vec4 dirLightsDirectionsAndIntensities[2];
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

vec3 cookTorrance(vec3 N, vec3 L, vec3 V, vec3 lightRadiance) {
    vec3 H = normalize(V + L);
        
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 albedo = texture(uAlbedoTexSampler, inData.texCoord).rgb;
    float metallic = texture(uMetallicTexSampler, inData.texCoord).r;
    float roughness = texture(uRoughnessTexSampler, inData.texCoord).r;

    vec3 F0 = mix(vec3(0.04), albedo, metallic); //Hardcoded reflectance for dielectrics
    vec3 F = fresnelSchlick(HdotV, F0);

    float G = geometrySmith(NdotV, NdotL, roughness);
    float NDF = distributionGGX(NdotH, roughness);
        
    vec3 specular = (NDF * G * F) / max((4.0 * NdotV * NdotL), 0.001);
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (vec3(1.0) - metallic);
    return (kD * albedo / PI + specular) * lightRadiance * NdotL;
}

void main() {
    vec3 col = vec3(0.0);
    mat3 TBN = mat3(normalize(inData.tbnMatrix[0]), normalize(inData.tbnMatrix[1]), normalize(inData.tbnMatrix[2]));

    vec3 N = TBN * normalize((texture(uNormalTexSampler, inData.texCoord).rgb * 2.0 - 1.0));
    vec3 V = normalize(uSceneConstants.cameraPosition - inData.position);

    for(int i = 0; i < uSceneConstants.dirLightsCount; ++i) {
        vec3 L = -normalize(uSceneConstants.dirLightsDirectionsAndIntensities[i].xyz);

        //May apply ambient occlusion here.
        vec3 amb = vec3(0.01);

        col += amb + cookTorrance(N, L, V, uSceneConstants.dirLightColors[i] * uSceneConstants.dirLightsDirectionsAndIntensities[i].w);
        //col = vec3(diffuse, diffuse, diffuse);
        //col = vec3(spec, spec, spec);
        //col = inData.tbnMatrix[2] *0.5+0.5;
        //col = N *0.5+0.5;
        //col = vec3(inData.texCoord, 0.0);
        //col = texture(uNormalTexSampler, inData.texCoord).rgb;
        //col = vec3(1,0,0);
    }

    outColor = vec4(col, 1.0);
    //outColor = vec4(1.0,0.0,0.0, 1.0);
    //outColor = vec4(inTexCoord, 0.0, 1.0).yyyy;
    //outColor = texture(uTexSampler, inTexCoord);
}