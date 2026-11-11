#include <scene/components/ComponentFactory.h>

#include "ComponentFactory.inl"

#include <persistance/Archive.h>
#include <scene/Entity.h>

#include <unordered_map>
#include <cstdio>
#include <string>

bool ComponentFactory::m_ScriptCacheValid = false;

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

void ComponentFactory::registerScript(const std::string& name, Creator creator, ComponentOps ops, ScriptTypeInfo typeInfo) {
    ComponentFactory::getScriptRegistry()[name] = { std::move(creator), ops, std::move(typeInfo) };
    m_ScriptCacheValid = false;
}

// out of line so the registry is always Engine.dll's single copy
void ComponentFactory::clearScriptRegistry() {
    getScriptRegistry().clear();
    m_ScriptCacheValid = false;
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
            printf("[Error] Unknown component type: %s\n", name.c_str());
    }
}

void ComponentFactory::fillExistingComponent(Entity& entity, const Archive& arch) {
    std::string name;
    arch.get("type", name);

    auto& builtInRegistry = getBuiltInRegistry();
    auto it = builtInRegistry.find(name);
    if (it != builtInRegistry.end())
        it->second.ops.deserialize(entity, arch);
    else {
        auto& scriptRegistry = getScriptRegistry();
        auto it = scriptRegistry.find(name);
        if (it != scriptRegistry.end())
            it->second.ops.deserialize(entity, arch);
        else
            printf("[Error] Couldn't paste unknown component type: %s\n", name.c_str());
    }
}

void ComponentFactory::remove(unsigned int UID, Entity& entity) {
    const BuiltInEntry* entry = findEntry(UID, getBuiltInRegistry());
    if (entry) { entry->ops.remove(entity); return; }

    const ScriptEntry* sentry = findEntry(UID, getScriptRegistry());
    if (sentry) sentry->ops.remove(entity);
}

void ComponentFactory::serializeEntity(const Entity& entity, Archive& arch) {
    serializeComponents(entity, arch, getBuiltInRegistry());
    serializeComponents(entity, arch, getScriptRegistry());
}

void ComponentFactory::serializeComponent(const Entity& entity, const std::string& componentName, Archive& arch) {
    if (!trySerializeComponent(entity, componentName, arch, getBuiltInRegistry())) // try to find component in built-in registry first
        trySerializeComponent(entity, componentName, arch, getScriptRegistry()); // then try the srcipts
}

std::vector<unsigned int> ComponentFactory::getEntityComponentUIDs(const Entity& entity) {
    std::vector<unsigned int> UIDs;

    collectGroup(entity, UIDs, getBuiltInRegistry());
    collectGroup(entity, UIDs, getScriptRegistry());

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

    if (m_ScriptCacheValid) return names;

    names.clear();
    for (const auto& [name, entry] : getScriptRegistry())
        names.push_back(name);

    m_ScriptCacheValid = true;
    return names;
}

const ScriptTypeInfo* ComponentFactory::getScriptTypeInfo(unsigned int UID) {
    const std::string& name = __componentTypeUIDToString(UID);

    auto& registry = getScriptRegistry();
    auto it = registry.find(name);
    if (it == registry.end()) return nullptr;

    return &it->second.typeInfo;
}
