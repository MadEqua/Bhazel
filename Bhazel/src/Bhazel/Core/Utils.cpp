#include "bzpch.h"

#include "Utils.h"
#include "Bhazel/Renderer/Renderer.h"
#include  <glm/gtc/matrix_transform.hpp>


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
        if(Renderer::api == Renderer::API::OpenGL)
            return glm::orthoRH_NO(left, right, bottom, top, near, far);
        else if(Renderer::api == Renderer::API::D3D11 || Renderer::api == Renderer::API::Vulkan)
            return glm::orthoRH_ZO(left, right, bottom, top, near, far);
        else {
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return glm::mat4();
        }
    }

    glm::mat4 frustum(float left, float right, float bottom, float top, float near, float far) {
        if(Renderer::api == Renderer::API::OpenGL)
            return glm::frustumRH_NO(left, right, bottom, top, near, far);
        else if(Renderer::api == Renderer::API::D3D11 || Renderer::api == Renderer::API::Vulkan)
            return glm::frustumRH_ZO(left, right, bottom, top, near, far);
        else {
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return glm::mat4();
        }
    }

    glm::mat4 perspective(float fovy, float aspectRatio, float near, float far) {
        if(Renderer::api == Renderer::API::OpenGL)
            return glm::perspectiveRH_NO(glm::radians(fovy), aspectRatio, near, far);
        else if(Renderer::api == Renderer::API::D3D11 || Renderer::api == Renderer::API::Vulkan)
            return glm::perspectiveRH_ZO(glm::radians(fovy), aspectRatio, near, far);
        else {
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return glm::mat4();
        }
    }
}