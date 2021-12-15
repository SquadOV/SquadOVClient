#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace shared::valve {

class VdfObject;
using VdfObjectPtr = std::shared_ptr<VdfObject>;

class VdfObject {
public:
    bool hasKey(const std::string& key) const;
    const VdfObject& operator[](const std::string& key) const;
    const std::string& value() const { return _value; }

    void setObject(const std::string& key, const VdfObjectPtr& object);
    void setValue(const std::string& value);
private:
    std::unordered_map<std::string, VdfObjectPtr> _kvp;
    std::string _value;
};

VdfObject readVdfFile(const std::filesystem::path& path);

}