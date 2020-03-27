#include "bzpch.h"

#include "Utils.h"
#include "Graphics/Graphics.h"
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

    std::string removeFileNameFromPath(const std::string &path) {
        size_t lastSeparatorPos = path.find_last_of("/\\");
        return path.substr(0, lastSeparatorPos + 1);
    }

    std::string removeExtensionFromFileName(const std::string &fileName) {
        size_t dotPos = fileName.find_last_of('.');
        if(dotPos != std::string::npos) {
            return fileName.substr(0, dotPos);
        }
        return fileName;
    }

    glm::mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
        if(Graphics::api == Graphics::API::OpenGL)
            return glm::orthoRH_NO(left, right, bottom, top, near, far);
        else if(Graphics::api == Graphics::API::D3D11 || Graphics::api == Graphics::API::Vulkan)
            return glm::orthoRH_ZO(left, right, bottom, top, near, far);
        else {
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return glm::mat4();
        }
    }

    glm::mat4 frustum(float left, float right, float bottom, float top, float near, float far) {
        if(Graphics::api == Graphics::API::OpenGL)
            return glm::frustumRH_NO(left, right, bottom, top, near, far);
        else if(Graphics::api == Graphics::API::D3D11 || Graphics::api == Graphics::API::Vulkan)
            return glm::frustumRH_ZO(left, right, bottom, top, near, far);
        else {
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return glm::mat4();
        }
    }

    glm::mat4 perspective(float fovy, float aspectRatio, float near, float far) {
        if(Graphics::api == Graphics::API::OpenGL)
            return glm::perspectiveRH_NO(glm::radians(fovy), aspectRatio, near, far);
        else if(Graphics::api == Graphics::API::D3D11 || Graphics::api == Graphics::API::Vulkan)
            return glm::perspectiveRH_ZO(glm::radians(fovy), aspectRatio, near, far);
        else {
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return glm::mat4();
        }
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