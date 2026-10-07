#pragma once

#include "Entity.h"

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace ECS {
    struct EntityHandle;

    class ComponentPoolRegistry;
    class EntityManager;
}

class Archive;
class PhysicsWorld;
class BehaviourComponent;

class Scene {
    friend class Application;
    friend class Entity;
    friend class Project;
public:
    ~Scene();

    static void createDefaultScene(Scene& scene);

    void renameScene(const std::string& name) { sceneName = name; }
    void newScene(const std::string& sceneName = "scene");
    bool openScene(const std::string& relativeScenePath);

    std::string getSceneName() const { return sceneName; }
    void setSceneName(const std::string& name) { sceneName = name; }

    const std::string& getScenePath() const { return scenePath; }
    void setScenePath(const std::string& relativeScenePath) { scenePath = relativeScenePath; }

    Entity createEntity(const std::string& name);
    Entity createEntity(const std::string& name, const Entity& parent);
    Entity createEntityImmediate(const std::string& name);
    Entity createEntityImmediate(const std::string& name, const Entity& parent);
    void removeEntity(const Entity& entity);
    void removeEntity(const std::string& name);
    void removeEntity(unsigned int ID);
    void renameEntity(unsigned int ID, const std::string& name);
    bool entityExists(const Entity& entity) const;
    bool entityExists(unsigned int ID) const;
    bool entityExists(const std::string& name) const;

    // First flushed entity with this name, the invalid Entity (isValid() == false) if there is none
    Entity findEntity(const std::string& name) const;
    Entity findEntity(unsigned int ID) const;

    Entity getActiveCameraEntity() const;
    void setActiveCamera(const Entity& camera);
    void onCameraAdded(const Entity& camera);

    void onUpdate(float dt);
    void clear();

    const std::vector<Entity>& getEntities() const;

    PhysicsWorld& getPhysicsWorld();

    ECS::ComponentPoolRegistry* getComponentPoolRegistry() const; // evil component pool registry type leakage >:D

    bool isPlaying = false;
private:
    Scene(const std::string& sceneName = "scene");

    void addNewEntities();
    void removeDeadEntities();

    void swapAndPopEntity(const ECS::EntityHandle& handle);
    void removeEntity(const ECS::EntityHandle& handle);
    void renameEntity(const ECS::EntityHandle& handle, const std::string& name);
    bool entityExists(const ECS::EntityHandle& handle) const;
    std::string getEntityName(const ECS::EntityHandle& handle) const;

    bool copyTransform(uint32_t entityId, Archive& outArch) const;
    bool copyComponent(uint32_t entityId, uint32_t componentUID, Archive& outArch) const;
    bool copyEntity(uint32_t entityId, Archive& outArch) const;

    void pasteTransform(uint32_t entityId, const Archive& transformArchive);
    void pasteComponent(uint32_t entityId, const Archive& componentArchive);
    void pasteEntity(const Archive& entityArcive);

    void ensureSparseSize(const ECS::EntityHandle& handle);

    bool tryGetEntityIndex(const ECS::EntityHandle& handle, uint32_t& outIndex) const;

    // parents a freshly created entity, keeping its (identity) world transform like setParent does
    void parentNewEntity(const Entity& child, const Entity& parent);

    // Backends of Entity's generic component API
    void addBuiltInComponent(const ECS::EntityHandle& handle, unsigned int UID);
    bool hasComponent(const ECS::EntityHandle& handle, unsigned int UID) const;
    void removeComponent(const ECS::EntityHandle& handle, unsigned int UID);
    BehaviourComponent* addBehaviour(const ECS::EntityHandle& handle, unsigned int UID, std::unique_ptr<BehaviourComponent> behaviour);
    BehaviourComponent* getBehaviour(const ECS::EntityHandle& handle, unsigned int UID) const;

    void tickBehaviours(float dt);
    void pushRigidBodies(); // transform -> physics world
    void pullRigidBodies(); // physics world -> transform

    float physicsAccumulator = 0.0f;
    float colliderCacheTimer = 0.0f;

    std::unique_ptr<PhysicsWorld> physicsWorld;

    std::string sceneName;
    std::string scenePath;

    // "no camera" is entityId == generation == UINT32_MAX, same sentinel values as
    // ECS::INVALID_ENTITY_HANDLE; stored as raw fields (not ECS::EntityHandle) so this
    // header doesn't need EntityHandle.h just to declare two integers.
    uint32_t activeCameraEntityId;
    uint32_t activeCameraGeneration;

    // Entity is only a handle, the state that used to live inside it lives beside it here:
    // entityNames[i] is the name of entities[i], both are moved by the same swap-and-pop
    std::vector<Entity> entities;
    std::vector<std::string> entityNames;

    // created but not flushed into `entities` yet, namesToAdd[i] belongs to entitiesToAdd[i]
    std::vector<Entity> entitiesToAdd;
    std::vector<std::string> namesToAdd;

    std::vector<Entity> entitiesToRemove;

    std::vector<uint32_t> sparseEntities; // Sparse index vector for entities, handle.entityId -> dense index in entities

    // Scratch of tickBehaviours, kept between frames so its capacity is reused instead of allocating every
    // frame. An entity handle as raw fields, same reason as activeCameraEntityId.
    struct BehaviourTickEntry {
        uint32_t UID;
        uint32_t entityId;
        uint32_t generation;
    };
    std::vector<BehaviourTickEntry> behaviourTickList;

    std::unique_ptr<ECS::EntityManager> entityManager;
    std::unique_ptr<ECS::ComponentPoolRegistry> componentRegistry;
};
