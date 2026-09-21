#pragma once

#include <string>
#include <typeinfo>

// DO NOT CALL THIS YOURSELF
// Defined out-of-line in ComponentFactory.cpp so the static name map lives in ONE
// module (Engine.dll)
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

// clean the name MSVC type_info::name() returns
inline std::string cleanComponentTypeName(const char* raw) {
	std::string s = raw;
	if (s.rfind("class ", 0) == 0)  return s.substr(6);
	if (s.rfind("struct ", 0) == 0) return s.substr(7);
	return s;
}

// derives the name from the compiler's RTTI
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
