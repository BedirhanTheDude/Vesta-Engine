#include <scene/Entity.h>

#include <persistance/Archive.h>

inline std::string cleanTypeName(const std::string& name) {
    const std::string prefix = "class ";
    if (name.substr(0, prefix.size()) == prefix)
        return name.substr(prefix.size());
    return name;
}

Entity::Entity(Scene* owningScene, const std::string& entityName, unsigned int ID, Entity* parent)
: scene(owningScene), name(entityName) {
	transform.owner = this;

	if (parent) {
		transform.setParent(&(parent->transform));
	}

    this->ID = ID;
}

Entity::Entity(Scene* owningScene, const std::string& entityName, unsigned int ID,
	const glm::vec3& pos, const glm::vec3& rot,
	const glm::vec3& scale, Entity* parent) : Entity(owningScene, entityName, ID, parent) {
	transform.setPosition(pos);
	transform.setRotation(rot);
	transform.setScale(scale);

    this->ID = ID;
}

void Entity::serialize(Archive& arch) const {
    arch.set("name", name);

    TransformComponent* parent = transform.getParent();
    if (parent && parent->owner)
        arch.set("parent", parent->owner->getName());
    else
        arch.set("parent", std::string(""));

    glm::vec3 worldPos = transform.getWorldPosition();
    glm::quat worldRot = transform.getWorldRotationQuat();
    glm::vec3 worldScl = transform.getWorldScale();

    Archive transformArch;
    transformArch.set("position", worldPos);
    transformArch.set("rotation", worldRot);
    transformArch.set("scale",    worldScl);
    arch.set("transform", std::move(transformArch));

    for (const auto& [idx, comp] : components) {
        Archive cj;
        cj.set("type", cleanTypeName(typeid(*comp).name()));
        comp->serialize(cj);
        arch.append("components", std::move(cj));
    }
}

TransformComponent& Entity::getTransform() {
	return transform;
}

const TransformComponent& Entity::getTransform() const {
	return transform;
}
