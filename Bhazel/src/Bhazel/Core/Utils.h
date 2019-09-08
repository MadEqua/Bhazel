#pragma once

namespace BZ::Utils {

    std::string trim(const std::string &in);
    std::string getFileNameFromPath(const std::string &path);
    std::string removeExtensionFromFileName(const std::string &fileName);
}