#pragma once

#include <persistance/Serializable.h>

#include <scene/components/Component.h>
#include <scene/components/BehaviourComponent.h>
#include <scene/components/TransformComponent.h>

#include <memory>
#include <string>
#include <map>
#include <unordered_set>
#include <type_traits>
#include <stdexcept>

class Scene;

class Entity : Serializable {
    friend class Scene;

public:
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    ~Entity() = default;

    int getID() const { return ID; }

    void serialize(Archive& arch) const override;
    void deserialize(const Archive& arch) override {}

    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }

    Scene& getScene() const { return *scene; }

    TransformComponent& getTransform();
    const TransformComponent& getTransform() const;

    template<typename T, typename... Args>
    T& addComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        static_assert(!std::is_same_v<T, TransformComponent>,
            "TransformComponent is built-in, use getTransform() instead");

        if(hasComponent<T>())
            throw std::runtime_error("Entity already has this component type");

        unsigned int UID = componentTypeUID<T>();

        std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args>(args)...);
        component->owner = this;
        component->setTypeUID(UID);

        unsigned int idx = nextCompIdx++;

        T* rawPtr = component.get();
        static_cast<Component*>(rawPtr)->setIdx(idx);

        componentIdxMap[UID] = idx;
        components[idx] = std::move(component);
        if (std::is_base_of_v<BehaviourComponent, T>)
            behaviourIndices.insert(idx);

        if (!rawPtr->onAttach()) {
            components.erase(idx);
            componentIdxMap.erase(UID);
            if (std::is_base_of_v<BehaviourComponent, T>)
                behaviourIndices.erase(idx);
            throw std::runtime_error("Component rejected attachment");
        }

        return *rawPtr;
    }

    template<typename T, typename... Args>
    T& addComponentDeferred(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        static_assert(!std::is_same_v<T, TransformComponent>,
            "TransformComponent is built-in, use getTransform() instead");

        if (hasComponent<T>())
            throw std::runtime_error("Entity already has this component type");
        
        unsigned int UID = componentTypeUID<T>();

        std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args>(args)...);
        component->owner = this;
        component->setTypeUID(UID);

        unsigned int idx = nextCompIdx++;

        T* rawPtr = component.get();
        static_cast<Component*>(rawPtr)->setIdx(idx);

        componentIdxMap[UID] = idx;
        components[idx] = std::move(component);
        if (std::is_base_of_v<BehaviourComponent, T>)
            behaviourIndices.insert(idx);

        return *rawPtr;
    }

    template<typename T>
    void removeComponent() {
        unsigned int UID = componentTypeUID<T>();
        auto cit = componentIdxMap.find(UID);
        if (cit != componentIdxMap.end()) {
            unsigned int idx = cit->second;
            components[idx]->onDetach();
            components.erase(idx);
            componentIdxMap.erase(cit);
            if (std::is_base_of_v<BehaviourComponent, T>)
                behaviourIndices.erase(idx);
        }
    }

    // type-erased removal by component UID; the editor inspector only knows a
    // component's runtime UID, not its static type.
    void removeComponent(unsigned int UID) {
        auto cit = componentIdxMap.find(UID);
        if (cit != componentIdxMap.end()) {
            unsigned int idx = cit->second;
            components[idx]->onDetach();
            components.erase(idx);
            componentIdxMap.erase(cit);
            behaviourIndices.erase(idx); // no-op when idx isn't a behaviour
        }
    }

    template<typename T>
    T* getComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        unsigned int UID = componentTypeUID<T>();
        auto cit = componentIdxMap.find(UID);
        if (cit != componentIdxMap.end())
            return static_cast<T*>(components[cit->second].get());
        else return nullptr;
    }

    template<typename T>
    const T* getComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        unsigned int UID = componentTypeUID<T>();
        auto cit = componentIdxMap.find(UID);
        if (cit != componentIdxMap.end())
            return static_cast<T*>(components.at(cit->second).get());
        else return nullptr;
    }

    template<typename T>
    bool hasComponent() const {
        unsigned int UID = componentTypeUID<T>();
        auto cit = componentIdxMap.find(UID);
        if (cit != componentIdxMap.end()) return true;
        else return false;
    }

    Component* getComponent(unsigned int UID) {
        auto cit = componentIdxMap.find(UID);
        if (cit != componentIdxMap.end())
            return components[cit->second].get();
        else 
            return nullptr;
    }

    const Component* getComponent(unsigned int UID) const {
        auto cit = componentIdxMap.find(UID);
        if (cit != componentIdxMap.end())
            return components.at(cit->second).get();
        else
            return nullptr;
    }

    const std::map<unsigned int, std::unique_ptr<Component>>& getComponents() const { return components; }
    const std::map<unsigned int, unsigned int>& getComponentIdxMap() { return componentIdxMap; }
    const std::unordered_set<unsigned int>& getBehaviourIndices() { return behaviourIndices; }

    TransformComponent transform;

private:
    Entity(Scene* owningScene, const std::string& entityName, unsigned int ID, Entity* parent = nullptr);
    Entity(Scene* owningScene, const std::string& entityName, unsigned int ID,
        const glm::vec3& pos, const glm::vec3& rot,
        const glm::vec3& scale, Entity* parent = nullptr);

private:

    void setID(int newID) { ID = newID; }
    int ID;

    unsigned int nextCompIdx = 0;

    Scene* scene = nullptr;
    std::string name;

    std::map<unsigned int, std::unique_ptr<Component>> components;
    std::map<unsigned int, unsigned int> componentIdxMap; // [componentTypeUID, componentIdx]
    std::unordered_set<unsigned int> behaviourIndices; // indices for components map for behaviour components
    // component index is based on the order the component got added
};