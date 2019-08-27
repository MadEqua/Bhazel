#pragma once

//#include <glm/glm.hpp>

#include "Bhazel/Renderer/Shader.h"

#include "D3D11Context.h"
#include "D3D11Includes.h"


namespace BZ {

    class D3D11Shader : public Shader {
    public:
        D3D11Shader(const std::string &vertexSrc, const std::string &fragmentSrc);

        void bind() const;
        void unbind() const;

        ID3DBlob* getVertexShaderBlob() { return vertexShaderBlobPtr.Get(); }

        /*void setUniformInt(const std::string &name, int v);

        void setUniformFloat(const std::string &name, float v);
        void setUniformFloat2(const std::string &name, const glm::vec2 &vec);
        void setUniformFloat3(const std::string &name, const glm::vec3 &vec);
        void setUniformFloat4(const std::string &name, const glm::vec4 &vec);

        void setUniformMat3(const std::string &name, const glm::mat3 &mat);
        void setUniformMat4(const std::string &name, const glm::mat4 &mat);*/

    private:
        D3D11Context &context;

        wrl::ComPtr<ID3DBlob> vertexShaderBlobPtr; //Used for InputDescription validation

        wrl::ComPtr<ID3D11VertexShader> vertexShaderPtr;
        wrl::ComPtr<ID3D11PixelShader> pixelShaderPtr;
    };
}