#include <scene/Scene.h>

#include <core/Input.h>
#include <core/Project.h>
#include <scene/Entity.h>
#include <scene/components/CameraComponent.h>
#include <scene/components/DirectionalLightComponent.h>
#include <scene/components/BehaviourComponent.h>
#include <scene/components/RigidBodyComponent.h>
#include <scene/components/TransformComponent.h>
#include <scene/components/ComponentFactory.h>

#include <scene/components/Transform.h>
#include <scene/components/MeshData.h>
#include <scene/components/RigidBody.h>

#include <scene/system/TransformSystem.h>
#include <scene/system/RigidBodySystem.h>

#include <physics/PhysicsWorld.h>

#include <persistance/SceneSerializer.h>
#include <persistance/Archive.h>

#include <ecs/EntityHandle.h>
#include <ecs/EntityManager.h>
#include <ecs/ComponentPool.h>
#include <ecs/ComponentPoolRegistry.h>

#include <utility>
#include <filesystem>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <mutex>

void Scene::createDefaultScene(Scene& scene) {
	scene.clear();

	Entity cameraEntity = scene.createEntityImmediate("Main Camera");
	cameraEntity.getTransform().setPosition(glm::vec3(0, 2, 6));
	cameraEntity.addComponent<CameraComponent>();
	scene.setActiveCamera(cameraEntity);

	Entity lightEntity = scene.createEntityImmediate("Directional Light");
	lightEntity.getTransform().setRotation(glm::vec3(-45.0f, -45.0f, 0.0f));
	auto light = lightEntity.addComponent<DirectionalLightComponent>();
	light.setColor(glm::vec3(1.0f));
	light.setAmbientStrength(0.2f);
	light.setDiffuseStrength(0.8f);
	light.setSpecularStrength(0.5f);
}

Scene::Scene(const std::string& sceneName) : sceneName(sceneName) {
	physicsWorld = std::make_unique<PhysicsWorld>();
	physicsWorld->init();

	entityManager = std::make_unique<ECS::EntityManager>();
	componentRegistry = std::make_unique<ECS::ComponentPoolRegistry>();

	activeCameraEntityId = ECS::INVALID_ENTITY_INDEX;
	activeCameraGeneration = ECS::INVALID_ENTITY_GENERATION;
}

Scene::~Scene() {
	clear();
	physicsWorld->shutdown();
}

void Scene::newScene(const std::string& sceneName) {
	clear();
	this->sceneName = sceneName;
	scenePath.clear();

	createDefaultScene(*this);
}

bool Scene::openScene(const std::string& relativeScenePath) {
	std::string previousName = sceneName;
	std::string previousPath = scenePath;

	scenePath = relativeScenePath;
	sceneName = std::filesystem::u8path(relativeScenePath).stem().string();

	if (SceneSerializer::load(*this)) return true;

	// load leaves the current scene untouched on failure, it keeps its own name and file
	sceneName = std::move(previousName);
	scenePath = std::move(previousPath);
	return false;
}

Entity Scene::createEntity(const std::string& name) {
	ECS::EntityHandle handle = entityManager->create();

	// every entity has a transform for its whole life, Entity::getTransform() relies on it
	componentRegistry->getTransforms().add(handle);

	entitiesToAdd.push_back(Entity(this, handle));
	namesToAdd.push_back(name);

	return entitiesToAdd.back();
}

Entity Scene::createEntity(const std::string& name, const Entity& parent) {
	Entity entity = createEntity(name);
	parentNewEntity(entity, parent);

	return entity;
}

Entity Scene::createEntityImmediate(const std::string& name) {
	ECS::EntityHandle handle = entityManager->create();

	componentRegistry->getTransforms().add(handle);

	ensureSparseSize(handle);

	entities.push_back(Entity(this, handle));
	entityNames.push_back(name);
	sparseEntities[handle] = static_cast<uint32_t>(entities.size() - 1);

	return entities.back();
}

