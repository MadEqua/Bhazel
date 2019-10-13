#include "bzpch.h"

#include "Utils.h"
#include <glm/gtc/matrix_transform.hpp>


namespace BZ::Utils {

    std::string trim(const std::string &in) {
        size_t first = in.find_first_not_of(' ');
        if(first == std::string::npos)
            return "";
        size_t last = in.find_last_not_of(' ');
        return in.substr(first, (last - first + 1));
    }

    std::string getFileNameFromPath(const std::string &path) {
        size_t lastSeparatorPos = path.find_last_of("/\\");
        size_t subStrStart = lastSeparatorPos == std::string::npos ? 0 : lastSeparatorPos + 1;

        std::string nameAndExt = path.substr(subStrStart);
        return removeExtensionFromFileName(nameAndExt);
    }

    std::string removeExtensionFromFileName(const std::string &fileName) {
        size_t dotPos = fileName.find_last_of('.');
        if(dotPos != std::string::npos) {
            return fileName.substr(0, dotPos);
        }
        return fileName;
    }

    glm::mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
#ifdef BZ_PLATFORM_OPENGL43
        return glm::orthoRH_NO(left, right, bottom, top, near, far);
#elif defined BZ_PLATFORM_VULKAN or defined BZ_PLATFORM_D3D11
        return glm::orthoRH_ZO(left, right, bottom, top, near, far);
#endif
    }

    glm::mat4 frustum(float left, float right, float bottom, float top, float near, float far) {
#ifdef BZ_PLATFORM_OPENGL43
        return glm::frustumRH_NO(left, right, bottom, top, near, far);
#elif defined BZ_PLATFORM_VULKAN or defined BZ_PLATFORM_D3D11
        return glm::frustumRH_ZO(left, right, bottom, top, near, far);
#endif
    }

    glm::mat4 perspective(float fovy, float aspectRatio, float near, float far) {
#ifdef BZ_PLATFORM_OPENGL43
        return glm::perspectiveRH_NO(glm::radians(fovy), aspectRatio, near, far);
#elif defined BZ_PLATFORM_VULKAN or defined BZ_PLATFORM_D3D11
        return glm::perspectiveRH_ZO(glm::radians(fovy), aspectRatio, near, far);
#endif
    }
}