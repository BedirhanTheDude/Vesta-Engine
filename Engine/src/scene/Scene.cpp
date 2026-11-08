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

Scene::Scene(const std::string& sceneName) : m_SceneName(sceneName) {
	m_PhysicsWorld = std::make_unique<PhysicsWorld>();
	m_PhysicsWorld->init();

	m_EntityManager = std::make_unique<ECS::EntityManager>();
	m_ComponentRegistry = std::make_unique<ECS::ComponentPoolRegistry>();

	m_ActiveCameraEntityId = ECS::INVALID_ENTITY_INDEX;
	m_ActiveCameraGeneration = ECS::INVALID_ENTITY_GENERATION;
}

Scene::~Scene() {
	clear();
	m_PhysicsWorld->shutdown();
}

void Scene::newScene(const std::string& sceneName) {
	clear();
	this->m_SceneName = sceneName;
	m_ScenePath.clear();

	createDefaultScene(*this);
}

bool Scene::openScene(const std::string& relativeScenePath) {
	std::string previousName = m_SceneName;
	std::string previousPath = m_ScenePath;

	m_ScenePath = relativeScenePath;
	m_SceneName = std::filesystem::u8path(relativeScenePath).stem().string();

	if (SceneSerializer::load(*this)) return true;

	// load leaves the current scene untouched on failure, it keeps its own name and file
	m_SceneName = std::move(previousName);
	m_ScenePath = std::move(previousPath);
	return false;
}

Entity Scene::createEntity(const std::string& name) {
	ECS::EntityHandle handle = m_EntityManager->create();

	// every entity has a transform for its whole life, Entity::getTransform() relies on it
	m_ComponentRegistry->getTransforms().add(handle);

	m_EntitiesToAdd.push_back(Entity(this, handle));
	m_NamesToAdd.push_back(name);

	return m_EntitiesToAdd.back();
}

Entity Scene::createEntity(const std::string& name, const Entity& parent) {
	Entity entity = createEntity(name);
	parentNewEntity(entity, parent);

	return entity;
}

Entity Scene::createEntityImmediate(const std::string& name) {
	ECS::EntityHandle handle = m_EntityManager->create();

	m_ComponentRegistry->getTransforms().add(handle);

	ensureSparseSize(handle);

	m_Entities.push_back(Entity(this, handle));
	m_EntityNames.push_back(name);
	m_SparseEntities[handle] = static_cast<uint32_t>(m_Entities.size() - 1);

	return m_Entities.back();
}

Entity Scene::createEntityImmediate(const std::string& name, const Entity& parent) {
	Entity entity = createEntityImmediate(name);
	parentNewEntity(entity, parent);

	return entity;
}

void Scene::parentNewEntity(const Entity& child, const Entity& parent) {
	// a parent from another scene (or a dead/invalid one) would index the wrong pools, the child stays a root
	if (!parent.isValid() || parent.m_Scene != this || !entityExists(parent.getHandle())) return;

	Transform* transform = m_ComponentRegistry->getTransforms().get(child.getHandle());
	if (!transform) return;

	TransformSystem::setParent(*transform, child.getHandle(), parent.getHandle(), this);
}

bool Scene::tryGetEntityIndex(const ECS::EntityHandle& handle, uint32_t& outIndex) const {
	if (handle.isInvalid() || handle >= m_SparseEntities.size())
		return false;

	uint32_t index = m_SparseEntities[handle.entityId];
	if (index == ECS::INVALID_ENTITY_INDEX || index >= m_Entities.size())
		return false;

	if (!(m_Entities[index].getHandle() == handle))
		return false;

	outIndex = index;
	return true;
}

