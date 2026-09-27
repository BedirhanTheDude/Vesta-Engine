#pragma once

#include <scene/components/Component.h>

#include <memory>
#include <vector>

class Material;
class Archive;

class MaterialComponent : public Component {
public:
    explicit MaterialComponent(const Entity& entity) : Component(entity) {}

    // what addComponent<MaterialComponent>(material) forwards to, nullptr gives a default material
    void init(std::shared_ptr<Material> material = nullptr);
    void init(std::vector<std::shared_ptr<Material>> mats);

    size_t getMaterialCount() const;

    // nullptr if index is out of range.
    // the material is shared, the pointer stays valid as long as it is not replaced.
    Material* getMaterial(int index = 0) const;

    void setMaterial(int index, std::shared_ptr<Material> mat);
    void setMaterials(const std::vector<std::shared_ptr<Material>>& materials);

    void serialize(Archive& arch) const;
    void deserialize(const Archive& arch);

    void addMaterial(std::shared_ptr<Material> mat);
    void addDefaultMaterial();

    // a component without any material would draw nothing, so it gets a default one
    bool onAttach();
    void onDetach() {}
};
