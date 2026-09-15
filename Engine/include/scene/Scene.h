#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <set>

class Entity;
class CameraComponent;
class PhysicsWorld;

class Scene {
    friend class Application;
public:
    ~Scene();

    static void createDefaultScene(Scene& scene);

    void renameScene(const std::string& name) { sceneName = name; }
    void newScene(const std::string& sceneName = "scene");
    void openScene(const std::string& sceneName);

    std::string getSceneName() const { return sceneName; }
    void setSceneName(const std::string& name) { sceneName = name; }

    Entity& createEntity(const std::string& name, Entity* parent = nullptr);
    Entity& createEntityImmediate(const std::string& name, Entity* parent = nullptr);
    void removeEntity(Entity* entity);
    void removeEntity(const std::string& name);
    void removeEntity(unsigned int ID);
    Entity* findEntity(unsigned int ID);
    Entity* findEntity(const std::string& name);

    CameraComponent* getActiveCamera() const { return activeCamera; }
    void setActiveCamera(CameraComponent* camera);
    void onCameraAdded(CameraComponent* camera);

    void onUpdate(float dt);
    void clear();

    const std::map<unsigned int, std::unique_ptr<Entity>>& getEntities() const { return entities; }
    const std::set<unsigned int>& getEntityIDs() const { return entityIDs; }

    PhysicsWorld& getPhysicsWorld();

    std::function<void(Entity*)> onEntityRemoved;
    bool isPlaying = false;
private:
    Scene(const std::string& sceneName = "scene");

    void addNewEntities();
    void removeDeadEntities();

    float physicsAccumulator = 0.0f;
    float colliderCacheTimer = 0.0f;

    unsigned int nextEntityID = 0;

    std::unique_ptr<PhysicsWorld> physicsWorld;

    std::string sceneName;

    std::map<unsigned int, std::unique_ptr<Entity>> entities;
    std::set<unsigned int> entityIDs;
    std::vector<std::unique_ptr<Entity>> entitiesToAdd;
    std::vector<Entity*> entitiesToRemove;
    CameraComponent* activeCamera = nullptr;
};
