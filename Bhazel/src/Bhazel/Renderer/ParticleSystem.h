#pragma once

#include "PipelineSettings.h"

namespace BZ {

    class Buffer;
    class InputDescription;
    class Shader;
    class Texture;

    //Emits particles on a unit cube [-0.5, 0.5]. Use scale (and position) to map to world coords.
    //This unit cube makes easy to make a visual editor for any particle system.
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
            alignas(16) glm::vec2 size;
        };

        //In Local coords
        struct ParticleRanges {
            RangeVec3 positionRange;
            RangeVec3 sizeRange;
            RangeFloat lifeRange; //Seconds
            RangeVec3 velocityRange;
            RangeVec3 accelerationRange;
            RangeVec3 tintRange;

            ParticleRanges();
        };

    public:
        ParticleSystem(uint32 particleCount);

        void init();
        void render();

        /*void setPosition(const glm::vec3 &position) { this->position = position; }
        void setScale(const glm::vec3 &scale) { this->scale = scale; }
        void setRotation(const glm::vec3 &eulerAngles) { this->eulerAngles = eulerAngles; }

        const glm::vec3& getPosition() const { return position; }
        const glm::vec3& getScale() const { return position; }
        const glm::vec3& getRotation() const { return eulerAngles; }*/

        //TODO: Transform class with all of this. And caching of the resultant matrix.
        glm::vec3 position;
        glm::vec3 eulerAngles;
        glm::vec3 scale;

        ParticleRanges ranges;

    private:
        uint32 particleCount;

        Ref<Buffer> quadVertexBuffer;
        Ref<InputDescription> quadInputDescription;

        Ref<Buffer> computeBuffer;

        Ref<Shader> computeShader;
        Ref<Shader> particleShader;
        
        Ref<Buffer> constantBuffer;

        Ref<Texture> particleTexture;

        BlendingSettings particleBlendingSettings;
        BlendingSettings disableBlendingSettings;

        constexpr static int WORK_GROUP_SIZE = 128;
    };
}