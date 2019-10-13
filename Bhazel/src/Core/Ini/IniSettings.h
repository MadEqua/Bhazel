#pragma once


namespace BZ {

    class IniSettings {
    public:
        bool containsField(const std::string &name) const;

        template<class T>
        T getFieldAsBasicType(const std::string &fieldName, T defaultValue = T()) const;

        const std::string& getFieldAsString(const std::string &fieldName, const std::string &defaultValue = "") const;

        void addField(const std::string &name, const std::string &value);

    private:
        std::unordered_map<std::string, std::string> fields;
    };
}