// swap and pop to maintain dense entity vector with sparse index vector
void Scene::swapAndPopEntity(const ECS::EntityHandle& handle) {
	uint32_t denseIndex;
	if (!tryGetEntityIndex(handle, denseIndex)) return;

	// remove this from parent's children
	ECS::ComponentPool<Transform>& pool = m_ComponentRegistry->getTransforms();
	Transform* entityTransform = pool.get(handle);
	if (entityTransform) {
		Transform* parentTransform = pool.get(entityTransform->parent);
		if (parentTransform)
			TransformSystem::removeChild(*parentTransform, handle);
	}

	uint32_t last = static_cast<uint32_t>(m_Entities.size() - 1);

	if (denseIndex != last) {
		Entity movedEntity = m_Entities[last];
		m_Entities[denseIndex] = movedEntity;
		m_EntityNames[denseIndex] = std::move(m_EntityNames[last]);

		m_SparseEntities[movedEntity.m_EntityId] = denseIndex;
	}

	m_Entities.pop_back();
	m_EntityNames.pop_back();
	m_SparseEntities[handle] = ECS::INVALID_ENTITY_INDEX;
}

void Scene::removeEntity(const Entity& entity) {
	if (!entity.isValid() || entity.m_Scene != this) return;

	removeEntity(entity.getHandle());
}

void Scene::removeEntity(const ECS::EntityHandle& handle) {
	if (!entityExists(handle)) return;

	// recursively kill children
	if (Transform* transform = m_ComponentRegistry->getTransforms().get(handle)) {
		for (const ECS::EntityHandle& childHandle : transform->children)
			removeEntity(childHandle);
	}

	m_EntitiesToRemove.push_back(Entity(this, handle));
}

void Scene::removeEntity(const std::string& name) {
	for (size_t i = 0; i < m_Entities.size(); i++) {
		if (m_EntityNames[i] == name) {
			/*if (onEntityRemoved)
				onEntityRemoved(entity);*/
			removeEntity(m_Entities[i].getHandle());
			return;
		}
	}
}

void Scene::removeEntity(unsigned int entityId) {
	uint32_t generation = m_EntityManager->getGeneration(static_cast<uint32_t>(entityId));
	ECS::EntityHandle handle{ entityId, generation };

	uint32_t denseIndex;
	if (!tryGetEntityIndex(handle, denseIndex)) return; // alive in EntityManager but not yet flushed from entitiesToAdd, or genuinely gone

	removeEntity(m_Entities[denseIndex].getHandle());
}

bool Scene::copyTransform(uint32_t entityId, Archive& outArch) const {
	uint32_t generation = m_EntityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return false;

	outArch.reset();

	Transform* transform = m_ComponentRegistry->getTransforms().get(handle);

	TransformSystem::serialize(*transform, outArch);
	return true; // If there is an entity there is a transform
}

bool Scene::copyComponent(uint32_t entityId, uint32_t componentUID, Archive& outArch) const {
	uint32_t generation = m_EntityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return false;

	outArch.reset();

	// Guaranteed that scene isn't modified through this entity instance
	Entity entity(const_cast<Scene*>(this), handle); // Flyweight entity
	std::string componentName = componentTypeUIDToString(componentUID);
	ComponentFactory::serializeComponent(entity, componentName, outArch);
	return outArch.has("type");
}

bool Scene::copyEntity(uint32_t entityId, Archive& outArch) const {
	uint32_t generation = m_EntityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return false;

	outArch.reset();

	Entity entity(const_cast<Scene*>(this), handle);

	outArch.set("name", entity.getName());

	Transform* transform = m_ComponentRegistry->getTransforms().get(handle);

	outArch.set("parent", std::string(""));

	Archive transformArch;
	TransformSystem::serialize(*transform, transformArch);
	outArch.set("transform", std::move(transformArch));

	ComponentFactory::serializeEntity(entity, outArch);
	return outArch.has("components");
}

void Scene::pasteTransform(uint32_t entityId, const Archive& transformArchive) {
	if (!transformArchive.has("position")) return;

	uint32_t generation = m_EntityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return;

	Transform* transform = m_ComponentRegistry->getTransforms().get(handle);

	TransformSystem::deserialize(*transform, transformArchive);
}

