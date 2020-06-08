#pragma once

#include "IniSettings.h"


namespace BZ {

class IniParser {
  public:
    IniParser() = default;

    BZ_NON_COPYABLE(IniParser);

    bool parse(const char *filePath);

    const IniSettings &getParsedIniSettings() { return out; }

  private:
    IniSettings out;
};
}