Entity Scene::createEntityImmediate(const std::string& name, const Entity& parent) {
	Entity entity = createEntityImmediate(name);
	parentNewEntity(entity, parent);

	return entity;
}

void Scene::parentNewEntity(const Entity& child, const Entity& parent) {
	// a parent from another scene (or a dead/invalid one) would index the wrong pools, the child stays a root
	if (!parent.isValid() || parent.scene != this || !entityExists(parent.getHandle())) return;

	Transform* transform = componentRegistry->getTransforms().get(child.getHandle());
	if (!transform) return;

	TransformSystem::setParent(*transform, child.getHandle(), parent.getHandle(), this);
}

bool Scene::tryGetEntityIndex(const ECS::EntityHandle& handle, uint32_t& outIndex) const {
	if (handle.isInvalid() || handle >= sparseEntities.size())
		return false;

	uint32_t index = sparseEntities[handle.entityId];
	if (index == ECS::INVALID_ENTITY_INDEX || index >= entities.size())
		return false;

	if (!(entities[index].getHandle() == handle))
		return false;

	outIndex = index;
	return true;
}

// swap and pop to maintain dense entity vector with sparse index vector
void Scene::swapAndPopEntity(const ECS::EntityHandle& handle) {
	uint32_t denseIndex;
	if (!tryGetEntityIndex(handle, denseIndex)) return;

	// remove this from parent's children
	ECS::ComponentPool<Transform>& pool = componentRegistry->getTransforms();
	Transform* entityTransform = pool.get(handle);
	if (entityTransform) {
		Transform* parentTransform = pool.get(entityTransform->parent);
		if (parentTransform)
			TransformSystem::removeChild(*parentTransform, handle);
	}

	uint32_t last = static_cast<uint32_t>(entities.size() - 1);

	if (denseIndex != last) {
		Entity movedEntity = entities[last];
		entities[denseIndex] = movedEntity;
		entityNames[denseIndex] = std::move(entityNames[last]);

		sparseEntities[movedEntity.entityId] = denseIndex;
	}

	entities.pop_back();
	entityNames.pop_back();
	sparseEntities[handle] = ECS::INVALID_ENTITY_INDEX;
}

void Scene::removeEntity(const Entity& entity) {
	if (!entity.isValid() || entity.scene != this) return;

	removeEntity(entity.getHandle());
}

void Scene::removeEntity(const ECS::EntityHandle& handle) {
	if (!entityExists(handle)) return;

	// recursively kill children
	if (Transform* transform = componentRegistry->getTransforms().get(handle)) {
		for (const ECS::EntityHandle& childHandle : transform->children)
			removeEntity(childHandle);
	}

	entitiesToRemove.push_back(Entity(this, handle));
}

void Scene::removeEntity(const std::string& name) {
	for (size_t i = 0; i < entities.size(); i++) {
		if (entityNames[i] == name) {
			/*if (onEntityRemoved)
				onEntityRemoved(entity);*/
			removeEntity(entities[i].getHandle());
			return;
		}
	}
}

void Scene::removeEntity(unsigned int entityId) {
	uint32_t generation = entityManager->getGeneration(static_cast<uint32_t>(entityId));
	ECS::EntityHandle handle{ entityId, generation };

	uint32_t denseIndex;
	if (!tryGetEntityIndex(handle, denseIndex)) return; // alive in EntityManager but not yet flushed from entitiesToAdd, or genuinely gone

	removeEntity(entities[denseIndex].getHandle());
}

bool Scene::copyTransform(uint32_t entityId, Archive& outArch) const {
	uint32_t generation = entityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return false;

	outArch = Archive();

	Transform* transform = componentRegistry->getTransforms().get(handle);

	outArch.set("position", transform->position);
	outArch.set("rotation", transform->rotation);
	outArch.set("scale", transform->scale);
	return true; // If there is an entity there is a transform
}

