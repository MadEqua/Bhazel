#include "bzpch.h"

#include "Utils.h"


namespace BZ {

    std::string trim(const std::string &in) {
        size_t first = in.find_first_not_of(' ');
        if(first == std::string::npos)
            return "";
        size_t last = in.find_last_not_of(' ');
        return in.substr(first, (last - first + 1));
    }
}