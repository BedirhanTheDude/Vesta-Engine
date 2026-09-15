#pragma once

#include <scene/components/Component.h>

#include <memory>
#include <vector>

class Material;
class Archive;

class MaterialComponent : public Component {
public:
    MaterialComponent(std::shared_ptr<Material> material = nullptr);

    MaterialComponent(std::vector<std::shared_ptr<Material>> mats) : materials(std::move(mats)) {}

    size_t getMaterialCount() const { return materials.size(); }
    
    Material* getMaterial(int index = 0) const;

    void setMaterial(int index, std::shared_ptr<Material> mat);
    void setMaterials(const std::vector<std::shared_ptr<Material>>& materials);

	void serialize(Archive& arch) const override;
	void deserialize(const Archive& arch) override;

    void addMaterial(std::shared_ptr<Material> mat);
    void addDefaultMaterial();

private:
    
    void registerMaterialProperties(unsigned int materialIdx);
    void registerMaterialPayloads(unsigned int materialIdx);
    void rebuildMaterialRegistration();

    std::vector<std::shared_ptr<Material>> materials;
    std::vector<std::vector<Property>> propertyCache;
};
