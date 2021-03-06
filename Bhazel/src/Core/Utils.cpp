#include "bzpch.h"

#include "Utils.h"


namespace BZ::Utils {

std::string trim(const std::string &in) {
    size_t first = in.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";
    size_t last = in.find_last_not_of(' ');
    return in.substr(first, (last - first + 1));
}

std::string getFileNameFromPath(const std::string &path) {
    size_t lastSeparatorPos = path.find_last_of("/\\");
    size_t subStrStart = lastSeparatorPos == std::string::npos ? 0 : lastSeparatorPos + 1;
    return path.substr(subStrStart);
}

std::string removeFileNameFromPath(const std::string &path) {
    size_t lastSeparatorPos = path.find_last_of("/\\");
    return path.substr(0, lastSeparatorPos + 1);
}

std::string getExtensionFromFileName(const std::string &fileName) {
    size_t dotPos = fileName.find_last_of('.');
    size_t subStrStart = dotPos == std::string::npos ? 0 : dotPos + 1;
    return fileName.substr(subStrStart);
}

std::string removeExtensionFromFileName(const std::string &fileName) {
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos) {
        return fileName.substr(0, dotPos);
    }
    return fileName;
}

std::string appendToFileName(const std::string &pathWithFilename, const std::string &toAppend) {
    size_t dotPos = pathWithFilename.find_last_of('.');
    if (dotPos != std::string::npos) {
        return pathWithFilename.substr(0, dotPos) + toAppend + pathWithFilename.substr(dotPos, pathWithFilename.size());
    }
    else {
        return pathWithFilename + toAppend;
    }
}

glm::mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
    glm::mat4 mat = glm::orthoRH_ZO(left, right, bottom, top, near, far);
    mat[1] = -mat[1]; // Take into account Vulkan clip space, y is down.
    return mat;
}

glm::mat4 frustum(float left, float right, float bottom, float top, float near, float far) {
    glm::mat4 mat = glm::frustumRH_ZO(left, right, bottom, top, near, far);
    mat[1] = -mat[1]; // Take into account Vulkan clip space, y is down.
    return mat;
}

glm::mat4 perspective(float fovy, float aspectRatio, float near, float far) {
    glm::mat4 mat = glm::perspectiveRH_ZO(glm::radians(fovy), aspectRatio, near, far);
    mat[1] = -mat[1]; // Take into account Vulkan clip space, y is down.
    return mat;
}

std::size_t hashCombine(std::size_t hash1, std::size_t hash2) {
    return hash1 ^ (hash2 * 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
}

uint32 packColor(const glm::vec4 &color) {
    uint32 pack = 0;
    pack |= static_cast<uint8>(color.a * 255.0f) << 24;
    pack |= static_cast<uint8>(color.r * 255.0f) << 16;
    pack |= static_cast<uint8>(color.g * 255.0f) << 8;
    pack |= static_cast<uint8>(color.b * 255.0f);
    return pack;
}

glm::vec4 unpackColor(uint32 color) {
    glm::vec4 vec;
    vec.a = ((color >> 24) & 255) / 255.0f;
    vec.r = ((color >> 16) & 255) / 255.0f;
    vec.g = ((color >> 8) & 255) / 255.0f;
    vec.b = (color & 255) / 255.0f;
    return vec;
}
}