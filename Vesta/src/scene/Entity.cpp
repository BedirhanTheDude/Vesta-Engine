#include <scene/Entity.h>

#include <scene/Scene.h>
#include <scene/components/TransformComponent.h>
#include <scene/components/BehaviourComponent.h>
#include <scene/components/ComponentFactory.h>
#include <ecs/EntityHandle.h>

Entity::Entity(Scene* owningScene, const ECS::EntityHandle& handle)
    : m_Scene(owningScene), m_EntityId(handle.entityId), m_Generation(handle.generation) {
}

ECS::EntityHandle Entity::getHandle() const {
    return { m_EntityId, m_Generation };
}

std::string Entity::getName() const {
    if (!isValid()) return std::string();

    return m_Scene->getEntityName(getHandle());
}

void Entity::setName(const std::string& newName) const {
    if (!isValid()) return;

    m_Scene->renameEntity(getHandle(), newName);
}

bool Entity::isValid() const {
    return m_Scene != nullptr && m_EntityId != ECS::INVALID_ENTITY_INDEX
        && m_Generation != ECS::INVALID_ENTITY_GENERATION;
}

bool Entity::isAlive() const {
    return isValid() && m_Scene->entityExists(getHandle());
}

TransformComponent Entity::getTransform() const {
    return TransformComponent(*this);
}

bool Entity::hasComponent(unsigned int UID) const {
    return isValid() && m_Scene->hasComponent(getHandle(), UID);
}

BehaviourComponent* Entity::getBehaviour(unsigned int UID) const {
    return getBehaviourRaw(UID);
}

void Entity::removeComponent(unsigned int UID) {
    ComponentFactory::remove(UID, *this);
}

void Entity::addBuiltInRaw(unsigned int UID) const {
    if (!isValid())
        throw std::runtime_error("Cannot add a component to an invalid entity");

    m_Scene->addBuiltInComponent(getHandle(), UID);
}

void Entity::removeComponentRaw(unsigned int UID) const {
    if (!isValid()) return;

    m_Scene->removeComponent(getHandle(), UID);
}

BehaviourComponent* Entity::addBehaviourRaw(unsigned int UID) const {
    if (!isValid())
        throw std::runtime_error("Cannot add a component to an invalid entity");

    return m_Scene->addBehaviour(getHandle(), UID);
}

BehaviourComponent* Entity::getBehaviourRaw(unsigned int UID) const {
    if (!isValid()) return nullptr;

    return m_Scene->getBehaviour(getHandle(), UID);
}
