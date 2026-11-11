#pragma once

#include <scene/components/Component.h>

#include <memory>

class Mesh;
class Archive;

class MeshComponent : public Component {
public:
	explicit MeshComponent(const Entity& entity) : Component(entity) {}

	// what addComponent<MeshComponent>(mesh) forwards to
	void init(std::shared_ptr<Mesh> mesh = nullptr);

	void setMesh(std::shared_ptr<Mesh> mesh);
	// nullptr if there is no mesh. 
	// The mesh is shared, the pointer stays valid as long as it is not replaced.
	Mesh* getMesh() const;

	void setPrimitive(unsigned int primitive);
	unsigned int getPrimitive() const;

	void serialize(Archive& arch) const override;
	void deserialize(const Archive& arch) override;

	bool onAttach() { return true; }
	void onDetach() {}
};
