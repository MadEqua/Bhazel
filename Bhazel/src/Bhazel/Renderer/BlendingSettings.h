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

    //Final Color = BlendingEquation(SourceBlendingFunction * SourceColor, DestinationBlendingFunction * DestinationColor)
    struct BlendingSetting {
        BlendingSetting() = default;
        BlendingSetting(BlendingFunction sourceBlendingFunction, BlendingFunction destinationBlendingFunction, BlendingEquation blendingEquation);

        bool operator==(const BlendingSetting &rhs) const;
        bool operator!=(const BlendingSetting &rhs) const;

        bool needsConstantColor() const;

        BlendingFunction sourceBlendingFunction;
        BlendingFunction destinationBlendingFunction;
        BlendingEquation blendingEquation;
    };

    struct BlendingSettings {
        BlendingSettings();
        BlendingSettings(const BlendingSetting &settingRGBA);
        BlendingSettings(const BlendingSetting &settingRGB, const BlendingSetting &settingAlpha);
        BlendingSettings(const BlendingSetting &settingRGB, const BlendingSetting &settingAlpha, const glm::vec4 &constantColor);

        bool operator==(const BlendingSettings &rhs) const;
        bool operator!=(const BlendingSettings &rhs) const;

        inline bool needsConstantColor() const { return settingRGB.needsConstantColor() || settingAlpha.needsConstantColor(); }

        bool enableBlending;
        BlendingSetting settingRGB;
        BlendingSetting settingAlpha;
        glm::vec4 constantColor = {0.0f, 0.0f, 0.0f, 0.0f};
    };
}
