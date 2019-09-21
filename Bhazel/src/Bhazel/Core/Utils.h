#pragma once

#undef near
#undef far

namespace BZ::Utils {

    std::string trim(const std::string &in);
    std::string getFileNameFromPath(const std::string &path);
    std::string removeExtensionFromFileName(const std::string &fileName);

    glm::mat4 ortho(float left, float right, float bottom, float top, float near, float far);
    glm::mat4 frustum(float left, float right, float bottom, float top, float near, float far);
    glm::mat4 perspective(float fovy, float aspectRatio, float near, float far);
}