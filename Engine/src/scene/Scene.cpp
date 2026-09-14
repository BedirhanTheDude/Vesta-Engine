#include <scene/Scene.h>

#include <core/Input.h>
#include <scene/Entity.h>
#include <scene/components/CameraComponent.h>
#include <scene/components/DirectionalLightComponent.h>
#include <scene/components/BehaviourComponent.h>
#include <scene/components/RigidBodyComponent.h>

#include <physics/PhysicsWorld.h>

#include <persistance/SceneSerializer.h>
#include <persistance/Archive.h>

#include <unordered_set>

void Scene::createDefaultScene(Scene& scene) {
	scene.clear();

	Entity& cameraEntity = scene.createEntityImmediate("Main Camera");
	cameraEntity.transform.setPosition(glm::vec3(0, 2, 6));
	auto& cam = cameraEntity.addComponent<CameraComponent>();
	scene.setActiveCamera(&cam);

	Entity& lightEntity = scene.createEntityImmediate("Directional Light");
	lightEntity.transform.setRotation(glm::vec3(-45.0f, -45.0f, 0.0f));
	auto& light = lightEntity.addComponent<DirectionalLightComponent>();
	light.color = glm::vec3(1.0f);
	light.ambientStrength = 0.2f;
	light.diffuseStrength = 0.8f;
	light.specularStrength = 0.5f;
}

Scene::Scene(const std::string& sceneName) : sceneName(sceneName) {
	physicsWorld = std::make_unique<PhysicsWorld>();
	physicsWorld->init();
}

Scene::~Scene() {
	clear();
	entities.clear();
	physicsWorld->shutdown();
	physicsWorld.release();
}

void Scene::newScene(const std::string& sceneName) {
	clear();
	this->sceneName = sceneName;

	createDefaultScene(*this);
}

void Scene::openScene(const std::string& sceneName) {
	clear();
	this->sceneName = sceneName;

	SceneSerializer::load(*this);
}

Entity& Scene::createEntity(const std::string& name, Entity* parent) {
	auto entity = std::unique_ptr<Entity>(new Entity(this, name, nextEntityID, parent));
	nextEntityID++;
	Entity& ref = *entity;

	entitiesToAdd.push_back(std::move(entity));
	return ref;
}

Entity& Scene::createEntityImmediate(const std::string& name, Entity* parent) {
	auto entity = std::unique_ptr<Entity>(new Entity(this, name, nextEntityID, parent));
	nextEntityID++;
	Entity& ref = *entity;
	entityIDs.insert(ref.getID());
	entities[ref.getID()] = std::move(entity);
	return ref;
}

void Scene::removeEntity(Entity* entity) {
	if (onEntityRemoved)
		onEntityRemoved(entity);
	entitiesToRemove.push_back(entity);

	for (auto child : entity->transform.getChildren()) {
		removeEntity(child->getEntity());
	}
}

void Scene::removeEntity(const std::string& name) {
	for (auto it = entities.begin(); it != entities.end(); it++) {
		Entity* entity = it->second.get();
		if (entity->name.compare(name) == 0) {
			if (onEntityRemoved)
				onEntityRemoved(entity);
			entitiesToRemove.push_back(entity);

			for (auto child : entity->transform.getChildren()) {
				removeEntity(child->getEntity());
			}

			return;
		}
	}
}

void Scene::removeEntity(unsigned int ID) {
	auto eit = entities.find(ID);
	if (eit != entities.end()) {
		Entity* entity = eit->second.get();
		if (onEntityRemoved)
			onEntityRemoved(entity);
		entitiesToRemove.push_back(entity);

		for (auto child : entity->transform.getChildren()) {
			removeEntity(child->getEntity());
		}
	}
}

Entity* Scene::findEntity(unsigned int ID) {
	auto eit = entities.find(ID);
	if (eit != entities.end()) {
		return eit->second.get();
	}

	return nullptr;
}

Entity* Scene::findEntity(const std::string& name) {
	for (auto& [ID, entity] : entities) {
		if (entity->getName() == name)
			return entity.get();
	}
	return nullptr;
}

void Scene::setActiveCamera(CameraComponent* camera) {
	if (camera) activeCamera = camera;
	else {
		for (auto& [ID, entity] : entities) {
			auto cam = entity->getComponent<CameraComponent>();
			if (cam) {
				activeCamera = cam;
				break;
			}
		}
	}
}

void Scene::onCameraAdded(CameraComponent* camera)
{
	if (activeCamera == nullptr)
		activeCamera = camera;
}

void Scene::onUpdate(float dt) {
	removeDeadEntities();
	addNewEntities();

	if (!isPlaying) return;

	if (dt <= 0.0f || dt > 0.1f)
		dt = 1.0f / 60.0f;

	const float fixedPhysicsStep = 1.0f / 60.0f;
	physicsAccumulator += dt;
	colliderCacheTimer += dt;

	if (colliderCacheTimer > 1.0f) {
		physicsWorld->cleanShapeCache();
		colliderCacheTimer -= 1.0f;
	}

	bool isPhysicsLoop = physicsAccumulator >= fixedPhysicsStep;

	if (isPhysicsLoop)
		for (auto& [ID, entity] : entities) {
			auto* rb = entity->getComponent<RigidBodyComponent>();
			if (rb) rb->pushToWorld();
		}

	while (physicsAccumulator >= fixedPhysicsStep) {
		physicsWorld->step(fixedPhysicsStep);
		physicsAccumulator -= fixedPhysicsStep;
	}

	if (isPhysicsLoop)
		for (auto& [ID, entity] : entities) {
			auto* rb = entity->getComponent<RigidBodyComponent>();
			if (rb) rb->pullFromWorld();
		}

	for (const auto& [ID, entity] : entities) {
		const auto& components = entity->getComponents();
		for (unsigned int behaviourIdx : entity->getBehaviourIndices())
			static_cast<BehaviourComponent*>(components.at(behaviourIdx).get())->tick(dt);
	}
}

PhysicsWorld& Scene::getPhysicsWorld() { return *physicsWorld; }

void Scene::addNewEntities() {
	for (auto& entity : entitiesToAdd) {
		entityIDs.insert(entity->getID());
		entities[entity->getID()] = std::move(entity);
	}
	entitiesToAdd.clear();
}

void Scene::removeDeadEntities() {
	for (auto* dead : entitiesToRemove) {
		auto it = entities.find(dead->getID());
		if (it != entities.end()) {
			entityIDs.erase(it->second->getID());
			entities.erase(it->second->getID());
		}
	}
	entitiesToRemove.clear();
}

void Scene::clear() {
	entities.clear();
	entitiesToAdd.clear();
	entitiesToRemove.clear();
	activeCamera = nullptr;

	nextEntityID = 0;
}