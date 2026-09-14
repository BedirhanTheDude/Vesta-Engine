#define GLM_ENABLE_EXPERIMENTAL

#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

#include <persistance/Archive.h>

#include <cmath>

#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static float wrap360(float a)
{
    while (a > 360.0f)
        a -= 360.0f;

    while (a < -360.0f)
        a += 360.0f;

    return a;
}

static glm::vec3 wrapEuler360(glm::vec3 e)
{
    return {
        wrap360(e.x),
        wrap360(e.y),
        wrap360(e.z)
    };
}

static float wrap180(float a)
{
    return glm::degrees(glm::atan(glm::sin(glm::radians(a)), glm::cos(glm::radians(a))));
}

static float nearestAngleDegrees(float angle, float reference)
{
    return reference + wrap180(angle - reference);
}

static glm::vec3 closestEulerDegrees(glm::quat q, glm::vec3 previousDegrees)
{
    glm::vec3 e = glm::degrees(glm::eulerAngles(glm::normalize(q)));

    e.x = nearestAngleDegrees(e.x, previousDegrees.x);
    e.y = nearestAngleDegrees(e.y, previousDegrees.y);
    e.z = nearestAngleDegrees(e.z, previousDegrees.z);

    return e;
}

TransformComponent::TransformComponent() = default;

TransformComponent::~TransformComponent()
{
    if (parent)
        parent->removeChild(this);

    for (auto* child : children)
        child->parent = nullptr;
}

void TransformComponent::serialize(Archive& arch) const {};
void TransformComponent::deserialize(const Archive& arch) {};

glm::mat4 TransformComponent::getMatrix() const
{
    if (isMatrixValid)
        return modelMatrix;

    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, position);
    local *= glm::mat4_cast(rotation);
    local = glm::scale(local, scale);

    modelMatrix = parent ? parent->getMatrix() * local : local;
    isMatrixValid = true;

    return modelMatrix;
}

void TransformComponent::invalidate()
{
    isMatrixValid = false;
    worldEulerValid = false;
    physicsDirty = true;

    for (auto* child : children)
        child->invalidate();
}

void TransformComponent::setParent(TransformComponent* newParent)
{
    if (onBeforeReparent && !onBeforeReparent())
        return;

    if (parent == newParent)
        return;

    glm::mat4 worldMatrix = getMatrix();

    if (parent)
        parent->removeChild(this);

    parent = newParent;

    if (parent) {
        parent->children.push_back(this);
        parent->childrenIDs.insert(this->owner->getID());
    }
        
    if (parent) {
        glm::mat4 localMatrix = glm::inverse(parent->getMatrix()) * worldMatrix;

        position = glm::vec3(localMatrix[3]);

        scale = glm::vec3(
            glm::length(glm::vec3(localMatrix[0])),
            glm::length(glm::vec3(localMatrix[1])),
            glm::length(glm::vec3(localMatrix[2]))
        );

        glm::mat3 rotMat(
            glm::vec3(localMatrix[0]) / scale.x,
            glm::vec3(localMatrix[1]) / scale.y,
            glm::vec3(localMatrix[2]) / scale.z
        );

        rotation = glm::normalize(glm::quat_cast(rotMat));

        cachedEuler = wrapEuler360(closestEulerDegrees(rotation, cachedEuler));
        eulerDirty = false;
    }

    invalidate();
}

void TransformComponent::removeChild(TransformComponent* child)
{
    childrenIDs.erase(child->owner->getID());
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
    invalidate();
}

void TransformComponent::translate(const glm::vec3& delta)
{
    position += delta;
    invalidate();
}

void TransformComponent::setPosition(const glm::vec3& pos)
{
    position = pos;
    invalidate();
}

void TransformComponent::setRotation(const glm::vec3& eulerDegrees)
{
    cachedEuler = wrapEuler360(eulerDegrees);
    eulerDirty = false;

    rotation = glm::normalize(glm::quat(glm::radians(cachedEuler)));

    invalidate();
}

void TransformComponent::setRotation(const glm::quat& quat)
{
    rotation = glm::normalize(quat);
    eulerDirty = true;

    invalidate();
}

void TransformComponent::rotate(const glm::vec3& eulerDeltaDegrees)
{
    glm::vec3 wrappedDelta = wrapEuler360(eulerDeltaDegrees);

    glm::quat delta = glm::quat(glm::radians(wrappedDelta));
    rotation = glm::normalize(delta * rotation);

    eulerDirty = true;

    invalidate();
}

void TransformComponent::rotateAroundAxis(const glm::vec3& axis, float angleDegrees)
{
    if (glm::length2(axis) == 0.0f)
        return;

    float wrappedAngle = wrap360(angleDegrees);

    rotation = glm::normalize(
        glm::angleAxis(glm::radians(wrappedAngle), glm::normalize(axis)) * rotation
    );

    eulerDirty = true;

    invalidate();
}

void TransformComponent::setScale(const glm::vec3& scale)
{
    this->scale = scale;
    invalidate();
}

void TransformComponent::scaleBy(const glm::vec3& factor)
{
    scale *= factor;
    invalidate();
}

glm::mat3 TransformComponent::getRotationMatrix() const
{
    return glm::mat3_cast(rotation);
}

glm::vec3 TransformComponent::forward() const
{
    return glm::normalize(rotation * glm::vec3(0, 0, -1));
}

glm::vec3 TransformComponent::right() const
{
    return glm::normalize(rotation * glm::vec3(1, 0, 0));
}

glm::vec3 TransformComponent::up() const
{
    return glm::normalize(rotation * glm::vec3(0, 1, 0));
}

glm::vec3 TransformComponent::getPosition() const
{
    return position;
}

glm::vec3 TransformComponent::getEulerRotation() const
{
    if (eulerDirty) {
        cachedEuler = wrapEuler360(closestEulerDegrees(rotation, cachedEuler));
        eulerDirty = false;
    }

    return cachedEuler;
}

glm::quat TransformComponent::getRotationQuat() const
{
    return rotation;
}

glm::vec3 TransformComponent::getRawEulerRotation() const
{
    return wrapEuler360(glm::degrees(glm::eulerAngles(rotation)));
}

glm::vec3 TransformComponent::getScale() const
{
    return scale;
}

glm::vec3 TransformComponent::getWorldPosition() const
{
    return glm::vec3(getMatrix()[3]);
}

glm::vec3 TransformComponent::getWorldEulerAngles() const
{
    glm::quat world = getWorldRotationQuat();

    if (!worldEulerValid) {
        cachedWorldEuler = wrapEuler360(closestEulerDegrees(world, cachedWorldEuler));
        worldEulerValid = true;
    }

    return cachedWorldEuler;
}

glm::quat TransformComponent::getWorldRotationQuat() const
{
    if (!parent)
        return rotation;

    return glm::normalize(parent->getWorldRotationQuat() * rotation);
}

glm::vec3 TransformComponent::getWorldScale() const
{
    if (!parent)
        return scale;

    return parent->getWorldScale() * scale;
}