#include <scene/components/BehaviourComponent.h>

#include <scene/components/ComponentFactory.h>
#include <property/Property.h>
#include <property/PropertySerializer.h>
#include <persistence/Archive.h>

#include <cstdint>

static const void* rebasePtr(const void* p, std::uintptr_t oldBase, std::uintptr_t newBase, size_t size) noexcept {
    auto addr = reinterpret_cast<std::uintptr_t>(p);
    if (addr < oldBase || addr >= oldBase + size) return p;
    return reinterpret_cast<const void*>(newBase + (addr - oldBase));
}

void BehaviourComponent::serialize(Archive& arch) const {
    PropertySerializer::serialize(*(PropertyHolder*)this, arch);
    arch.set("_scriptStarted_", m_Started);
}

void BehaviourComponent::deserialize(const Archive& arch) {
    PropertySerializer::deserialize(*(PropertyHolder*)this, arch);
    arch.get("_scriptStarted_", m_Started);
}

void BehaviourComponent::rebaseInternalPointers(const void* oldBase, const void* newBase, size_t size) noexcept {
    const std::uintptr_t o = reinterpret_cast<std::uintptr_t>(oldBase);
    const std::uintptr_t n = reinterpret_cast<std::uintptr_t>(newBase);

    for (PropertyGroup& group : m_PropertyGroups)
        for (auto& [idx, prop] : group.properties)
            prop.ptr = const_cast<void*>(rebasePtr(prop.ptr, o, n, size));

    for (PayloadGroup& group : m_PayloadGroups)
        for (auto& [idx, payload] : group.payloads)
            payload.ptr = const_cast<void*>(rebasePtr(payload.ptr, o, n, size));
}