bool Scene::copyComponent(uint32_t entityId, uint32_t componentUID, Archive& outArch) const {
	uint32_t generation = entityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return false;

	outArch = Archive();

	// Guaranteed that scene isn't modified through this entity instance
	Entity entity(const_cast<Scene*>(this), handle); // Flyweight entity
	std::string componentName = componentTypeUIDToString(componentUID);
	ComponentFactory::serializeComponent(entity, componentName, outArch);
	return outArch.has("type");
}

bool Scene::copyEntity(uint32_t entityId, Archive& outArch) const {
	uint32_t generation = entityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return false;

	outArch = Archive();

	Entity entity(const_cast<Scene*>(this), handle);

	outArch.set("name", entity.getName());

	TransformComponent transform = entity.getTransform();

	outArch.set("parent", std::string(""));

	// world space, the parent is re-applied (keeping the world transform) once every entity exists
	Archive transformArch;
	transformArch.set("position", transform.getWorldPosition());
	transformArch.set("rotation", transform.getWorldRotationQuat());
	transformArch.set("scale", transform.getWorldScale());
	outArch.set("transform", std::move(transformArch));

	ComponentFactory::serializeEntity(entity, outArch);
	return outArch.has("components");
}

void Scene::pasteTransform(uint32_t entityId, const Archive& transformArchive) {
	if (!transformArchive.has("position")) return;

	uint32_t generation = entityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return;

	Transform* transform = componentRegistry->getTransforms().get(handle);

	glm::vec3 pos(0.0f), scl(1.0f);
	glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
	transformArchive.get("position", pos);
	transformArchive.get("rotation", rot);
	transformArchive.get("scale", scl);

	// Has to go through this for cache invalidation
	TransformSystem::setPosition(*transform, pos, this);
	TransformSystem::setRotation(*transform, rot, this);
	TransformSystem::setScale(*transform, scl, this);
}

void Scene::pasteComponent(uint32_t entityId, const Archive& componentArchive) {
	std::string typeName;
	componentArchive.get("type", typeName);
	if (typeName.empty()) return;

	uint32_t generation = entityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return;

	Entity entity(const_cast<Scene*>(this), handle);

	uint32_t componentUID = componentNameToUID(typeName);
	auto* pool = componentRegistry->getPool(componentUID);
	if (!pool) // component wasn't built-in
		pool = componentRegistry->getBehaviourPool(componentUID);

	if (!pool) return;

	bool hasComponent = pool->has(handle);
	
	if (hasComponent)
		ComponentFactory::fillExistingComponent(entity, componentArchive);
	else
		ComponentFactory::create(typeName, entity, componentArchive);
}

void Scene::pasteEntity(const Archive& entityArchive) {
	std::string name;
	entityArchive.get("name", name);
	if (name.empty()) return;

	// has to acquire scene lock because this has to deserialize the entity on spot
	// so it calls createEntityImmediate
	std::mutex& sceneMutex = Project::getSceneMutex();
	std::lock_guard<std::mutex> lock(sceneMutex);

	Entity entity = createEntityImmediate(name);

	Archive transformArch = entityArchive.get("transform");
	glm::vec3 pos(0.0f), scl(1.0f);
	glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
	transformArch.get("position", pos);
	transformArch.get("rotation", rot);
	transformArch.get("scale", scl);

	Transform* transform = componentRegistry->getTransforms().get(entity.getHandle());
	TransformSystem::setPosition(*transform, pos, this);
	TransformSystem::setRotation(*transform, rot, this);
	TransformSystem::setScale(*transform, scl, this);

	size_t compCount = entityArchive.size("components");
	for (size_t c = 0; c < compCount; c++) {
		Archive cj = entityArchive.at("components", c);
		std::string type;
		cj.get("type", type);
		ComponentFactory::create(type, entity, cj);
	}
}

