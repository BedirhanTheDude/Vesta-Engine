#pragma once

#include <property/PropertyUtil.h>

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>

enum class PropertyType { Bool, Char, Int, Float, Double, String, Color, Vec2, Vec3, Vec4, Mat3, Mat4, Enum };

struct Property {
    std::string name;
    PropertyType type;
    std::function<void()> callback;
    std::vector<const char*> enumValues;
    void* ptr;
};

struct PropertyHash {
    size_t operator()(const Property& p) const {
        return std::hash<std::string>()(p.name) ^
            (std::hash<int>()(static_cast<int>(p.type)) << 1);
    }
};

struct PropertyEqual {
    bool operator()(const Property& a, const Property& b) const {
        return a.name == b.name && a.type == b.type;
    }
};

struct PropertyGroup {
    std::string name;
    std::map<unsigned int, Property> properties;
    std::unordered_set<Property, PropertyHash, PropertyEqual> seen;

    unsigned int nextPropIdx = 0;

    void reset() {
        properties.clear();
        seen.clear();
        nextPropIdx = 0;
    }
};

class PropertyHolder {
public:
    void registerProperty(const std::string& name, PropertyType type, void* ptr, std::vector<const char*> enumValues,
        std::function<void()> callback = nullptr, unsigned int groupID = 0, const std::string& groupName = "defaultGroup", Property prop = {}) {
        
        prop.enumValues = std::move(enumValues);
        registerProperty(name, type, ptr, callback, groupID, groupName, std::move(prop));
    }

    void registerProperty(const std::string& name, PropertyType type, void* ptr, std::function<void()> callback = nullptr,
        unsigned int groupID = 0, const std::string& groupName = "defaultGroup", Property prop = {}) {
        if (groupID >= propertyGroups.size())
            propertyGroups.resize(groupID + 1);

        PropertyGroup& group = propertyGroups[groupID];
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

    std::map<unsigned int, Property>& getProperties(unsigned int groupID = 0) {
        static std::map<unsigned int, Property> empty;
        if (groupID < propertyGroups.size()) return propertyGroups[groupID].properties;
        return empty;
    }
    const std::map<unsigned int, Property>& getProperties(unsigned int groupID = 0) const {
        static const std::map<unsigned int, Property> empty;
        if (groupID < propertyGroups.size()) return propertyGroups[groupID].properties;
        return empty;
    }
    std::vector<PropertyGroup>& getPropertyGroups() { return propertyGroups; }
    const std::vector<PropertyGroup>& getPropertyGroups() const { return propertyGroups; }

    /*template<typename T>
    T* getPropertyValue(const std::string& name, unsigned int groupID = 0) {
        void* ptr = getPropertyVoidPtr(name, groupID);
        if (ptr) return static_cast<T*>(ptr);
        else return nullptr;
    }

    void* getPropertyVoidPtr(const std::string& name, unsigned int groupID = 0) {
        for (auto& [propIdx, prop] : getProperties(groupID)) {
            if (prop.name == name)
                return prop.ptr;
        }
        return nullptr;
    }*/

protected:
    std::vector<PropertyGroup> propertyGroups;
};