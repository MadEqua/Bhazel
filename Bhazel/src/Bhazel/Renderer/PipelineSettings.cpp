#include "bzpch.h"

#include "PipelineSettings.h"


namespace BZ {

    bool BlendingSetting::operator==(const BlendingSetting &rhs) const {
        return sourceBlendingFunction == rhs.sourceBlendingFunction &&
               destinationBlendingFunction == rhs.destinationBlendingFunction &&
               blendingEquation == rhs.blendingEquation;
    }

    bool BlendingSetting::operator!=(const BlendingSetting &rhs) const {
        return !(rhs == *this);
    }

    bool BlendingSetting::needsConstantColor() const {
        return sourceBlendingFunction == BlendingFunction::ConstantColor ||
               sourceBlendingFunction == BlendingFunction::ConstantAlpha ||
               sourceBlendingFunction == BlendingFunction::OneMinusConstantColor||
               sourceBlendingFunction == BlendingFunction::OneMinusConstantAlpha ||
               destinationBlendingFunction == BlendingFunction::ConstantColor ||
               destinationBlendingFunction == BlendingFunction::ConstantAlpha ||
               destinationBlendingFunction == BlendingFunction::OneMinusConstantColor ||
               destinationBlendingFunction == BlendingFunction::OneMinusConstantColor;
    }

    BlendingSettings::BlendingSettings(bool enableBlending) :
        enableBlending(enableBlending),
        settingRGB({BlendingFunction::SourceAlpha, BlendingFunction::OneMinusSourceAlpha, BlendingEquation::Add}),
        settingAlpha({BlendingFunction::SourceAlpha, BlendingFunction::OneMinusSourceAlpha, BlendingEquation::Add}) {
    }

    BlendingSettings::BlendingSettings(const BlendingSetting &settingRGBA) :
        enableBlending(true), 
        settingRGB(settingRGBA),
        settingAlpha(settingRGBA) {
    }

    BlendingSettings::BlendingSettings(const BlendingSetting &settingRGB, const BlendingSetting &settingAlpha) :
        enableBlending(true), 
        settingRGB(settingRGB), 
        settingAlpha(settingAlpha) {
    }

    BlendingSettings::BlendingSettings(const BlendingSetting &settingRGB, const BlendingSetting &settingAlpha, 
                                       const glm::vec4 &constantColor) :
        enableBlending(true), 
        settingRGB(settingRGB), 
        settingAlpha(settingAlpha), 
        constantColor(constantColor) {
    }

    bool BlendingSettings::operator==(const BlendingSettings &rhs) const {
        return enableBlending == rhs.enableBlending &&
               settingRGB == rhs.settingRGB &&
               settingAlpha == rhs.settingAlpha &&
               constantColor == rhs.constantColor;
    }

    bool BlendingSettings::operator!=(const BlendingSettings &rhs) const {
        return !(rhs == *this);
    }


    DepthSettings::DepthSettings(bool enableDepthTest, bool enableDepthWrite) :
        enableDepthTest(enableDepthTest), 
        testFunction(TestFunction::LessOrEqual),
        enableDepthWrite(enableDepthWrite),
        depthClearValue(1.0f) {
    }

    DepthSettings::DepthSettings(bool enableDepthTest, TestFunction testFunction, bool enableDepthWrite, float depthClearValue) :
        enableDepthTest(enableDepthTest),
        testFunction(testFunction),
        enableDepthWrite(enableDepthWrite),
        depthClearValue(depthClearValue) {
    }
}