void Scene::renameEntity(const ECS::EntityHandle& handle, const std::string& name) {
	uint32_t denseIndex;
	if (tryGetEntityIndex(handle, denseIndex)) {
		entityNames[denseIndex] = name;
		return;
	}

	// created this frame, not flushed into `entities` yet
	for (size_t i = 0; i < entitiesToAdd.size(); i++) {
		if (entitiesToAdd[i].getHandle() == handle) {
			namesToAdd[i] = name;
			return;
		}
	}
}

void Scene::renameEntity(unsigned int ID, const std::string& name) {
	ECS::EntityHandle handle{ static_cast<uint32_t>(ID), entityManager->getGeneration(static_cast<uint32_t>(ID)) };
	renameEntity(handle, name);
}

std::string Scene::getEntityName(const ECS::EntityHandle& handle) const {
	uint32_t denseIndex;
	if (tryGetEntityIndex(handle, denseIndex))
		return entityNames[denseIndex];

	for (size_t i = 0; i < entitiesToAdd.size(); i++) {
		if (entitiesToAdd[i].getHandle() == handle)
			return namesToAdd[i];
	}

	return std::string();
}

bool Scene::entityExists(const ECS::EntityHandle& handle) const {
	return entityManager->isAlive(handle);
}

bool Scene::entityExists(const Entity& entity) const {
	return entity.isValid() && entity.scene == this && entityExists(entity.getHandle());
}

bool Scene::entityExists(unsigned int ID) const {
	ECS::EntityHandle handle{ static_cast<uint32_t>(ID), entityManager->getGeneration(static_cast<uint32_t>(ID)) };

	uint32_t denseIndex;
	return tryGetEntityIndex(handle, denseIndex);
}

bool Scene::entityExists(const std::string& name) const {
	for (const std::string& entityName : entityNames) {
		if (entityName == name) return true;
	}
	return false;
}

Entity Scene::findEntity(const std::string& name) const {
	for (size_t i = 0; i < entities.size(); i++) {
		if (entityNames[i] == name) return entities[i];
	}
	return Entity();
}

Entity Scene::findEntity(unsigned int ID) const {
	ECS::EntityHandle handle{ static_cast<uint32_t>(ID), entityManager->getGeneration(static_cast<uint32_t>(ID)) };

	uint32_t denseIndex;
	if (tryGetEntityIndex(handle, denseIndex))
		return entities[denseIndex];

	return Entity();
}

const std::vector<Entity>& Scene::getEntities() const {
	return entities;
}

void Scene::setActiveCamera(const Entity& camera) {
	ECS::EntityHandle handle = camera.getHandle();
	activeCameraEntityId = handle.entityId;
	activeCameraGeneration = handle.generation;
}

Entity Scene::getActiveCameraEntity() const {
	ECS::EntityHandle handle{ activeCameraEntityId, activeCameraGeneration };

	uint32_t denseIndex;
	if (tryGetEntityIndex(handle, denseIndex))
		return entities[denseIndex];

	return Entity();
}

void Scene::onCameraAdded(const Entity& camera)
{
	if (!camera.isAlive()) return;

	ECS::EntityHandle handle = camera.getHandle();
	activeCameraEntityId = handle.entityId;
	activeCameraGeneration = handle.generation;
}

void Scene::onUpdate(float dt) {
	// The Project has a longer lifetime than Scene so no need to get this every frame
	static std::mutex& sceneMutex = Project::getSceneMutex();

	std::lock_guard<std::mutex> lock(sceneMutex); // unlocks the moment this goes out of scope

	removeDeadEntities();
	addNewEntities();

	if (!isPlaying) return;

	if (dt <= 0.0f || dt > 0.1f)
		dt = 1.0f / 60.0f;

	const float fixedPhysicsStep = 1.0f / 60.0f;
	physicsAccumulator += dt;
	colliderCacheTimer += dt;

	// NOTE: Maybe Scene.h is not the best place to enforce this
	if (colliderCacheTimer > 1.0f) {
		physicsWorld->cleanShapeCache();
		colliderCacheTimer -= 1.0f;
	}

	bool isPhysicsLoop = physicsAccumulator >= fixedPhysicsStep;

	if (isPhysicsLoop)
		pushRigidBodies();

	while (physicsAccumulator >= fixedPhysicsStep) {
		physicsWorld->step(fixedPhysicsStep);
		physicsAccumulator -= fixedPhysicsStep;
	}

	if (isPhysicsLoop)
		pullRigidBodies();

	tickBehaviours(dt);
}

