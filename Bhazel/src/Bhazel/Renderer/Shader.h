#pragma once


namespace BZ {

    enum class ShaderStage {
        Vertex,
        TesselationControl,
        TesselationEvaluation,
        Geometry,
        Fragment,
        Compute
    };


    class Shader {
    public:
        static constexpr int SHADER_STAGES_COUNT = 6;

        class Builder {
        public:
            //All shader code on a single file divided by '#type' directives.
            Builder& fromSingleSourceFile(const char *filePath);

            Builder& fromString(ShaderStage stage, const char *code);
            Builder& fromBinaryFile(ShaderStage stage, const char *filePath);
            Builder& fromSourceFile(ShaderStage stage, const char *filePath);
                     
            Builder& setName(const char *name);

            Ref<Shader> build() const;
        private:
            std::optional<bool> useBinaryBlob;
            const char *name = "Unnamed Shader";
            std::array<std::string, SHADER_STAGES_COUNT> codeStrings;
            std::array<std::vector<char>, SHADER_STAGES_COUNT> binaryBlobs;

            static void readAndPreprocessSingleSourceFile(const char *filePath, std::array<std::string, SHADER_STAGES_COUNT> &out);
            static std::vector<char> readBinaryFile(const char *filePath);
            static std::string readSourceFile(const char *filePath);
            static ShaderStage shaderTypeFromString(const std::string &string);
        };

       const std::string& getName() const { return name; }
       uint32 getStageCount() const;
       bool isStagePresent(ShaderStage stage) const;

    protected:
        Shader(const char *name, const std::array<std::string, SHADER_STAGES_COUNT> &codeStrings);
        Shader(const char *name, const std::array<std::vector<char>, SHADER_STAGES_COUNT> &binaryBlobs);
        virtual ~Shader() = default;

        std::bitset<SHADER_STAGES_COUNT> stages;
        std::string name;
    };



    class ShaderLibrary {
    public:
        void add(const char *name, const Ref<Shader> &shader);
        void add(const Ref<Shader> &shader);

        Ref<Shader> get(const char *name);
        bool exists(const char *name) const;

    private:
        std::unordered_map<std::string, Ref<Shader>> shaders;
    };
}