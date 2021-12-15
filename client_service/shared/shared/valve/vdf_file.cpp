#include "shared/valve/vdf_file.h"
#include "shared/log/log.h"

#include <boost/algorithm/string.hpp>
#include <deque>
#include <fstream>
#include <optional>
#include <sstream>

namespace shared::valve {
namespace {

constexpr char DELIMITER_STRING = '"';
constexpr char DELIMITER_OBJECT_START = '{';
constexpr char DELIMITER_OBJECT_END = '}';

class VdfObjectBuilder {
public:
    VdfObjectBuilder();
    VdfObjectBuilder(const std::string& key, const VdfObjectPtr& parentObject);

    const VdfObject& object() const { return *_object; }

    bool addChar(char c);
private:
    // Final object
    VdfObjectPtr _object;

    // Parent state (if any)
    std::optional<std::string> _parentKey;
    VdfObjectPtr _parentObject;

    // State so we can iteratively build up the VdfObject.
    std::deque<char> _delimiterStack;
    std::shared_ptr<VdfObjectBuilder> _subobjectBuilder;
    std::ostringstream _currentString;

    // Need to keep track of whether or not we started processing an object.
    // This way subobject builders can safely ignore processing whitespace and the like
    // before the actual start of the object where something gets put onto the delimiter stack
    // for the first time.
    bool _beganProcessing = false;
    bool _isObject = false;
};

VdfObjectBuilder::VdfObjectBuilder(const std::string& key, const VdfObjectPtr& parentObject):
    _parentKey(key),
    _parentObject(parentObject),
    _isObject(false)
{
    _object = std::make_shared<VdfObject>();
    _parentObject->setObject(key, _object);
}

VdfObjectBuilder::VdfObjectBuilder():
    _isObject(true)
{
    _object = std::make_shared<VdfObject>();
}

bool VdfObjectBuilder::addChar(char c) {
    if (_subobjectBuilder) {
        if (_subobjectBuilder->addChar(c)) {
            // In the case that the subobject is finished processing then we can
            // remove clear out the builder (the VdfObject relationship is setup in the
            // build construction) so we can move onto the next key/value pair.
            _subobjectBuilder.reset();
        }
    } else if (c == DELIMITER_STRING) {
        if (!_delimiterStack.empty() && _delimiterStack.back() == c) {
            // In this case we are finishing up a string - we need to determine whether or not this is a key or value.
            // If _isObject is set to true, then that means this is a key because any 'value' will be handled by a subobjectBuilder.
            // If _isObject is set to false, then that means this is a value because we created a string without first encounter a { } (object delimiter).
            //    And that should mean a parent key/object is set.
            auto finalStr = _currentString.str();
            boost::replace_all(finalStr, "\\\\", "\\");

            if (_isObject) {
                _subobjectBuilder = std::make_shared<VdfObjectBuilder>(finalStr, _object);
            } else {
                _object->setValue(finalStr);
            }

            _currentString.str("");
            _currentString.clear();
            _delimiterStack.pop_back();
        } else {
            // In this case we're starting up a new string - nothing fancy we need to do here
            // other than just to indicate that it's been started.
            _delimiterStack.push_back(c);
            _beganProcessing = true;
        }
    } else if (c == DELIMITER_OBJECT_START) {
        _beganProcessing = true;
        _isObject = true;
        _delimiterStack.push_back(c);
    } else if (!_delimiterStack.empty() && c == DELIMITER_OBJECT_END) {
        if (_delimiterStack.back() != DELIMITER_OBJECT_START) {
            LOG_WARNING("Delimiter stack corruption? Found '" << _delimiterStack.back() << "' when expecting '" << DELIMITER_OBJECT_START << "'!" << std::endl);
        }

        _delimiterStack.pop_back();
        _isObject = false;
    } else if (!_delimiterStack.empty() && _delimiterStack.back() == DELIMITER_STRING) {
        // Note that we only care to add to the string when we're actually trying to read in a string and we're not just
        // in between objects/keys.
        _currentString << c;
    }

    // Need to return whether or not we're "finished".
    // This is only relevant in the case of a subobject where we've completely finished processing the input object and need to
    // return future processing up one level.
    if (!_parentKey) {
        return false;
    }

    return _beganProcessing && _delimiterStack.empty();
}
    
}

VdfObject readVdfFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    VdfObjectBuilder builder;

    char c;
    while (file >> std::noskipws >> c) {
        builder.addChar(c);
    }

    return builder.object();
}

bool VdfObject::hasKey(const std::string& key) const {
    const auto it = _kvp.find(key);
    return (it != _kvp.end());
}

const VdfObject& VdfObject::operator[](const std::string& key) const {
    return *_kvp.at(key);
}

void VdfObject::setObject(const std::string& key, const VdfObjectPtr& object) {
    _kvp[key] = object;
}

void VdfObject::setValue(const std::string& value) {
    _value = value;
}

}