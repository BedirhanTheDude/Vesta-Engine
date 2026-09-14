#pragma once

#include <scene/components/Component.h>

#include <memory>

class Mesh;
class Archive;

class MeshComponent : public Component {
public:
	MeshComponent(std::shared_ptr<Mesh> mesh = nullptr);

	void setMesh(std::shared_ptr<Mesh> mesh);
	Mesh* getMesh();
	const Mesh* getMesh() const;

	void serialize(Archive& arch) const override;
	void deserialize(const Archive& arch) override;
private:
	std::shared_ptr<Mesh> mesh;

	unsigned int primitive = 0;
};