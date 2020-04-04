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
    //All in tangent space
    vec3 position;
    vec3 L[2];
    vec3 V;
    vec2 texCoord;
} inData;

layout(set = 3, binding = 0) uniform sampler2D uAlbedoTexSampler;
layout(set = 3, binding = 1) uniform sampler2D uNormalTexSampler;
layout(set = 3, binding = 2) uniform sampler2D uMetallicTexSampler;
layout(set = 3, binding = 3) uniform sampler2D uRoughnessTexSampler;
layout(set = 3, binding = 4) uniform sampler2D uHeightTexSampler;

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

vec3 cookTorrance(vec3 N, vec3 L, vec3 V, vec3 lightRadiance, vec2 texCoord) {
    vec3 H = normalize(V + L);
        
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 albedo = texture(uAlbedoTexSampler, texCoord).rgb;
    float metallic = texture(uMetallicTexSampler, texCoord).r;
    float roughness = texture(uRoughnessTexSampler, texCoord).r;

    vec3 F0 = mix(vec3(0.04), albedo, metallic); //Hardcoded reflectance for dielectrics
    vec3 F = fresnelSchlick(HdotV, F0);

    float G = geometrySmith(NdotV, NdotL, roughness);
    float NDF = distributionGGX(NdotH, roughness);
        
    vec3 specular = (NDF * G * F) / max((4.0 * NdotV * NdotL), 0.001);
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (vec3(1.0) - metallic);
    return (kD * albedo / PI + specular) * lightRadiance * NdotL;
}

vec2 parallaxMap(vec2 texCoord, vec3 viewDirTangentSpace) {
    //float height = 1-texture(uHeightTexSampler, texCoord).r;
    //vec2 p = viewDirTangentSpace.xy / viewDirTangentSpace.z * (height * 0.05);
    //return texCoord - p;

     // number of depth layers
    const float numLayers = 10;
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDirTangentSpace.xy * 0.01; 
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords = texCoord;
    float currentDepthMapValue = 1.0 - texture(uHeightTexSampler, currentTexCoords).r;
    
    while(currentLayerDepth < currentDepthMapValue) {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = 1.0 - texture(uHeightTexSampler, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = (1.0 - texture(uHeightTexSampler, prevTexCoords).r) - currentLayerDepth + layerDepth;
    
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main() {
    
    vec3 V = normalize(inData.V);

    vec2 texCoord = parallaxMap(inData.texCoord, V);
    if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
        discard;

    vec3 N = normalize(texture(uNormalTexSampler, texCoord).rgb * 2.0 - 1.0);

    vec3 col = vec3(0.0);
    for(int i = 0; i < uSceneConstants.dirLightsCount; ++i) {
        vec3 L = normalize(inData.L[i]);

        //May apply ambient occlusion here.
        vec3 amb = vec3(0.01);

        col += amb + cookTorrance(N, L, V, uSceneConstants.dirLightColors[i] * uSceneConstants.dirLightsDirectionsAndIntensities[i].w, texCoord);
        //col = vec3(diffuse, diffuse, diffuse);
        //col = vec3(spec, spec, spec);
        //col = inData.tbnMatrix[2] *0.5+0.5;
        //col = N *0.5+0.5;
        //col = vec3(inData.texCoord, 0.0);
        //col = texture(uNormalTexSampler, inData.texCoord).rgb;
        //col = vec3(1,0,0);
    }

    outColor = vec4(col, 1.0);
    //outColor = texture(uDepthTexSampler, inData.texCoord).rrrr;
    //outColor = vec4(1.0,0.0,0.0, 1.0);
    //outColor = vec4(inTexCoord, 0.0, 1.0).yyyy;
    //outColor = texture(uTexSampler, inTexCoord);
}