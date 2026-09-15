#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

#include <scene/Entity.h>
#include <scene/components/Component.h>

class Archive;

// For registering ONLY built-in components
#define REGISTER(Type) \
    inline bool _autoReg_##Type = []() { \
        ComponentTypeInfo<Type>::name = #Type; \
        unsigned int UID = componentTypeUID<Type>(); \
        __componentTypeUIDToString(UID, true, #Type); \
        ComponentFactory::registerBuiltIn(#Type, [](Entity& e, const Archive& j) { \
            auto& c = e.addComponentDeferred<Type>(); \
            c.deserialize(j); \
            c.onAttach(); \
        }); \
        return true; \
    }()

class ComponentFactory {
public:
    using Creator = std::function<void(Entity&, const Archive&)>;

    static void registerBuiltIn(const std::string& name, Creator creator);
    static void registerScript(const std::string& name, Creator creator);
    static void create(const std::string& name, Entity& entity, const Archive& arch);
    static void clearScriptRegistry() { getScriptRegistry().clear(); }

    static std::vector<std::string> getBuiltInNames();
    static std::vector<std::string> getScriptNames();

private:
    static std::unordered_map<std::string, Creator>& getScriptRegistry() {
        static std::unordered_map<std::string, Creator> scriptRegistry;
        return scriptRegistry;
    }

    static std::unordered_map<std::string, Creator>& getBuiltInRegistry() { 
        static std::unordered_map<std::string, Creator> builtInRegistry;
        return builtInRegistry;
    }
};