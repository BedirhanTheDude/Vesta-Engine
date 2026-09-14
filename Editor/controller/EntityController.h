#pragma once

#include <string>
#include <set>
#include <map>

class Entity;

class EntityController {
	friend class ComponentController;
	friend class TransformController;
public:
	static const std::set<unsigned int>& getAllEntityIDs();
	static const std::set<unsigned int>& getEntityChildIDs(unsigned int parentID);

	static const std::map<unsigned int, unsigned int>& getComponentIdxMap(unsigned int entityID);

	static void setSelectedEntityID(unsigned int ID);
	static void setEntityParent(unsigned int childID, unsigned int parentID);
	static void unparentEntity(unsigned int ID);
	static void clearSelectedEntityID();
	static void renameEntity(unsigned int ID, const char* name);
	static void removeEntity(unsigned int ID);

	static Entity* getSelectedEntity();
	static void setSelectedEntity(Entity* entity);

	static int64_t getSelectedEntityID();
	static std::string getEntityName(unsigned int ID);

	static bool wouldCreateCycle(unsigned int childID, unsigned int newParentID);
	static bool entityHasParent(unsigned int childID);
private:
	static int64_t selectedEntityID;

	static Entity* resolveEntity(unsigned int ID);
};