void Scene::pushRigidBodies() {
	ECS::ComponentPool<Transform>& transforms = componentRegistry->getTransforms();
	ECS::ComponentPool<MeshData>& meshes = componentRegistry->getMeshes();

	for (auto item : componentRegistry->getRigidBodies()) {
		if (item.component.type == RigidBodyType::Static) continue; // no need to push statics

		Transform* transform = transforms.get(item.entity);
		if (!transform) continue;

		RigidBodySystem::pushToWorld(item.component, *transform, meshes.get(item.entity), *physicsWorld);
	}
}

void Scene::pullRigidBodies() {
	ECS::ComponentPool<Transform>& transforms = componentRegistry->getTransforms();

	for (auto item : componentRegistry->getRigidBodies()) {
		if (item.component.type != RigidBodyType::Dynamic) continue; // no need to pull statics and kinematics

		Transform* transform = transforms.get(item.entity);
		if (!transform) continue;

		RigidBodySystem::pullFromWorld(item.component, *transform, this);
	}
}

void Scene::tickBehaviours(float dt) {
	behaviourTickList.clear();

	for (auto& [UID, pool] : componentRegistry->getBehaviourPools()) {
		for (auto item : pool)
			behaviourTickList.push_back({ UID, item.entity.entityId, item.entity.generation });
	}

	for (const BehaviourTickEntry& entry : behaviourTickList) {
		ECS::EntityHandle handle{ entry.entityId, entry.generation };
		if (!entityManager->isAlive(handle)) continue;

		ECS::ComponentPoolRegistry::BehaviourPool* pool = componentRegistry->getBehaviourPool(entry.UID);
		if (!pool) continue;

		std::unique_ptr<BehaviourComponent>* slot = pool->get(handle);
		if (slot && *slot)
			(*slot)->tick(dt);
	}

	for (const BehaviourTickEntry& entry : behaviourTickList) {
		ECS::EntityHandle handle{ entry.entityId, entry.generation };
		if (!entityManager->isAlive(handle)) continue;

		ECS::ComponentPoolRegistry::BehaviourPool* pool = componentRegistry->getBehaviourPool(entry.UID);
		if (!pool) continue;

		std::unique_ptr<BehaviourComponent>* slot = pool->get(handle);
		if (slot && *slot && (*slot)->started)
			(*slot)->onLateUpdate(dt);
	}
}

PhysicsWorld& Scene::getPhysicsWorld() { return *physicsWorld; }

ECS::ComponentPoolRegistry* Scene::getComponentPoolRegistry() const {
	return componentRegistry.get();
}

void Scene::addBuiltInComponent(const ECS::EntityHandle& handle, unsigned int UID) {
	ECS::IComponentPool* pool = componentRegistry->getPool(UID);
	if (!pool)
		throw std::runtime_error("Unknown built-in component type");

	if (!entityManager->isAlive(handle))
		throw std::runtime_error("Cannot add a component to an entity that is not alive");

	if (pool->has(handle))
		throw std::runtime_error("Entity already has this component type");

	pool->addDefault(handle);
}

bool Scene::hasComponent(const ECS::EntityHandle& handle, unsigned int UID) const {
	if (!entityManager->isAlive(handle)) return false;

	if (ECS::IComponentPool* pool = componentRegistry->getPool(UID))
		return pool->has(handle);

	if (ECS::ComponentPoolRegistry::BehaviourPool* pool = componentRegistry->getBehaviourPool(UID))
		return pool->has(handle);

	return false;
}

