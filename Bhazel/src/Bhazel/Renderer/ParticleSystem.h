#pragma once

namespace BZ {

    class Buffer;
    class InputDescription;
    class Shader;

    class ParticleSystem 
    {
    private:
        struct alignas(16) RangeFloat {
            float min;
            float max;

            RangeFloat(const float &min, const float &max) : min(min), max(max) {}
            RangeFloat(const float &v) : min(v), max(v) {}
        };

        struct RangeVec3 {
            alignas(16) glm::vec3 min;
            alignas(16) glm::vec3 max;

            RangeVec3(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max) {}
            RangeVec3(const glm::vec3 &v) : min(v), max(v) {}
        };

        struct Particle {
            glm::vec4 positionAndLife;
            alignas(16) glm::vec3 velocity;
            alignas(16) glm::vec3 acceleration;
            alignas(16) glm::vec3 tint;
        };

        struct ParticleRanges {
            RangeVec3 positionRange; //Local coords
            RangeFloat lifeRange; //Seconds
            RangeVec3 velocityRange;
            RangeVec3 accelerationRange;
            RangeVec3 tintRange;

            ParticleRanges();
        };

    public:
        ParticleSystem(uint32 particleCount);

        void init();
        void render(const glm::vec3 &position);

        ParticleRanges ranges;

    private:
        uint32 particleCount;

        Ref<Buffer> buffer;
        Ref<InputDescription> inputDescription;
        Ref<Shader> computeShader;
        Ref<Shader> particleShader;
        Ref<Buffer> constantBuffer;

        constexpr static int WORK_GROUP_SIZE = 128;
    };
}