void Scene::pasteComponent(uint32_t entityId, const Archive& componentArchive) {
	std::string typeName;
	componentArchive.get("type", typeName);
	if (typeName.empty()) return;

	uint32_t generation = m_EntityManager->getGeneration(entityId);
	ECS::EntityHandle handle{ entityId, generation };
	if (handle.isInvalid()) return;

	Entity entity(const_cast<Scene*>(this), handle);

	uint32_t componentUID = componentNameToUID(typeName);
	auto* pool = m_ComponentRegistry->getPool(componentUID);
	if (!pool) // component wasn't built-in
		pool = m_ComponentRegistry->getScriptPool(componentUID);

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
	Transform* transform = m_ComponentRegistry->getTransforms().get(entity.getHandle());
	TransformSystem::deserialize(*transform, transformArch);

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
		m_EntityNames[denseIndex] = name;
		return;
	}

	// created this frame, not flushed into `entities` yet
	for (size_t i = 0; i < m_EntitiesToAdd.size(); i++) {
		if (m_EntitiesToAdd[i].getHandle() == handle) {
			m_NamesToAdd[i] = name;
			return;
		}
	}
}

void Scene::renameEntity(unsigned int ID, const std::string& name) {
	ECS::EntityHandle handle{ static_cast<uint32_t>(ID), m_EntityManager->getGeneration(static_cast<uint32_t>(ID)) };
	renameEntity(handle, name);
}

std::string Scene::getEntityName(const ECS::EntityHandle& handle) const {
	uint32_t denseIndex;
	if (tryGetEntityIndex(handle, denseIndex))
		return m_EntityNames[denseIndex];

	for (size_t i = 0; i < m_EntitiesToAdd.size(); i++) {
		if (m_EntitiesToAdd[i].getHandle() == handle)
			return m_NamesToAdd[i];
	}

	return std::string();
}

bool Scene::entityExists(const ECS::EntityHandle& handle) const {
	return m_EntityManager->isAlive(handle);
}

bool Scene::entityExists(const Entity& entity) const {
	return entity.isValid() && entity.m_Scene == this && entityExists(entity.getHandle());
}

bool Scene::entityExists(unsigned int ID) const {
	ECS::EntityHandle handle{ static_cast<uint32_t>(ID), m_EntityManager->getGeneration(static_cast<uint32_t>(ID)) };

	uint32_t denseIndex;
	return tryGetEntityIndex(handle, denseIndex);
}

bool Scene::entityExists(const std::string& name) const {
	for (const std::string& entityName : m_EntityNames) {
		if (entityName == name) return true;
	}
	return false;
}

Entity Scene::findEntity(const std::string& name) const {
	for (size_t i = 0; i < m_Entities.size(); i++) {
		if (m_EntityNames[i] == name) return m_Entities[i];
	}
	return Entity();
}

Entity Scene::findEntity(unsigned int ID) const {
	ECS::EntityHandle handle{ static_cast<uint32_t>(ID), m_EntityManager->getGeneration(static_cast<uint32_t>(ID)) };

	uint32_t denseIndex;
	if (tryGetEntityIndex(handle, denseIndex))
		return m_Entities[denseIndex];

	return Entity();
}

const std::vector<Entity>& Scene::getEntities() const {
	return m_Entities;
}

void Scene::setActiveCamera(const Entity& camera) {
	ECS::EntityHandle handle = camera.getHandle();
	m_ActiveCameraEntityId = handle.entityId;
	m_ActiveCameraGeneration = handle.generation;
}

Entity Scene::getActiveCameraEntity() const {
	ECS::EntityHandle handle{ m_ActiveCameraEntityId, m_ActiveCameraGeneration };

	uint32_t denseIndex;
	if (tryGetEntityIndex(handle, denseIndex))
		return m_Entities[denseIndex];

	return Entity();
}

