#include <scene/Entity.h>

#include <scene/Scene.h>
#include <scene/components/TransformComponent.h>
#include <scene/components/BehaviourComponent.h>
#include <scene/components/ComponentFactory.h>
#include <ecs/EntityHandle.h>

Entity::Entity(Scene* owningScene, const ECS::EntityHandle& handle)
    : scene(owningScene), entityId(handle.entityId), generation(handle.generation) {
}

ECS::EntityHandle Entity::getHandle() const {
    return { entityId, generation };
}

std::string Entity::getName() const {
    if (!isValid()) return std::string();

    return scene->getEntityName(getHandle());
}

void Entity::setName(const std::string& newName) const {
    if (!isValid()) return;

    scene->renameEntity(getHandle(), newName);
}

bool Entity::isValid() const {
    return scene != nullptr && entityId != ECS::INVALID_ENTITY_INDEX
        && generation != ECS::INVALID_ENTITY_GENERATION;
}

bool Entity::isAlive() const {
    return isValid() && scene->entityExists(getHandle());
}

TransformComponent Entity::getTransform() const {
    return TransformComponent(*this);
}

bool Entity::hasComponent(unsigned int UID) const {
    return isValid() && scene->hasComponent(getHandle(), UID);
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

    scene->addBuiltInComponent(getHandle(), UID);
}

void Entity::removeComponentRaw(unsigned int UID) const {
    if (!isValid()) return;

    scene->removeComponent(getHandle(), UID);
}

BehaviourComponent* Entity::addBehaviourRaw(unsigned int UID, std::unique_ptr<BehaviourComponent> behaviour) const {
    if (!isValid())
        throw std::runtime_error("Cannot add a component to an invalid entity");

    return scene->addBehaviour(getHandle(), UID, std::move(behaviour));
}

BehaviourComponent* Entity::getBehaviourRaw(unsigned int UID) const {
    if (!isValid()) return nullptr;

    return scene->getBehaviour(getHandle(), UID);
}
