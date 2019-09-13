#pragma once

namespace BZ::Utils {

    std::string trim(const std::string &in);
    std::string getFileNameFromPath(const std::string &path);
    std::string removeExtensionFromFileName(const std::string &fileName);

    glm::mat4 ortho(float left, float right, float bottom, float top, float zNear, float zFar);
    glm::mat4 frustum(float left, float right, float bottom, float top, float zNear, float zFar);
    glm::mat4 perspective(float fovy, float aspectRatio, float zNear, float zFar);
}