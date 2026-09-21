#pragma once

#include <scene/components/ComponentTypeUID.h>

#include <memory>
#include <string>
#include <optional>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <cstdint>

namespace ECS {
    struct EntityHandle;
}

class Scene;
class Component;
class BehaviourComponent;
class TransformComponent;

class Entity {
    friend class Scene;

public:
    // invalid entity is default
    Entity() = default;

    Entity(Scene* owningScene, const ECS::EntityHandle& handle);
    ECS::EntityHandle getHandle() const;

    uint32_t getID() const { return entityId; }

    std::string getName() const;
    void setName(const std::string& newName) const;

    Scene& getScene() const { return *scene; }

    bool isValid() const;

    // valid AND still present in its scene
    bool isAlive() const;

    bool operator==(const Entity& other) const {
        return scene == other.scene && entityId == other.entityId && generation == other.generation;
    }
    bool operator!=(const Entity& other) const { return !(*this == other); }

    TransformComponent getTransform() const;

    template<typename T, typename... Args>
    decltype(auto) addComponent(Args&&... args) {
        return addComponentImpl<T, true>(std::forward<Args>(args)...);
    }

    // same but T::onAttach() is not called
    template<typename T, typename... Args>
    decltype(auto) addComponentDeferred(Args&&... args) {
        return addComponentImpl<T, false>(std::forward<Args>(args)...);
    }

    template<typename T>
    void removeComponent() {
        static_assert(!std::is_same_v<T, TransformComponent>,
            "TransformComponent is built-in, every entity keeps its transform");

        const unsigned int UID = componentTypeUID<T>();

        if constexpr (std::is_base_of_v<BehaviourComponent, T>) {
            if (BehaviourComponent* behaviour = getBehaviourRaw(UID)) {
                behaviour->onDetach();
                removeComponentRaw(UID);
            }
        }
        else {
            if (!hasComponent(UID)) return;

            T proxy(*this);
            proxy.onDetach();
            removeComponentRaw(UID);
        }
    }

    // type-erased removal by component UID
    // primarily used in the editor because it doesn't know engine types
    void removeComponent(unsigned int UID);

    // built-ins: std::optional<T>, empty if the entity has no T
    // user scripts: T*, nullptr if the entity has no T
    template<typename T>
    decltype(auto) getComponent() const {
        const unsigned int UID = componentTypeUID<T>();

        if constexpr (std::is_base_of_v<BehaviourComponent, T>) {
            return static_cast<T*>(getBehaviourRaw(UID));
        }
        else {
            if (!hasComponent(UID)) return std::optional<T>();

            return std::optional<T>(std::in_place, *this);
        }
    }

    template<typename T>
    bool hasComponent() const {
        return hasComponent(componentTypeUID<T>());
    }

    bool hasComponent(unsigned int UID) const;

    // nullptr if there is no script with matching UID (or UID is built-in)
    // for generic code that only knows the runtime UID  (the inspector)
    BehaviourComponent* getBehaviour(unsigned int UID) const;

private:
    template<typename T, bool Attach, typename... Args>
    decltype(auto) addComponentImpl(Args&&... args) {
        static_assert(!std::is_same_v<T, TransformComponent>,
            "TransformComponent is built-in, use getTransform() instead");

        const unsigned int UID = componentTypeUID<T>();

        if constexpr (std::is_base_of_v<BehaviourComponent, T>) {
            if (hasComponent(UID))
                throw std::runtime_error("Entity already has this component type");

            std::unique_ptr<T> behaviour = std::make_unique<T>(std::forward<Args>(args)...);
            T* rawPtr = behaviour.get();

            addBehaviourRaw(UID, std::move(behaviour));

            if constexpr (Attach) {
                if (!rawPtr->onAttach()) {
                    removeComponentRaw(UID);
                    throw std::runtime_error("Component rejected attachment");
                }
            }

            return *rawPtr;
        }
        else {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            addBuiltInRaw(UID);

            T proxy(*this);
            if constexpr (sizeof...(Args) > 0)
                proxy.init(std::forward<Args>(args)...);

            if constexpr (Attach) {
                if (!proxy.onAttach()) {
                    removeComponentRaw(UID);
                    throw std::runtime_error("Component rejected attachment");
                }
            }

            return proxy;
        }
    }

    // methods with "Raw" at the end: Entry point into ECS
    /*
    */
    void addBuiltInRaw(unsigned int UID) const;
    void removeComponentRaw(unsigned int UID) const;
    BehaviourComponent* addBehaviourRaw(unsigned int UID, std::unique_ptr<BehaviourComponent> behaviour) const;
    BehaviourComponent* getBehaviourRaw(unsigned int UID) const;

    Scene* scene = nullptr;
    uint32_t entityId = UINT32_MAX;
    uint32_t generation = UINT32_MAX;
};
