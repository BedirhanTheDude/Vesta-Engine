#pragma once

#include <scene/components/ComponentFactory.h>

#include <persistence/Archive.h>
#include <scene/Entity.h>

#include <type_traits>
#include <unordered_map>

template<typename Entry>
inline bool trySerializeComponent(const Entity& entity, const std::string& componentName,
    Archive& arch, const std::unordered_map<std::string, Entry>& registry) {

    static_assert(std::is_same<Entry, ComponentFactory::BuiltInEntry>::value || std::is_same<Entry, ComponentFactory::ScriptEntry>::value,
        "Entry must be BuiltInEntry or ScriptEntry");

    for (const auto& [name, entry] : registry) {
        if (name != componentName) continue;

        if (entry.ops.serialize(entity, arch)) {
            arch.set("type", name);
            return true;
        }
    }

    return false;
}

template<typename Entry>
inline void serializeComponents(const Entity& entity, Archive& arch,
    const std::unordered_map<std::string, Entry>& registry) {

    static_assert(std::is_same<Entry, ComponentFactory::BuiltInEntry>::value || std::is_same<Entry, ComponentFactory::ScriptEntry>::value,
        "Entry must be BuiltInEntry or ScriptEntry");

    for (const auto& [name, entry] : registry) {
        Archive cj;
        if (!entry.ops.serialize(entity, cj)) continue;

        cj.set("type", name);
        arch.append("components", std::move(cj));
    }
}

template<typename Entry>
inline void collectGroup(const Entity& entity, std::vector<unsigned int>& UIDs,
    const std::unordered_map<std::string, Entry>& registry) {

    static_assert(std::is_same<Entry, ComponentFactory::BuiltInEntry>::value || std::is_same<Entry, ComponentFactory::ScriptEntry>::value,
        "Entry must be BuiltInEntry or ScriptEntry");

    for (const auto& [name, entry] : registry) {
        if (entry.ops.has(entity))
            UIDs.push_back(componentNameToUID(name));
    }
}

template<typename Entry>
const Entry* findEntry(unsigned int UID, const std::unordered_map<std::string, Entry>& registry) {

    static_assert(std::is_same<Entry, ComponentFactory::BuiltInEntry>::value || std::is_same<Entry, ComponentFactory::ScriptEntry>::value,
        "Entry must be BuiltInEntry or ScriptEntry");

    std::string name = componentTypeUIDToStringRegistry(UID);
    if (name.empty()) return nullptr;

    auto sit = registry.find(name);
    if (sit != registry.end()) return &sit->second;

    return nullptr;
}