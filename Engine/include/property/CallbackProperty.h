#pragma once

#include <property/PropertyUtil.h>

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>

enum class CallbackPropertyType { Bool, Char, Int, Float, Double, String, Color, Vec2, Vec3, Vec4, Mat3, Mat4, Enum };

// ptr is const: the view may read the current value to display it, but must never
// write through it. On edit the view passes the new value to callback(const void*),
// and the owning component applies it through its own setter (preserving invariants).
struct CallbackProperty {
    std::string name;
    CallbackPropertyType type;
    std::function<void(const void*)> callback;
    std::vector<const char*> enumValues;
    const void* ptr;
};

struct CallbackPropertyHash {
    size_t operator()(const CallbackProperty& p) const {
        return std::hash<std::string>()(p.name) ^
            (std::hash<int>()(static_cast<int>(p.type)) << 1);
    }
};

struct CallbackPropertyEqual {
    bool operator()(const CallbackProperty& a, const CallbackProperty& b) const {
        return a.name == b.name && a.type == b.type;
    }
};

struct CallbackPropertyGroup {
    std::string name;
    std::map<unsigned int, CallbackProperty> properties;
    std::unordered_set<CallbackProperty, CallbackPropertyHash, CallbackPropertyEqual> seen;

    unsigned int nextPropIdx = 0;

    void reset() {
        properties.clear();
        seen.clear();
        nextPropIdx = 0;
    }
};

class CallbackPropertyHolder {
public:
    void registerCallback(const std::string& name, CallbackPropertyType type, const void* ptr, std::vector<const char*> enumValues,
        std::function<void(const void*)> callback = nullptr, unsigned int groupID = 0, const std::string& groupName = "defaultGroup", CallbackProperty prop = {}) {

        prop.enumValues = std::move(enumValues);
        registerCallback(name, type, ptr, callback, groupID, groupName, std::move(prop));
    }

    void registerCallback(const std::string& name, CallbackPropertyType type, const void* ptr, std::function<void(const void*)> callback = nullptr,
        unsigned int groupID = 0, const std::string& groupName = "defaultGroup", CallbackProperty prop = {}) {
        if (groupID >= callbackPropertyGroups.size())
            callbackPropertyGroups.resize(groupID + 1);

        CallbackPropertyGroup& group = callbackPropertyGroups[groupID];
        if (group.name.empty())
            group.name = camelToWords(groupName);

        std::string propName = camelToWords(name);

        prop.name = propName;
        prop.type = type;
        prop.callback = callback;
        prop.ptr = ptr;

        if (group.seen.count(prop)) return;

        group.properties[group.nextPropIdx++] = prop;
        group.seen.insert(prop);
    }

    std::map<unsigned int, CallbackProperty>& getCallbackProperties(unsigned int groupID = 0) {
        static std::map<unsigned int, CallbackProperty> empty;
        if (groupID < callbackPropertyGroups.size()) return callbackPropertyGroups[groupID].properties;
        return empty;
    }
    std::vector<CallbackPropertyGroup>& getCallbackPropertyGroups() { return callbackPropertyGroups; }

protected:
    std::vector<CallbackPropertyGroup> callbackPropertyGroups;
};