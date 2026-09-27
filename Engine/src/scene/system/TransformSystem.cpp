#define GLM_ENABLE_EXPERIMENTAL

#include "TransformSystem.h"

#include <persistance/Archive.h>

#include <ecs/EntityHandle.h>
#include <ecs/ComponentPool.h>
#include <ecs/ComponentPoolRegistry.h>

#include <scene/Scene.h>
#include <core/Application.h>

#include <vector>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <memory>
#include <algorithm>

#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

static float wrap360(float a)
{
    return std::fmod(a, 360.0f);
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

namespace TransformSystem {

    static void invalidate(Transform& transform, Scene* scene);

    // don't know yet
    void serialize(const Transform& transform, Archive& arch) {};
    void deserialize(Transform& transform, const Archive& arch) {};

	glm::mat4 getMatrix(const Transform& transform, Scene* scene) {
        if (transform.isMatrixValid)
            return transform.modelMatrix;

        if (!scene) scene = Application::getCurrentScene();

        glm::mat4 local = glm::mat4(1.0f);
        local = glm::translate(local, transform.position);
        local *= glm::mat4_cast(transform.rotation);
        local = glm::scale(local, transform.scale);

        transform.modelMatrix = local;

        if (!transform.parent.isInvalid()) {
            Transform* parent = scene->getComponentPoolRegistry()->getTransforms().get(transform.parent);
            if (parent) {
                transform.modelMatrix = getMatrix(*parent, scene) * local;
            }
        }

        transform.isMatrixValid = true;

        return transform.modelMatrix;
	}

    void translate(Transform& transform, const glm::vec3& delta, Scene* scene) {
        transform.position += delta;

        invalidate(transform, scene);
    }

    void setPosition(Transform& transform, const glm::vec3& pos, Scene* scene) {
        transform.position = pos;

        invalidate(transform, scene);
    }

    void setRotation(Transform& transform, const glm::vec3& eulerDegrees, Scene* scene) {
        transform.cachedEuler = wrapEuler360(eulerDegrees);
        transform.eulerDirty = false;

        transform.rotation = glm::normalize(glm::quat(glm::radians(transform.cachedEuler)));

        invalidate(transform, scene);
    }

    void setRotation(Transform& transform, const glm::quat& quat, Scene* scene) {
        transform.rotation = glm::normalize(quat);
        transform.eulerDirty = true;

        invalidate(transform, scene);
    }

    void rotate(Transform& transform, const glm::vec3& eulerDeltaDegrees, Scene* scene) {
        glm::vec3 wrappedDelta = wrapEuler360(eulerDeltaDegrees);

        glm::quat delta = glm::quat(glm::radians(wrappedDelta));
        transform.rotation = glm::normalize(delta * transform.rotation);

        transform.eulerDirty = true;

        invalidate(transform, scene);
    }

    void rotateAroundAxis(Transform& transform, const glm::vec3& axis, float angleDegrees, Scene* scene) {
        if (glm::length2(axis) == 0.0f)
            return;

        float wrappedAngle = wrap360(angleDegrees);

        transform.rotation = glm::normalize(
            glm::angleAxis(glm::radians(wrappedAngle), glm::normalize(axis)) * transform.rotation
        );

        transform.eulerDirty = true;

        invalidate(transform, scene);
    }

    void setScale(Transform& transform, const glm::vec3& scale, Scene* scene) {
        if (!glm::all(glm::greaterThanEqual(scale, glm::vec3(0.0f))))
            return;

        transform.scale = scale;
        invalidate(transform, scene);
    }

    void scaleBy(Transform& transform, const glm::vec3& factor, Scene* scene) {
        if (!glm::all(glm::greaterThanEqual(factor, glm::vec3(0.0f))))
            return;

        transform.scale *= factor;
        invalidate(transform, scene);
    }

    // NOTE: Unrelated transform and entityHandle results in undefined behaviour
    // This preserves world position and orientation while reparenting
    void setParent(Transform& transform, const ECS::EntityHandle& entityHandle,
        const ECS::EntityHandle& parentHandle, Scene* scene) {
        if (!scene) scene = Application::getCurrentScene();

        if (transform.parent == parentHandle)
            return;

        ECS::ComponentPool<Transform>& pool = scene->getComponentPoolRegistry()->getTransforms();

        assert(pool.get(entityHandle) == &transform);

        // a rigid body simulates in world space, reparenting its entity would desync it from the physics actor
        if (scene->getComponentPoolRegistry()->getRigidBodies().has(entityHandle)) {
            printf("Warning: Cannot reparent entity with RigidBodyComponent.\n");
            return;
        }

        Transform* parentTransform = pool.get(parentHandle);
        if (!parentHandle.isInvalid() && !parentTransform)
            return;

        // walk up from the new parent, reaching this entity means it is our own descendant (or ourselves)
        for (ECS::EntityHandle ancestorHandle = parentHandle; !ancestorHandle.isInvalid();) {
            if (ancestorHandle == entityHandle)
                return;

            Transform* ancestor = pool.get(ancestorHandle);
            if (!ancestor) break;

            ancestorHandle = ancestor->parent;
        }

        glm::mat4 worldMatrix = getMatrix(transform, scene);

        if (!transform.parent.isInvalid()) {
            Transform* oldParentTransform = pool.get(transform.parent);
            if (oldParentTransform) removeChild(*oldParentTransform, entityHandle);
        }

        transform.parent = parentHandle;

        if (parentTransform)
            parentTransform->children.push_back(entityHandle);

        // no parent means local space is world space
        glm::mat4 localMatrix = parentTransform
            ? glm::inverse(getMatrix(*parentTransform, scene)) * worldMatrix
            : worldMatrix;

        transform.position = glm::vec3(localMatrix[3]);

        glm::vec3 xAxis = glm::vec3(localMatrix[0]);
        glm::vec3 yAxis = glm::vec3(localMatrix[1]);
        glm::vec3 zAxis = glm::vec3(localMatrix[2]);

        transform.scale = glm::vec3(glm::length(xAxis), glm::length(yAxis), glm::length(zAxis));

        // a zero scale axis has no direction to recover, keep the previous rotation instead of dividing by zero
        if (transform.scale.x > 1e-6f && transform.scale.y > 1e-6f && transform.scale.z > 1e-6f) {
            glm::mat3 rotMat(
                xAxis / transform.scale.x,
                yAxis / transform.scale.y,
                zAxis / transform.scale.z
            );

            transform.rotation = glm::normalize(glm::quat_cast(rotMat));

            transform.cachedEuler = wrapEuler360(closestEulerDegrees(transform.rotation, transform.cachedEuler));
            transform.eulerDirty = false;
        }

        invalidate(transform, scene);
    }

    void removeChild(Transform& transform, const ECS::EntityHandle& childHandle) {
        transform.children.erase(std::remove(transform.children.begin(),
            transform.children.end(), childHandle), transform.children.end());
        //invalidate(transform); // I don't think removing a child invalidates any caches?
    }

    glm::mat3 getRotationMatrix(const Transform& transform) {
        return glm::mat3_cast(transform.rotation);
    }

    glm::vec3 forward(const Transform& transform) {
        return glm::normalize(transform.rotation * glm::vec3(0, 0, -1));
    }

    glm::vec3 right(const Transform& transform) {
        return glm::normalize(transform.rotation * glm::vec3(1, 0, 0));
    }

    glm::vec3 up(const Transform& transform) {
        return glm::normalize(transform.rotation * glm::vec3(0, 1, 0));
    }

    glm::vec3 getPosition(const Transform& transform) {
        return transform.position;
    }

    glm::vec3 getEulerRotation(const Transform& transform) {
        if (transform.eulerDirty) {
            transform.cachedEuler = wrapEuler360(closestEulerDegrees(transform.rotation, transform.cachedEuler));
            transform.eulerDirty = false;
        }

        return transform.cachedEuler;
    }

    glm::quat getRotationQuat(const Transform& transform) {
        return transform.rotation;
    }

    glm::vec3 getRawEulerRotation(const Transform& transform) {
        return wrapEuler360(glm::degrees(glm::eulerAngles(transform.rotation)));
    }

    glm::vec3 getScale(const Transform& transform) {
        return transform.scale;
    }

    glm::vec3 getWorldPosition(const Transform& transform, Scene* scene) {
        return glm::vec3(getMatrix(transform, scene)[3]);
    }

    glm::vec3 getWorldEulerAngles(const Transform& transform, Scene* scene) {
        glm::quat world = getWorldRotationQuat(transform, scene);

        if (!transform.worldEulerValid) {
            transform.cachedWorldEuler = wrapEuler360(closestEulerDegrees(world, transform.cachedWorldEuler));
            transform.worldEulerValid = true;
        }

        return transform.cachedWorldEuler;
    }

    glm::quat getWorldRotationQuat(const Transform& transform, Scene* scene) {
        if (transform.parent.isInvalid())
            return transform.rotation;

        if (!scene) scene = Application::getCurrentScene();

        ECS::ComponentPool<Transform>& pool = scene->getComponentPoolRegistry()->getTransforms();
        Transform* parentTransform = pool.get(transform.parent);

        return (parentTransform) ? glm::normalize(getWorldRotationQuat(*parentTransform, scene) * transform.rotation)
            : transform.rotation;
    }

    glm::vec3 getWorldScale(const Transform& transform, Scene* scene) {
        if (transform.parent.isInvalid())
            return transform.scale;

        if (!scene) scene = Application::getCurrentScene();

        ECS::ComponentPool<Transform>& pool = scene->getComponentPoolRegistry()->getTransforms();
        Transform* parentTransform = pool.get(transform.parent);

        return (parentTransform) ? getWorldScale(*parentTransform, scene) * transform.scale : transform.scale;
    }

    bool tryGetParent(const Transform& transform, ECS::EntityHandle& parentHandle) {
        parentHandle = transform.parent;

        return !parentHandle.isInvalid();
    }

    const std::vector<ECS::EntityHandle>& getChildren(const Transform& transform) {
        return transform.children;
    }

    static void invalidate(Transform& transform, Scene* scene) {
        transform.isMatrixValid = false;
        transform.worldEulerValid = false;
        transform.physicsDirty = true;

        if (transform.children.empty()) return;

        if (!scene) scene = Application::getCurrentScene();
        ECS::ComponentPool<Transform>& pool = scene->getComponentPoolRegistry()->getTransforms();

        for (ECS::EntityHandle& childHandle : transform.children) {
            Transform* childTransform = pool.get(childHandle);
            if (childTransform) invalidate(*childTransform, scene);
        }
    }
}
