#pragma once

#include "IniSettings.h"


namespace BZ {

    class IniParser {
    public:
        bool parse(const std::string &filePath);

        inline const IniSettings& getParsedIniSettings() { return out; }

    private:
        IniSettings out;
    };
}