#pragma once

#include <unordered_map>
#include <type_traits>
#include <functional>
#include <string>
#include <vector>

#include <scene/Entity.h>
#include <scene/components/Component.h>
#include <scene/components/BehaviourComponent.h>

class Archive;

struct ScriptTypeInfo {
    size_t size;
    size_t alignment;

    void (*constructor)(void*);
    void (*destructor)(void*);
    void (*moveConstructor)(void*, void*) noexcept;
    BehaviourComponent* (*getBehaviour)(void*);
    const BehaviourComponent* (*getBehaviourConst)(const void*);
};

template<typename T>
ScriptTypeInfo makeTypeInfo() {

    static_assert(
        std::is_nothrow_move_constructible_v<T>,
        "Script types must have noexcept move constructors"
    );

    return {
        sizeof(T),
        alignof(T),

        [](void* p) {
            new (p) T();
        },

        [](void* p) {
            static_cast<T*>(p)->~T();
        },

        [](void* dst, void* src) noexcept {
            T* source = static_cast<T*>(src);
            new (dst) T(std::move(*source));
            source->~T();
        },

        [](void* p) -> BehaviourComponent* {
            return static_cast<BehaviourComponent*>(static_cast<T*>(p));
        },

        [](const void* p) -> const BehaviourComponent* {
            return static_cast<const BehaviourComponent*>(static_cast<const T*>(p));
        }
    };
}

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
// this self-registering lambda gets invoked before main() for every built-in component
// a similar mechanism exists for user script registration
#define REGISTER(Type) \
    namespace { \
        inline bool _autoReg_##Type = []() { \
            ComponentTypeInfo<Type>::name = #Type; \
            unsigned int UID = componentTypeUID<Type>(); \
            componentTypeUIDToStringRegistry(UID, true, #Type); \
            ComponentFactory::registerBuiltIn(#Type, [](Entity& e, const Archive& j) { \
                auto&& c = e.addComponentDeferred<Type>(); \
                c.deserialize(j); \
                c.onAttach(); \
            }, COMPONENT_OPS(Type)); \
            return true; \
        }(); \
    }

class ComponentFactory {
public:
    using Creator = std::function<void(Entity&, const Archive&)>;

    static void registerBuiltIn(const std::string& name, Creator creator, ComponentOps ops);
    static void registerScript(const std::string& name, Creator creator, ComponentOps ops, ScriptTypeInfo info);
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

    struct BuiltInEntry {
        Creator creator;
        ComponentOps ops;
    };

    struct ScriptEntry {
        Creator creator;
        ComponentOps ops;
        ScriptTypeInfo typeInfo;
    };

    static const ScriptTypeInfo* getScriptTypeInfo(unsigned int UID);
private:
    static bool m_ScriptCacheValid;

    static std::unordered_map<std::string, ScriptEntry>& getScriptRegistry() {
        static std::unordered_map<std::string, ScriptEntry> scriptRegistry;
        return scriptRegistry;
    }

    static std::unordered_map<std::string, BuiltInEntry>& getBuiltInRegistry() {
        static std::unordered_map<std::string, BuiltInEntry> builtInRegistry;
        return builtInRegistry;
    }
};
