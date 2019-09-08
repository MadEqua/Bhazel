#include "bzpch.h"

#include "Utils.h"


namespace BZ::Utils {

    std::string trim(const std::string &in) {
        size_t first = in.find_first_not_of(' ');
        if(first == std::string::npos)
            return "";
        size_t last = in.find_last_not_of(' ');
        return in.substr(first, (last - first + 1));
    }

    std::string getFileNameFromPath(const std::string &path) {
        size_t lastSeparatorPos = path.find_last_of("/\\");
        size_t subStrStart = lastSeparatorPos == std::string::npos ? 0 : lastSeparatorPos + 1;

        std::string nameAndExt = path.substr(subStrStart);
        return removeExtensionFromFileName(nameAndExt);
    }

    std::string removeExtensionFromFileName(const std::string &fileName) {
        size_t dotPos = fileName.find_last_of('.');
        if(dotPos != std::string::npos) {
            return fileName.substr(0, dotPos);
        }
        return fileName;
    }
}