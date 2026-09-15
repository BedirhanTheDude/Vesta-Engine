#include <controller/AssetController.h>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/MaterialComponent.h>
#include <scene/components/MeshComponent.h>

#include <renderer/Mesh.h>
#include <renderer/Texture.h>
#include <renderer/ShaderProgram.h>
#include <renderer/LoadedModel.h>

#include <core/AssetManager.h>
#include <core/Application.h>

#include <memory>

bool AssetController::loadMesh(const std::string& meshName) {
	return AssetManager::loadMesh(meshName);
}

bool AssetController::loadModel(const std::string& modelName) {
	return AssetManager::loadModel(modelName);
}

bool AssetController::loadTexture(const std::string& textureName) {
	return AssetManager::loadTexture(textureName);
}

bool AssetController::setMesh(unsigned int entityID, const std::string& meshName) {
	Entity* entity = Application::getCurrentScene()->findEntity(entityID);
	if (!entity)
		return false;

	MeshComponent* mc;
	
	mc = entity->getComponent<MeshComponent>();
	if (!mc) return false;

	std::shared_ptr<Mesh> mesh = AssetManager::getMesh(meshName);

	if (!mesh)
		return false;

	mc->setMesh(mesh);

	return true;
}

bool AssetController::setModel(unsigned int entityID, const std::string& modelName) {
	Entity* entity = Application::getCurrentScene()->findEntity(entityID);
	if (!entity)
		return false;

	MeshComponent* meshc;
	MaterialComponent* matc;

	meshc = entity->getComponent<MeshComponent>();
	if (!meshc) return false;

	LoadedModel model = AssetManager::getModel(modelName);

	if (!model.mesh)
		return false;

	meshc->setMesh(model.mesh);

	matc = entity->getComponent<MaterialComponent>();
	if (matc) matc->setMaterials(model.materials);

	return true;
}

bool AssetController::setTexture(unsigned int entityID, unsigned int groupIdx, unsigned int payloadIdx, const std::string& textureName) {
	Entity* entity = Application::getCurrentScene()->findEntity(entityID);
	if (!entity)
		return false;

	MaterialComponent* mc;

	mc = entity->getComponent<MaterialComponent>();
	if (!mc) return false;

	std::shared_ptr<Texture> tex = AssetManager::getTexture(textureName);

	if (!tex)
		return false;

	Material* mat =mc->getMaterial(groupIdx);
	
	if (!mat)
		return false;

	// payload index is essentially always 0, because there is only one diffuse texture per material
	mat->diffuseTexture = tex;
	return true;
}

std::string AssetController::getResourceName(const Payload& payload) {
	switch (payload.type) {
	case PayloadType::Mesh: {
		auto* mesh = static_cast<std::shared_ptr<Mesh>*>(payload.ptr);
		return (mesh && *mesh) ? (*mesh)->getName() : std::string();
	}
	case PayloadType::Texture: {
		auto* tex = static_cast<std::shared_ptr<Texture>*>(payload.ptr);
		return (tex && *tex) ? (*tex)->getName() : std::string();
	}
	default:
		return std::string();
	}
}