#pragma once

#include <string>
#include <set>
#include <map>

class Entity;

namespace EntityController {
	const std::set<unsigned int>& getAllEntityIDs();
	const std::set<unsigned int>& getEntityChildIDs(unsigned int parentID);

	const std::map<unsigned int, unsigned int>& getComponentIdxMap(unsigned int entityID);

	void setSelectedEntityID(unsigned int ID);
	void setEntityParent(unsigned int childID, unsigned int parentID);
	void unparentEntity(unsigned int ID);
	void clearSelectedEntityID();
	void renameEntity(unsigned int ID, const char* name);
	void removeEntity(unsigned int ID);

	Entity* getSelectedEntity();
	void setSelectedEntity(Entity* entity);

	int64_t getSelectedEntityID();
	std::string getEntityName(unsigned int ID);

	bool wouldCreateCycle(unsigned int childID, unsigned int newParentID);
	bool entityHasParent(unsigned int childID);

	inline int64_t selectedEntityID = -1;

	Entity* resolveEntity(unsigned int ID);
};