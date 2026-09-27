#include <scene/components/TransformComponent.h>

#include <scene/components/Transform.h>
#include <scene/components/ProxyUtil.h>
#include <scene/system/TransformSystem.h>

glm::mat4 TransformComponent::getMatrix() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    if (!transform) return glm::mat4(1.0f);

    return TransformSystem::getMatrix(*transform, sceneOf(entity));
}

void TransformComponent::translate(const glm::vec3& delta)
{
    if (Transform* transform = resolveComponent<Transform>(entity))
        TransformSystem::translate(*transform, delta, sceneOf(entity));
}

void TransformComponent::setPosition(const glm::vec3& pos)
{
    if (Transform* transform = resolveComponent<Transform>(entity))
        TransformSystem::setPosition(*transform, pos, sceneOf(entity));
}

void TransformComponent::setRotation(const glm::vec3& eulerDegrees)
{
    if (Transform* transform = resolveComponent<Transform>(entity))
        TransformSystem::setRotation(*transform, eulerDegrees, sceneOf(entity));
}

void TransformComponent::setRotation(const glm::quat& quat)
{
    if (Transform* transform = resolveComponent<Transform>(entity))
        TransformSystem::setRotation(*transform, quat, sceneOf(entity));
}

void TransformComponent::rotate(const glm::vec3& eulerDeltaDegrees)
{
    if (Transform* transform = resolveComponent<Transform>(entity))
        TransformSystem::rotate(*transform, eulerDeltaDegrees, sceneOf(entity));
}

void TransformComponent::rotateAroundAxis(const glm::vec3& axis, float angleDegrees)
{
    if (Transform* transform = resolveComponent<Transform>(entity))
        TransformSystem::rotateAroundAxis(*transform, axis, angleDegrees, sceneOf(entity));
}

void TransformComponent::setScale(const glm::vec3& scale)
{
    if (Transform* transform = resolveComponent<Transform>(entity))
        TransformSystem::setScale(*transform, scale, sceneOf(entity));
}

void TransformComponent::scaleBy(const glm::vec3& factor)
{
    if (Transform* transform = resolveComponent<Transform>(entity))
        TransformSystem::scaleBy(*transform, factor, sceneOf(entity));
}

void TransformComponent::setParent(const Entity& newParent)
{
    Transform* transform = resolveComponent<Transform>(entity);
    if (!transform) return;

    // a parent from another scene has no meaning in this scene's pools
    if (newParent.isValid() && sceneOf(newParent) != sceneOf(entity)) return;

    ECS::EntityHandle parentHandle = newParent.isValid() ? newParent.getHandle() : ECS::INVALID_ENTITY_HANDLE;

    TransformSystem::setParent(*transform, entity.getHandle(), parentHandle, sceneOf(entity));
}

glm::mat3 TransformComponent::getRotationMatrix() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getRotationMatrix(*transform) : glm::mat3(1.0f);
}

glm::vec3 TransformComponent::forward() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::forward(*transform) : glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 TransformComponent::right() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::right(*transform) : glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 TransformComponent::up() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::up(*transform) : glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 TransformComponent::getPosition() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getPosition(*transform) : glm::vec3(0.0f);
}

glm::vec3 TransformComponent::getEulerRotation() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getEulerRotation(*transform) : glm::vec3(0.0f);
}

glm::quat TransformComponent::getRotationQuat() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getRotationQuat(*transform) : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

glm::vec3 TransformComponent::getRawEulerRotation() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getRawEulerRotation(*transform) : glm::vec3(0.0f);
}

glm::vec3 TransformComponent::getScale() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getScale(*transform) : glm::vec3(1.0f);
}

glm::vec3 TransformComponent::getWorldPosition() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getWorldPosition(*transform, sceneOf(entity)) : glm::vec3(0.0f);
}

glm::vec3 TransformComponent::getWorldEulerAngles() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getWorldEulerAngles(*transform, sceneOf(entity)) : glm::vec3(0.0f);
}

glm::quat TransformComponent::getWorldRotationQuat() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getWorldRotationQuat(*transform, sceneOf(entity)) : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

glm::vec3 TransformComponent::getWorldScale() const
{
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? TransformSystem::getWorldScale(*transform, sceneOf(entity)) : glm::vec3(1.0f);
}

Entity TransformComponent::getParent() const
{
    Transform* transform = resolveComponent<Transform>(entity);

    ECS::EntityHandle parentHandle;
    if (!transform || !TransformSystem::tryGetParent(*transform, parentHandle))
        return Entity();

    return Entity(sceneOf(entity), parentHandle);
}

std::vector<Entity> TransformComponent::getChildren() const
{
    std::vector<Entity> children;

    Transform* transform = resolveComponent<Transform>(entity);
    if (!transform) return children;

    for (const ECS::EntityHandle& childHandle : TransformSystem::getChildren(*transform))
        children.push_back(Entity(sceneOf(entity), childHandle));

    return children;
}
