#include <scene/components/ComponentFactory.h>

#include <persistance/Archive.h>
#include <scene/Entity.h>

#include <unordered_map>

using Creator = std::function<void(Entity&, const Archive&)>;

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

void ComponentFactory::registerBuiltIn(const std::string& name, Creator creator) {
    ComponentFactory::getBuiltInRegistry()[name] = creator;
}

void ComponentFactory::registerScript(const std::string& name, Creator creator) {
    ComponentFactory::getScriptRegistry()[name] = creator;
}

void ComponentFactory::create(const std::string& name, Entity& entity, const Archive& arch) {
    auto& builtInRegistry = getBuiltInRegistry();
    auto it = builtInRegistry.find(name);
    if (it != builtInRegistry.end())
        it->second(entity, arch);
    else {
        auto& scriptRegistry = getScriptRegistry();
        auto it = scriptRegistry.find(name);
        if (it != scriptRegistry.end())
            it->second(entity, arch);
        else
            printf("Unknown component type: %s\n", name.c_str());
    }
}

std::vector<std::string> ComponentFactory::getBuiltInNames() {
    std::vector<std::string> names;
    for (const auto& [name, creator] : getBuiltInRegistry())
        names.push_back(name);
    return names;
}

std::vector<std::string> ComponentFactory::getScriptNames() {
    std::vector<std::string> names;
    for (const auto& [name, creator] : getScriptRegistry())
        names.push_back(name);
    return names;
}
