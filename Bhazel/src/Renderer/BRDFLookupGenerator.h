#pragma once

#include <fstream>


namespace BZ {

glm::vec2 hammersley(uint32_t i, uint32_t N) {
    // Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
    uint32_t bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * static_cast<float>(2.3283064365386963e-10);
    return glm::vec2(float(i) / float(N), rdi);
}

float G1(float k, float NoV) {
    return NoV / (NoV * (1.0f - k) + k);
}

// Geometric Shadowing function
float gSmith(float NoL, float NoV, float roughness) {
    float k = (roughness * roughness) * 0.5f;
    return G1(k, NoL) * G1(k, NoV);
}

// Sample a half-vector in world space
// Based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
glm::vec3 importanceSampleGGX(glm::vec2 Xi, float roughness, glm::vec3 N) {
    // Maps a 2D point to a hemisphere with spread based on roughness
    float a = roughness * roughness;
    float phi = 2.0f * glm::pi<float>() * Xi.x;
    float cosTheta = sqrt(glm::clamp((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y), 0.0f, 1.0f));
    float sinTheta = sqrt(glm::clamp(1.0f - cosTheta * cosTheta, 0.0f, 1.0f));

    glm::vec3 H = glm::vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

    glm::vec3 up = glm::abs(N.z) < 0.999 ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
    glm::vec3 tangent = glm::normalize(glm::cross(up, N));
    glm::vec3 bitangent = glm::cross(N, tangent);

    return glm::normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
glm::vec2 integrateBRDF(float roughness, float NoV) {
    const glm::vec3 N = glm::vec3(0.0, 0.0, 1.0); // normal always pointing forward.
    const glm::vec3 V = glm::vec3(sqrt(glm::clamp(1.0 - NoV * NoV, 0.0, 1.0)), 0.0, NoV);
    float A = 0.0f;
    float B = 0.0f;

    uint32_t numSamples = 1024u;
    for (uint32_t i = 0u; i < numSamples; ++i) {
        glm::vec2 Xi = hammersley(i, numSamples);
        glm::vec3 H = importanceSampleGGX(Xi, roughness, N);
        glm::vec3 L = 2.0f * glm::dot(V, H) * H - V;

        float NoL = glm::max(glm::dot(N, L), 0.0f);
        if (NoL > 0.0f) {
            float NoH = glm::max(glm::dot(N, H), 0.001f);
            float VoH = glm::max(glm::dot(V, H), 0.001f);
            float currentNoV = glm::max(glm::dot(N, V), 0.001f);

            const float G = gSmith(NoL, currentNoV, roughness);

            const float G_Vis = (G * VoH) / (NoH * currentNoV /*avoid division by zero*/);
            const float Fc = pow(1.0f - VoH, 5.0f);

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    return glm::vec2(A, B) / float(numSamples);
}

// Call this to generate the BRDF Lookup table as a data array on a .h file ready to be used.
void generateCookTorranceBRDFLUT(uint32 mapDim = 256) {
    // byte *data = new byte[mapDim * mapDim * sizeof(glm::detail::hdata) * 2];

    std::ofstream file;
    file.open("BRDFLookup.h");
    file << "#pragma once\n";
    file << "namespace BZ {\n";
    file << "constexpr uint32 brdfLutSize = " << mapDim << ";\n";
    file << "//AUTO-GENERATED data in 16bit-Floating point format.\n";
    file << "constexpr uint16 const brdfLut[] = {\n";

    uint32 offset = 0;
    for (uint32 j = 0; j < mapDim; ++j) {
        for (uint32 i = 0; i < mapDim; ++i) {
            glm::vec2 v2 = integrateBRDF((static_cast<float>(j) + .5f) / static_cast<float>(mapDim),
                                         ((static_cast<float>(i) + .5f) / static_cast<float>(mapDim)));
            uint16 halfR = glm::detail::toFloat16(v2.r);
            uint16 halfG = glm::detail::toFloat16(v2.g);

            file << halfR << "," << halfG << ",";

            // memcpy(data + offset, &halfR, sizeof(uint16));
            // memcpy(data + offset + sizeof(uint16), &halfG, sizeof(uint16));
            offset += sizeof(uint16) * 2;
        }
    }

    file << "\n};}";
    file.close();

    // Ref<Texture2D> texture = Texture2D::create(data, mapDim, mapDim, TextureFormatEnum::R16G16_SFLOAT,
    // MipmapData::Options::DoNothing); delete [] data; return texture;
}
}