void Scene::onCameraAdded(const Entity& camera)
{
	if (!camera.isAlive()) return;

	ECS::EntityHandle handle = camera.getHandle();
	m_ActiveCameraEntityId = handle.entityId;
	m_ActiveCameraGeneration = handle.generation;
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
	m_PhysicsAccumulator += dt;
	m_ColliderCacheTimer += dt;

	// NOTE: Maybe Scene.h is not the best place to enforce this
	if (m_ColliderCacheTimer > 1.0f) {
		m_PhysicsWorld->cleanShapeCache();
		m_ColliderCacheTimer -= 1.0f;
	}

	bool isPhysicsLoop = m_PhysicsAccumulator >= fixedPhysicsStep;

	if (isPhysicsLoop)
		pushRigidBodies();

	while (m_PhysicsAccumulator >= fixedPhysicsStep) {
		m_PhysicsWorld->step(fixedPhysicsStep);
		m_PhysicsAccumulator -= fixedPhysicsStep;
	}

	if (isPhysicsLoop)
		pullRigidBodies();

	tickBehaviours(dt);
}

void Scene::pushRigidBodies() {
	ECS::ComponentPool<Transform>& transforms = m_ComponentRegistry->getTransforms();
	ECS::ComponentPool<MeshData>& meshes = m_ComponentRegistry->getMeshes();

	for (auto item : m_ComponentRegistry->getRigidBodies()) {
		if (item.component.type == RigidBodyType::Static) continue; // no need to push statics

		Transform* transform = transforms.get(item.entity);
		if (!transform) continue;

		RigidBodySystem::pushToWorld(item.component, *transform, meshes.get(item.entity), *m_PhysicsWorld);
	}
}

void Scene::pullRigidBodies() {
	ECS::ComponentPool<Transform>& transforms = m_ComponentRegistry->getTransforms();

	for (auto item : m_ComponentRegistry->getRigidBodies()) {
		if (item.component.type != RigidBodyType::Dynamic) continue; // no need to pull statics and kinematics

		Transform* transform = transforms.get(item.entity);
		if (!transform) continue;

		RigidBodySystem::pullFromWorld(item.component, *transform, this);
	}
}

void Scene::tickBehaviours(float dt) {
	m_BehaviourTickList.clear();

	for (auto& [UID, pool] : m_ComponentRegistry->getScriptPools()) {
		for (auto item : pool)
			m_BehaviourTickList.push_back({ UID, item.entity.entityId, item.entity.generation });
	}

	for (const BehaviourTickEntry& entry : m_BehaviourTickList) {
		ECS::EntityHandle handle{ entry.entityId, entry.generation };
		if (!m_EntityManager->isAlive(handle)) continue;

		ECS::ScriptPool* pool = m_ComponentRegistry->getScriptPool(entry.UID);
		if (!pool) continue;

		BehaviourComponent* slot = pool->getBehaviour(handle);
		if (slot)
			slot->tick(dt);
	}

	for (const BehaviourTickEntry& entry : m_BehaviourTickList) {
		ECS::EntityHandle handle{ entry.entityId, entry.generation };
		if (!m_EntityManager->isAlive(handle)) continue;

		ECS::ScriptPool* pool = m_ComponentRegistry->getScriptPool(entry.UID);
		if (!pool) continue;

		BehaviourComponent* slot = pool->getBehaviour(handle);
		if (slot && slot->m_Started)
			slot->onLateUpdate(dt);
	}
}

PhysicsWorld& Scene::getPhysicsWorld() { return *m_PhysicsWorld; }

ECS::ComponentPoolRegistry* Scene::getComponentPoolRegistry() const {
	return m_ComponentRegistry.get();
}

void Scene::addBuiltInComponent(const ECS::EntityHandle& handle, unsigned int UID) {
	ECS::IComponentPool* pool = m_ComponentRegistry->getPool(UID);
	if (!pool)
		throw std::runtime_error("Unknown built-in component type");

	if (!m_EntityManager->isAlive(handle))
		throw std::runtime_error("Cannot add a component to an entity that is not alive");

	if (pool->has(handle))
		throw std::runtime_error("Entity already has this component type");

	pool->addDefault(handle);
}

bool Scene::hasComponent(const ECS::EntityHandle& handle, unsigned int UID) const {
	if (!m_EntityManager->isAlive(handle)) return false;

	if (ECS::IComponentPool* pool = m_ComponentRegistry->getPool(UID))
		return pool->has(handle);

	if (ECS::ScriptPool* pool = m_ComponentRegistry->getScriptPool(UID))
		return pool->has(handle);

	return false;
}

