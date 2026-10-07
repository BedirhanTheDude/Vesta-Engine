#pragma once

#include <map>
#include <functional>
#include <string>
#include <vector>

#include <scene/Entity.h>
#include <scene/components/Component.h>

class Archive;

// type agnostic operations every component needs
struct ComponentOps {
    bool (*has)(const Entity&);
    void (*remove)(Entity&);
    bool (*serialize)(const Entity&, Archive&); // false if the entity has no such component
    void (*deserialize)(Entity&, const Archive&);
};

// gotta make my own syntactic sugar
#define COMPONENT_OPS(Type) ComponentOps{ \
    [](const Entity& e) { return e.hasComponent<Type>(); }, \
    [](Entity& e) { e.removeComponent<Type>(); }, \
    [](const Entity& e, Archive& a) { \
        auto c = e.getComponent<Type>(); \
        if (!c) return false; \
        c->serialize(a); \
        return true; }, \
    [](Entity& e, const Archive& a) { \
        auto c = e.getComponent<Type>(); \
        if (!c) return; \
        c->deserialize(a); \
        } \
    }

// for registering ONLY built-in components
// this self-registering lambda gets invoked before main(), how cool is that?
#define REGISTER(Type) \
    inline bool _autoReg_##Type = []() { \
        ComponentTypeInfo<Type>::name = #Type; \
        unsigned int UID = componentTypeUID<Type>(); \
        __componentTypeUIDToString(UID, true, #Type); \
        ComponentFactory::registerBuiltIn(#Type, [](Entity& e, const Archive& j) { \
            auto&& c = e.addComponentDeferred<Type>(); \
            c.deserialize(j); \
            c.onAttach(); \
        }, COMPONENT_OPS(Type)); \
        return true; \
    }()

class ComponentFactory {
public:
    using Creator = std::function<void(Entity&, const Archive&)>;

    static void registerBuiltIn(const std::string& name, Creator creator, ComponentOps ops);
    static void registerScript(const std::string& name, Creator creator, ComponentOps ops);
    static void create(const std::string& name, Entity& entity, const Archive& arch);
    static void fillExistingComponent(Entity& entity, const Archive& arch);
    static void clearScriptRegistry();

    // removes the component with this UID (onDetach included), does nothing if the entity has none
    static void remove(unsigned int UID, Entity& entity);

    // appends every component the entity has to arch's "components" array as { type, ...fields }
    // built-ins first, then scripts, each group in name order so the load order is deterministic
    static void serializeEntity(const Entity& entity, Archive& arch);
    static void serializeComponent(const Entity& entity, const std::string& componentName, Archive& arch);

    // UIDs of the components the entity currently has
    // mainly for the editor to iterate over, no other real use for this
    static std::vector<unsigned int> getEntityComponentUIDs(const Entity& entity);

    static const std::vector<std::string>& getBuiltInNames();
    static const std::vector<std::string>& getScriptNames();

private:
    static bool scriptCacheValid;

    struct Entry {
        Creator creator;
        ComponentOps ops;
    };

    static std::map<std::string, Entry>& getScriptRegistry() {
        static std::map<std::string, Entry> scriptRegistry;
        return scriptRegistry;
    }

    static std::map<std::string, Entry>& getBuiltInRegistry() {
        static std::map<std::string, Entry> builtInRegistry;
        return builtInRegistry;
    }

    static const Entry* findEntry(unsigned int UID);
};
