#pragma once


namespace BZ {

    enum class ShaderStageFlags {
        Vertex = 1,
        TesselationControl = 2,
        TesselationEvaluation = 4,
        Geometry = 8,
        Fragment = 16,
        Compute = 32,
        GraphicsAll = Vertex | TesselationControl | TesselationEvaluation | Geometry | Fragment,
        All = GraphicsAll | Compute,
    };

    EnumClassFlagOperators(ShaderStageFlags);

    enum class DescriptorType {
        ConstantBuffer
    };

    class DescriptorSetLayout {
    public:
        DescriptorSetLayout(DescriptorType type, uint8 shaderStageVisibilityMask) : 
            type(type), shaderStageVisibilityMask(shaderStageVisibilityMask) {}

       virtual ~DescriptorSetLayout() = 0;

    private:
        uint8 shaderStageVisibilityMask;
        DescriptorType type;
    };
}
