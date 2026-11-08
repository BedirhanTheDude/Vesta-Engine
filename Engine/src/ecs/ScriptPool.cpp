#include "ScriptPool.h"

#include <ecs/EntityHandle.h>

#include <new>
#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstddef>
#include <algorithm>

namespace ECS {
	ScriptPool::ScriptPool(const ScriptTypeInfo& info, size_t startSize)
	: m_Info(info), m_Stride(info.size), m_Capacity(startSize) {

		assert(m_Info.size > 0);
		assert(m_Info.alignment > 0);
		assert(m_Info.constructor);
		assert(m_Info.destructor);
		assert(m_Info.moveConstructor);

		if (m_Capacity > 0) {
			if (m_Capacity > SIZE_MAX / m_Stride) {
				std::printf("WARNING: Bad array length for script pool");
				m_Capacity = SIZE_MAX / m_Stride;
			}

			m_Buffer = ::operator new(
				m_Capacity * m_Stride,
				std::align_val_t(m_Info.alignment)
			);
		}
	}

	ScriptPool::~ScriptPool() {
		reset();

		if (m_Buffer) {
			::operator delete(
				m_Buffer,
				std::align_val_t(m_Info.alignment)
			);
		}
	}

	bool ScriptPool::has(EntityHandle entity) const {
		if (entity.isInvalid() || entity.entityId >= static_cast<uint32_t>(m_Sparse.size()))
			return false;

		uint32_t index = m_Sparse[entity.entityId];

		return index != INVALID_ENTITY_INDEX &&
			index < static_cast<uint32_t>(m_Entities.size()) &&
			m_Entities[index] == entity;
	}

	void* ScriptPool::add(EntityHandle entity) {
		assert(!has(entity) && !entity.isInvalid());

		if (m_Used == m_Capacity && !grow()) return nullptr; // Capacity is not enough and couldn't allocate new memory

		ensureSparseSize(entity);

		void* address = getAddress(m_Used);
		m_Info.constructor(address);

		m_Entities.push_back(entity);
		m_Sparse[entity.entityId] = static_cast<int>(m_Used);

		++m_Used;

		return address;
	}

	void* ScriptPool::addDefault(EntityHandle entity) { return add(entity); }

	void* ScriptPool::get(EntityHandle entity) {
		if (!has(entity)) return nullptr;

		return getAddress(m_Sparse[entity.entityId]);
	}

	const void* ScriptPool::get(EntityHandle entity) const {
		if (!has(entity)) return nullptr;

		return getAddress(m_Sparse[entity.entityId]);
	}

	BehaviourComponent* ScriptPool::getBehaviour(EntityHandle entity) {
		if (!has(entity)) return nullptr;

		return m_Info.getBehaviour(getAddress(m_Sparse[entity.entityId]));
	}

	const BehaviourComponent* ScriptPool::getBehaviour(EntityHandle entity) const {
		if (!has(entity)) return nullptr;

		return m_Info.getBehaviourConst(getAddress(m_Sparse[entity.entityId]));
	}

	BehaviourComponent* ScriptPool::getBehaviour(uint32_t index) {
		return m_Info.getBehaviour(getAddress(index));
	}

	const BehaviourComponent* ScriptPool::getBehaviour(uint32_t index) const {
		return m_Info.getBehaviourConst(getAddress(index));
	}

	void* ScriptPool::getRaw(EntityHandle entity) { return get(entity); }

	void ScriptPool::remove(EntityHandle entity) {
		if (!has(entity)) return;

		size_t denseIndex = m_Sparse[entity.entityId];

		size_t last = m_Used - 1;

		if (denseIndex != last) {
			void* denseAddr = getAddress(denseIndex);
			void* lastAddr = getAddress(last);

			m_Info.moveConstructor(denseAddr, lastAddr);

			EntityHandle movedEntity = m_Entities[last];

			m_Entities[denseIndex] = movedEntity;
			m_Sparse[movedEntity.entityId] = static_cast<uint32_t>(denseIndex);
		}

		--m_Used;
		m_Entities.pop_back();
		m_Sparse[entity.entityId] = INVALID_ENTITY_INDEX;
	}

	void ScriptPool::reset() {
		for (size_t i = 0; i < m_Used; ++i) {
			void* addr = getAddress(i);
			m_Info.destructor(addr);
		}

		m_Used = 0;
		m_Entities.clear();
		std::fill(m_Sparse.begin(), m_Sparse.end(), INVALID_ENTITY_INDEX);
	}

	inline void* ScriptPool::getAddress(size_t index) {
		return static_cast<std::byte*>(m_Buffer) + index * m_Stride;
	}

	inline const void* ScriptPool::getAddress(size_t index) const {
		return static_cast<const std::byte*>(m_Buffer) + index * m_Stride;
	}

	void ScriptPool::ensureSparseSize(EntityHandle entity) {
		assert(!entity.isInvalid());

		std::size_t sparseSize = m_Sparse.size();

		// Fill new positions in sparse array with UINT32_MAX
		if (entity.entityId >= static_cast<uint32_t>(sparseSize))
			m_Sparse.resize(std::max(sparseSize * 2, static_cast<std::size_t>(entity.entityId + 1)), INVALID_ENTITY_INDEX);
	}

	bool ScriptPool::grow() {
		size_t newCapacity = m_Capacity == 0 ? 8 : m_Capacity * 2;

		void* newBuffer = ::operator new(
			newCapacity * m_Stride,
			std::align_val_t(m_Info.alignment),
			std::nothrow
		);

		if (!newBuffer) return false;

		for (size_t i = 0; i < m_Used; ++i) {
			void* src = getAddress(i);
			void* dst = static_cast<std::byte*>(newBuffer) + i * m_Stride;
			
			m_Info.moveConstructor(dst, src);
		}

		::operator delete(
			m_Buffer,
			std::align_val_t(m_Info.alignment)
		);

		m_Buffer = newBuffer;
		m_Capacity = newCapacity;

		return true;
	}
}