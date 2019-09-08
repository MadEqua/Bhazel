#include "bzpch.h"

#include "IniParser.h"
#include "Bhazel/Core/Utils.h"

#include <fstream>


namespace BZ {

    bool IniParser::parse(const std::string &filePath) {
        std::ifstream inFile(filePath, std::ios::in);
        if(!inFile.is_open()) {
            BZ_LOG_CORE_ERROR("IniParser failed opening file {0}.", filePath);
            return false;
        }

        std::string line;
        while(std::getline(inFile, line)) {
            line = Utils::trim(line);
            size_t equalPos = line.find('=');
            if(line[0] != ';' && equalPos != std::string::npos) {
                std::string fieldName = Utils::trim(line.substr(0, equalPos));
                if(out.containsField(fieldName)) {
                    BZ_LOG_CORE_WARN("IniParser. Duplicate field on .ini file: {0}. Only considering the first appearance.", filePath);
                    continue;
                }

                std::string fieldValue = Utils::trim(line.substr(equalPos + 1));
                out.addField(fieldName, fieldValue);
            }
        }
        inFile.close();
        return true;
    }
}