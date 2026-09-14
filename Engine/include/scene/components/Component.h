#pragma once

#include <persistance/Serializable.h>
#include <property/Property.h>
#include <property/CallbackProperty.h>
#include <property/Payload.h>

#include <string>
#include <unordered_map>
#include <typeinfo>

// DO NOT CALL THIS YOURSELF
// Defined out-of-line in ComponentFactory.cpp so the static name map lives in ONE
// module (Engine.dll). As an inline header function each module (exe/Editor/scripts.dll)
// got its own copy: REGISTER populated Engine.dll's map, but the UI read its own empty one.
std::string __componentTypeUIDToString(unsigned int UID, bool set = false, const std::string& name = "");

template<typename T>
struct ComponentTypeInfo {
	static inline std::string name = "";
};

constexpr static unsigned int hashComponentName(const char* str) {
	unsigned int hash = 2166136261u;
	while (*str) {
		hash ^= static_cast<unsigned int>(*str++);
		hash *= 16777619u;
	}
	return hash;
}

// MSVC type_info::name() yields e.g. "class MeshComponent"; strip the tag prefix
// so the result matches the registered name string ("MeshComponent").
inline std::string cleanComponentTypeName(const char* raw) {
	std::string s = raw;
	if (s.rfind("class ", 0) == 0)  return s.substr(6);
	if (s.rfind("struct ", 0) == 0) return s.substr(7);
	return s;
}

// Derives the name from the compiler's RTTI rather than ComponentTypeInfo<T>::name,
// so the UID is identical in any module that instantiates it — the host exe AND a
// runtime-compiled script DLL — without relying on REGISTER having run in that module.
// The hashed string is still "MeshComponent", so the value is unchanged and stays
// consistent with componentNameToUID().
template<typename T>
unsigned int componentTypeUID() {
	static unsigned int id = hashComponentName(cleanComponentTypeName(typeid(T).name()).c_str());
	return id;
}

inline std::string componentTypeUIDToString(unsigned int UID) {
	std::string name = __componentTypeUIDToString(UID);
	return name == "" ? "<unkown_component>" : name;
}

// UID for a component known only by its registered name (e.g. "MeshComponent").
// Matches componentTypeUID<T>() because both hash the same registered name string.
inline unsigned int componentNameToUID(const std::string& name) {
	return hashComponentName(name.c_str());
}

class Entity;

class Component : public Serializable, public PropertyHolder, public CallbackPropertyHolder, public PayloadHolder {
	friend class Entity;
public:
	virtual ~Component() = default;

	Entity* getEntity() const { return owner; }

	virtual bool onAttach() { return true; }
	virtual void onDetach() {}

	void serialize(Archive& arch) const = 0;
	void deserialize(const Archive& arch) = 0;

	unsigned int getIdx() const { return index; }
	unsigned int getTypeUID() const { return typeUID; }

protected:
	unsigned int index;
	unsigned int typeUID;

	void setIdx(unsigned int idx) { index = idx; }
	void setTypeUID(unsigned int UID) { typeUID = UID; }

	Entity* owner = nullptr;
};
