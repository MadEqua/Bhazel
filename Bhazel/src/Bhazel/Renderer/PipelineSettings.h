#pragma once


namespace BZ {

    enum class BlendingFunction {
        Zero,
        One,
        SourceColor,
        OneMinusSourceColor,
        DestinationColor,
        OneMinusDestinationColor,
        SourceAlpha,
        OneMinusSourceAlpha,
        DestinationAlpha,
        OneMinusDestinationAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        AlphaSaturate, //(R, G, B) = (f, f, f) with f = min(As, 1 - Ad). A = 1

        //Dual Source Blending (Fragment shader outputting 2 colors to the same buffer)
        Source1Color,
        OneMinusSource1Color,
        Source1Alpha,
        OneMinusSource1Alpha
    };

    enum class BlendingEquation {
        Add,
        SourceMinusDestination,
        DestinationMinusSource,

        //Ignores BlendingFunction
        Min,
        Max
    };

    enum class TestFunction {
        Always, Never,
        Less, LessOrEqual,
        Greater, GreaterOrEqual,
        Equal, NotEqual
    };

    //Final Color = BlendingEquation(SourceBlendingFunction * SourceColor, DestinationBlendingFunction * DestinationColor)
    struct BlendingSetting {

        bool operator==(const BlendingSetting &rhs) const;
        bool operator!=(const BlendingSetting &rhs) const;

        bool needsConstantColor() const;

        BlendingFunction sourceBlendingFunction;
        BlendingFunction destinationBlendingFunction;
        BlendingEquation blendingEquation;
    };

    struct BlendingSettings {
        //If true init a common default: Src * Src.a + Dest * (1 - Src.a)
        explicit BlendingSettings(bool enableBlending);

        BlendingSettings(const BlendingSetting &settingRGBA);
        BlendingSettings(const BlendingSetting &settingRGB, const BlendingSetting &settingAlpha);
        BlendingSettings(const BlendingSetting &settingRGB, const BlendingSetting &settingAlpha, const glm::vec4 &constantColor);

        bool operator==(const BlendingSettings &rhs) const;
        bool operator!=(const BlendingSettings &rhs) const;

        bool needsConstantColor() const { return settingRGB.needsConstantColor() || settingAlpha.needsConstantColor(); }

        bool enableBlending;
        BlendingSetting settingRGB;
        BlendingSetting settingAlpha;
        glm::vec4 constantColor = {0.0f, 0.0f, 0.0f, 0.0f};
    };


    struct DepthSettings {
        //Init common defaults: TestFunction = LessOrEqual, DepthClearValue = 1
        explicit DepthSettings(bool enableDepthTest, bool enableDepthWrite);

        explicit DepthSettings(bool enableDepthTest, TestFunction testFunction, bool enableDepthWrite, float depthClearValue);

        bool enableDepthTest;
        TestFunction testFunction;
        bool enableDepthWrite;
        float depthClearValue; //[0, 1]
    };
}
