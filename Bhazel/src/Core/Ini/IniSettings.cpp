#include "bzpch.h"

#include "IniSettings.h"


namespace BZ {

template bool IniSettings::getFieldAsBasicType<bool>(const std::string &fieldName, bool defaultValue) const;
template int IniSettings::getFieldAsBasicType<int>(const std::string &fieldName, int defaultValue) const;
template uint32 IniSettings::getFieldAsBasicType<uint32>(const std::string &fieldName, uint32 defaultValue) const;
template float IniSettings::getFieldAsBasicType<float>(const std::string &fieldName, float defaultValue) const;


bool IniSettings::containsField(const std::string &name) const {
    return fields.find(name) != fields.end();
}

template <class T> T IniSettings::getFieldAsBasicType(const std::string &fieldName, T defaultValue) const {
    auto it = fields.find(fieldName);
    if (it != fields.end()) {
        std::istringstream iss(it->second);
        T value;
        iss >> value;
        return value;
    }
    else
        return defaultValue;
}

const std::string &IniSettings::getFieldAsString(const std::string &fieldName, const std::string &defaultValue) const {
    auto it = fields.find(fieldName);
    if (it != fields.end())
        return it->second;
    else
        return defaultValue;
}

void IniSettings::addField(const std::string &name, const std::string &value) {
    fields[name] = value;
}
}