#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <scene/Entity.h>


namespace EntityController {
	std::vector<unsigned int> getAllEntityIDs();
	std::vector<unsigned int> getEntityChildIDs(unsigned int parentID);

	void setSelectedEntityID(unsigned int ID);
	void setEntityParent(unsigned int childID, unsigned int parentID);
	void unparentEntity(unsigned int ID);
	void clearSelectedEntityID();
	void renameEntity(unsigned int ID, const char* name);
	void removeEntity(unsigned int ID);

	bool canPasteEntity();
	bool copyEntity(unsigned int ID);
	void pasteEntity();

	Entity getSelectedEntity();
	// selecting a dead or invalid entity clears the selection
	void setSelectedEntity(const Entity& entity);

	// UINT32_MAX when nothing is selected
	uint32_t getSelectedEntityID();
	std::string getEntityName(unsigned int ID);

	bool isEntityAlive(const Entity& entity);
	bool wouldCreateCycle(unsigned int childID, unsigned int newParentID);
	bool entityHasParent(unsigned int childID);

	// the invalid Entity if there is no live entity with this ID
	Entity resolveEntity(unsigned int ID);
};
