#include <scene/components/ComponentFactory.h>

#include <persistance/Archive.h>
#include <scene/Entity.h>

#include <unordered_map>
#include <cstdio>

bool ComponentFactory::scriptCacheValid = false;

// single definition (this TU is compiled only into Engine.dll), so the UID->name map
// is shared by every module that calls it: REGISTER/END_SCRIPT write it, the UI reads it.
std::string __componentTypeUIDToString(unsigned int UID, bool set, const std::string& name) {
    static std::unordered_map<unsigned int, std::string> nameMap;
    if (set) {
        nameMap[UID] = name;
        return "set";
    }
    auto nit = nameMap.find(UID);
    return nit != nameMap.end() ? nit->second : std::string();
}

void ComponentFactory::registerBuiltIn(const std::string& name, Creator creator, ComponentOps ops) {
    ComponentFactory::getBuiltInRegistry()[name] = { std::move(creator), ops };
}

void ComponentFactory::registerScript(const std::string& name, Creator creator, ComponentOps ops) {
    ComponentFactory::getScriptRegistry()[name] = { std::move(creator), ops };
    scriptCacheValid = false;
}

// out of line so the registry is always Engine.dll's single copy
void ComponentFactory::clearScriptRegistry() {
    getScriptRegistry().clear();
    scriptCacheValid = false;
}

void ComponentFactory::create(const std::string& name, Entity& entity, const Archive& arch) {
    auto& builtInRegistry = getBuiltInRegistry();
    auto it = builtInRegistry.find(name);
    if (it != builtInRegistry.end())
        it->second.creator(entity, arch);
    else {
        auto& scriptRegistry = getScriptRegistry();
        auto it = scriptRegistry.find(name);
        if (it != scriptRegistry.end())
            it->second.creator(entity, arch);
        else
            printf("Unknown component type: %s\n", name.c_str());
    }
}

const ComponentFactory::Entry* ComponentFactory::findEntry(unsigned int UID) {
    std::string name = __componentTypeUIDToString(UID);
    if (name.empty()) return nullptr;

    auto& builtInRegistry = getBuiltInRegistry();
    auto bit = builtInRegistry.find(name);
    if (bit != builtInRegistry.end()) return &bit->second;

    auto& scriptRegistry = getScriptRegistry();
    auto sit = scriptRegistry.find(name);
    if (sit != scriptRegistry.end()) return &sit->second;

    return nullptr;
}

void ComponentFactory::remove(unsigned int UID, Entity& entity) {
    const Entry* entry = findEntry(UID);
    if (!entry) return;

    entry->ops.remove(entity);
}

void ComponentFactory::serializeEntity(const Entity& entity, Archive& arch) {
    auto serializeGroup = [&](const std::map<std::string, Entry>& registry) {
        for (const auto& [name, entry] : registry) {
            Archive cj;
            if (!entry.ops.serialize(entity, cj)) continue;

            cj.set("type", name);
            arch.append("components", std::move(cj));
        }
    };

    serializeGroup(getBuiltInRegistry());
    serializeGroup(getScriptRegistry());
}

std::vector<unsigned int> ComponentFactory::getEntityComponentUIDs(const Entity& entity) {
    std::vector<unsigned int> UIDs;

    auto collectGroup = [&](const std::map<std::string, Entry>& registry) {
        for (const auto& [name, entry] : registry) {
            if (entry.ops.has(entity))
                UIDs.push_back(componentNameToUID(name));
        }
    };

    collectGroup(getBuiltInRegistry());
    collectGroup(getScriptRegistry());

    return UIDs;
}

const std::vector<std::string>& ComponentFactory::getBuiltInNames() {
    static std::vector<std::string> names; // no need to rebuild every time as these won't chage at runtime

    if (!names.empty()) return names;

    for (const auto& [name, entry] : getBuiltInRegistry())
        names.push_back(name);
    return names;
}

const std::vector<std::string>& ComponentFactory::getScriptNames() {
    static std::vector<std::string> names;

    if (scriptCacheValid) return names;

    names.clear();
    for (const auto& [name, entry] : getScriptRegistry())
        names.push_back(name);

    scriptCacheValid = true;
    return names;
}