void Scene::removeComponent(const ECS::EntityHandle& handle, unsigned int UID) {
	// the transform is not optional, everything else assumes every live entity has one
	if (UID == componentTypeUID<TransformComponent>()) return;

	if (ECS::IComponentPool* pool = m_ComponentRegistry->getPool(UID)) {
		pool->remove(handle);
		return;
	}

	if (ECS::ScriptPool* pool = m_ComponentRegistry->getScriptPool(UID))
		pool->remove(handle);
}

BehaviourComponent* Scene::addBehaviour(const ECS::EntityHandle& handle, unsigned int UID) {
	if (!m_EntityManager->isAlive(handle))
		throw std::runtime_error("Cannot add a component to an entity that is not alive");

	auto* pool = m_ComponentRegistry->getOrCreateScriptPool(UID);
	assert(pool);

	pool->add(handle);
	auto* behaviour = pool->getBehaviour(handle);

	if (!behaviour)
		throw std::runtime_error("Failed to allocate script pool storage");

	behaviour->entity = Entity(this, handle);
	behaviour->m_TypeUID = UID;

	return behaviour;
}

BehaviourComponent* Scene::getBehaviour(const ECS::EntityHandle& handle, unsigned int UID) const {
	auto* pool = m_ComponentRegistry->getScriptPool(UID);
	if (!pool) return nullptr;

	return pool->getBehaviour(handle);
}

void Scene::addNewEntities() {
	for (size_t i = 0; i < m_EntitiesToAdd.size(); i++) {
		ECS::EntityHandle handle = m_EntitiesToAdd[i].getHandle();

		// removed before it was ever flushed, removeDeadEntities already destroyed it
		if (!m_EntityManager->isAlive(handle)) continue;

		ensureSparseSize(handle);

		m_Entities.push_back(m_EntitiesToAdd[i]);
		m_EntityNames.push_back(std::move(m_NamesToAdd[i]));
		m_SparseEntities[handle] = static_cast<uint32_t>(m_Entities.size() - 1);
	}
	m_EntitiesToAdd.clear();
	m_NamesToAdd.clear();
}

void Scene::removeDeadEntities() {
	for (Entity& entity : m_EntitiesToRemove) {
		ECS::EntityHandle handle = entity.getHandle();

		// duplicate request, e.g. removed explicitly and again through its parent
		if (!m_EntityManager->isAlive(handle)) continue;

		swapAndPopEntity(handle);
		m_ComponentRegistry->removeEntityComponents(handle);
		m_EntityManager->destroy(handle); // bumps the generation, every handle to this entity is stale from here on
	}
	m_EntitiesToRemove.clear();
}

void Scene::clearScriptPools() {
	m_ComponentRegistry->getScriptPools().clear();
}

void Scene::clear() {
	m_Entities.clear();
	m_EntityNames.clear();
	m_EntitiesToAdd.clear();
	m_NamesToAdd.clear();
	m_EntitiesToRemove.clear();
	m_SparseEntities.clear();

	// generations restart from 0 after the reset below, a stale handle here could match a brand new entity
	m_ActiveCameraEntityId = ECS::INVALID_ENTITY_INDEX;
	m_ActiveCameraGeneration = ECS::INVALID_ENTITY_GENERATION;

	m_EntityManager->reset(); // its OWN reset function not std::unique_ptr::reset()
	m_ComponentRegistry->reset(); // again

	isPlaying = false;
}

void Scene::ensureSparseSize(const ECS::EntityHandle& handle) {
	assert(!handle.isInvalid());

	std::size_t sparseSize = m_SparseEntities.size();

	if (handle >= static_cast<uint32_t>(sparseSize))
		m_SparseEntities.resize(std::max(sparseSize * 2,
			static_cast<std::size_t>(handle.entityId + 1)), ECS::INVALID_ENTITY_INDEX);
}