void Scene::removeComponent(const ECS::EntityHandle& handle, unsigned int UID) {
	// the transform is not optional, everything else assumes every live entity has one
	if (UID == componentTypeUID<TransformComponent>()) return;

	if (ECS::IComponentPool* pool = componentRegistry->getPool(UID)) {
		pool->remove(handle);
		return;
	}

	if (ECS::ComponentPoolRegistry::BehaviourPool* pool = componentRegistry->getBehaviourPool(UID))
		pool->remove(handle);
}

BehaviourComponent* Scene::addBehaviour(const ECS::EntityHandle& handle, unsigned int UID,
	std::unique_ptr<BehaviourComponent> behaviour) {
	// the caller keeps a raw pointer to the behaviour, so dropping it silently here would leave that dangling
	if (!behaviour || !entityManager->isAlive(handle))
		throw std::runtime_error("Cannot add a component to an entity that is not alive");

	behaviour->entity = Entity(this, handle);
	behaviour->typeUID = UID;

	ECS::ComponentPoolRegistry::BehaviourPool& pool = componentRegistry->getOrCreateBehaviourPool(UID);

	std::unique_ptr<BehaviourComponent>& slot = pool.add(handle);
	slot = std::move(behaviour);

	return slot.get();
}

BehaviourComponent* Scene::getBehaviour(const ECS::EntityHandle& handle, unsigned int UID) const {
	ECS::ComponentPoolRegistry::BehaviourPool* pool = componentRegistry->getBehaviourPool(UID);
	if (!pool) return nullptr;

	std::unique_ptr<BehaviourComponent>* slot = pool->get(handle);
	return slot ? slot->get() : nullptr;
}

void Scene::addNewEntities() {
	for (size_t i = 0; i < entitiesToAdd.size(); i++) {
		ECS::EntityHandle handle = entitiesToAdd[i].getHandle();

		// removed before it was ever flushed, removeDeadEntities already destroyed it
		if (!entityManager->isAlive(handle)) continue;

		ensureSparseSize(handle);

		entities.push_back(entitiesToAdd[i]);
		entityNames.push_back(std::move(namesToAdd[i]));
		sparseEntities[handle] = static_cast<uint32_t>(entities.size() - 1);
	}
	entitiesToAdd.clear();
	namesToAdd.clear();
}

void Scene::removeDeadEntities() {
	for (Entity& entity : entitiesToRemove) {
		ECS::EntityHandle handle = entity.getHandle();

		// duplicate request, e.g. removed explicitly and again through its parent
		if (!entityManager->isAlive(handle)) continue;

		swapAndPopEntity(handle);
		componentRegistry->removeEntityComponents(handle);
		entityManager->destroy(handle); // bumps the generation, every handle to this entity is stale from here on
	}
	entitiesToRemove.clear();
}

void Scene::clear() {
	entities.clear();
	entityNames.clear();
	entitiesToAdd.clear();
	namesToAdd.clear();
	entitiesToRemove.clear();
	sparseEntities.clear();

	// generations restart from 0 after the reset below, a stale handle here could match a brand new entity
	activeCameraEntityId = ECS::INVALID_ENTITY_INDEX;
	activeCameraGeneration = ECS::INVALID_ENTITY_GENERATION;

	entityManager->reset(); // its OWN reset function not std::unique_ptr::reset()
	componentRegistry->reset(); // again

	isPlaying = false;
}

void Scene::ensureSparseSize(const ECS::EntityHandle& handle) {
	assert(!handle.isInvalid());

	std::size_t sparseSize = sparseEntities.size();

	if (handle >= static_cast<uint32_t>(sparseSize))
		sparseEntities.resize(std::max(sparseSize * 2,
			static_cast<std::size_t>(handle.entityId + 1)), ECS::INVALID_ENTITY_INDEX);
}
