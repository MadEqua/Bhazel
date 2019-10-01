#pragma once


namespace BZ {

    enum class ShaderType {
        Vertex,
        Fragment,
        Compute,
        Unknown
    };

    class Shader {
    public:
        static Ref<Shader> createFromSource(const std::string &filePath);
        static Ref<Shader> createFromSource(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);

        static Ref<Shader> createFromBlob(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
        
       const std::string& getName() const { return name; }

    protected:
        explicit Shader(const std::string& name) : name(name) {}
        virtual ~Shader() = default;

        std::string name;

        static std::unordered_map<ShaderType, std::string> readAndPreprocessFile(const std::string &filePath);
        static std::vector<char> readBlobFile(const std::string& filePath);

        static ShaderType shaderTypeFromString(const std::string &string);
    };


    class ShaderLibrary {
    public:
        void add(const std::string &name, const Ref<Shader> &shader);
        void add(const Ref<Shader> &shader);

        Ref<Shader> loadFromSource(const std::string &filepath);
        Ref<Shader> loadFromSource(const std::string &name, const std::string &filepath);
        Ref<Shader> loadFromBlob(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);

        Ref<Shader> get(const std::string &name);
        
        bool exists(const std::string &name) const;

    private:
        std::unordered_map<std::string, Ref<Shader>> shaders;
    };
}