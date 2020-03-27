#pragma once

#undef near
#undef far

namespace BZ::Utils {

    std::string trim(const std::string &in);
    std::string getFileNameFromPath(const std::string &path);
    std::string removeFileNameFromPath(const std::string &path);
    std::string removeExtensionFromFileName(const std::string &fileName);

    glm::mat4 ortho(float left, float right, float bottom, float top, float near, float far);
    glm::mat4 frustum(float left, float right, float bottom, float top, float near, float far);
    glm::mat4 perspective(float fovy, float aspectRatio, float near, float far);

    std::size_t hashCombine(std::size_t hash1, std::size_t hash2);

    //Returns ARGB
    uint32 packColor(const glm::vec4 &color);
    glm::vec4 unpackColor(uint32 color);
}