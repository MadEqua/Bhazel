#pragma once

#include "Buffer.h"


namespace BZ {

    enum class ShaderType {
        Vertex,
        Fragment,
        Unknown
    };

    class Shader {
    public:
        explicit Shader(const std::string &name) : name(name) {}
        virtual ~Shader() = default;

        virtual void bindToPipeline() const = 0;
        virtual void unbindFromPipeline() const = 0;

        const std::string& getName() const { return name; }

        static Ref<Shader> create(const std::string &filePath);
        static Ref<Shader> create(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);

    protected:
        std::string name;

        static std::unordered_map<ShaderType, std::string> readAndPreprocessFile(const std::string &filePath);
        static ShaderType shaderTypeFromString(const std::string &string);
    };


    class ShaderLibrary {
    public:
        void add(const std::string &name, const Ref<Shader> &shader);
        void add(const Ref<Shader> &shader);

        Ref<Shader> load(const std::string &filepath);
        Ref<Shader> load(const std::string &name, const std::string &filepath);

        Ref<Shader> get(const std::string &name);
        
        bool exists(const std::string &name) const;

    private:
        std::unordered_map<std::string, Ref<Shader>> shaders;
    };
}