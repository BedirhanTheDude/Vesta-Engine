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

		for (void* block : m_QueueBlocks) {
			::operator delete(
				block,
				std::align_val_t(m_Info.alignment)
			);
		}
	}

	void* ScriptPool::add(EntityHandle entity) {
		assert(!has(entity) && !entity.isInvalid());

		if (!m_IsTicking) return addToBuffer(entity);

		if (m_QueueUsed == m_QueueBlocks.size() * QUEUE_BLOCK_SLOTS)
			m_QueueBlocks.push_back(::operator new(QUEUE_BLOCK_SLOTS * m_Stride, std::align_val_t(m_Info.alignment)));

		uint32_t slot = static_cast<uint32_t>(m_QueueUsed);
		void* addr = getQueueSlot(slot);
		m_Info.constructor(addr);

		++m_QueueUsed;

		pushEntry({ entity, slot, QUEUE_ADD });
		return addr;
	}

	void* ScriptPool::addToBuffer(EntityHandle entity) {
		if (hasInBuffer(entity)) return nullptr;

		if (m_Used >= m_Capacity && !grow(&m_Buffer, m_Used, &m_Capacity)) return nullptr; // Capacity is not enough and couldn't allocate new memory

		ensureSparseSize(m_Sparse, entity);

		void* address = getAddress(m_Buffer, m_Used);
		m_Info.constructor(address);

		m_Entities.push_back(entity);
		m_Sparse[entity.entityId] = static_cast<uint32_t>(m_Used);

		++m_Used;

		return address;
	}

	void* ScriptPool::get(EntityHandle entity) {
		if (const QueueEntry* q = latestQueued(entity))
			return q->type == QUEUE_ADD ? getQueueSlot(q->slot) : nullptr;
		return hasInBuffer(entity) ? getAddress(m_Buffer, m_Sparse[entity.entityId]) : nullptr;
	}

	BehaviourComponent* ScriptPool::getBehaviour(EntityHandle entity) {
		void* mem = get(entity);
		return mem ? m_Info.getBehaviour(mem) : nullptr;
	}

	BehaviourComponent* ScriptPool::asBehaviour(void* mem) {
		if (!mem) return nullptr;
		assert(bufferOwnsSlot(m_Buffer, m_Used, mem) || queueOwnsSlot(mem));
		return m_Info.getBehaviour(mem);
	}

	BehaviourComponent* ScriptPool::getBehaviour(uint32_t index) {
		return m_Info.getBehaviour(getAddress(m_Buffer, index));
	}

	void ScriptPool::pushEntry(const QueueEntry& entry) {
		ensureSparseSize(m_QueueSparse, entry.entity);
		m_QueueSparse[entry.entity.entityId] = static_cast<uint32_t>(m_Queue.size());
		m_Queue.push_back(entry);
	}

	void ScriptPool::remove(EntityHandle entity) {
		if (!m_IsTicking) { removeFromBuffer(entity); return; }

		if (!has(entity)) return;

		if (const QueueEntry* q = latestQueued(entity)) {
			uint32_t i = m_QueueSparse[entity.entityId];
			m_Queue[i].type = QUEUE_CANCELLED;

			return;
		}

		pushEntry({ entity, 0, QUEUE_REMOVE }); // scripts are ticking and this script is in m_Buffer
	}

	void ScriptPool::removeFromBuffer(EntityHandle entity) {
		if (!hasInBuffer(entity)) return;

		size_t denseIndex = m_Sparse[entity.entityId];

		size_t last = m_Used - 1;

		void* denseAddr = getAddress(m_Buffer, denseIndex);
		m_Info.destructor(denseAddr);

		if (denseIndex != last) {
			void* lastAddr = getAddress(m_Buffer, last);

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
			void* addr = getAddress(m_Buffer, i);
			m_Info.destructor(addr);
		}

		for (size_t i = 0; i < m_QueueUsed; ++i) {
			void* addr = getQueueSlot(i);
			m_Info.destructor(addr);
		}

		m_Used = 0;
		m_QueueUsed = 0;
		m_Entities.clear();
		m_Queue.clear();
		std::fill(m_Sparse.begin(), m_Sparse.end(), INVALID_ENTITY_INDEX);
		std::fill(m_QueueSparse.begin(), m_QueueSparse.end(), INVALID_ENTITY_INDEX);
	}

	void ScriptPool::processQueue() {
		bool discardAdds = m_Used + m_QueueUsed > m_Capacity
			&& !grow(&m_Buffer, m_Used, &m_Capacity, m_Used + m_QueueUsed);

		for (const QueueEntry& q : m_Queue) {
			switch (q.type) {
			case QUEUE_REMOVE:
				removeFromBuffer(q.entity);
				break;
			case QUEUE_ADD:
				if (discardAdds) { m_Info.destructor(getQueueSlot(q.slot)); break; }
				ensureSparseSize(m_Sparse, q.entity);
				m_Info.moveConstructor(getAddress(m_Buffer, m_Used), getQueueSlot(q.slot));
				m_Entities.push_back(q.entity);
				m_Sparse[q.entity.entityId] = static_cast<uint32_t>(m_Used++);
				break;
			case QUEUE_CANCELLED:
				m_Info.destructor(getQueueSlot(q.slot));
				break;
			default:
				assert(false);
			}
			m_QueueSparse[q.entity.entityId] = INVALID_ENTITY_INDEX;
		}

		m_QueueUsed = 0;
		m_Queue.clear();
	}

	bool ScriptPool::queueOwnsSlot(const void* mem) const {
		size_t checkedCount = 0;
		for (size_t i = 0; i < m_QueueBlocks.size(); ++i) {
			size_t usedCount = m_QueueUsed > checkedCount + QUEUE_BLOCK_SLOTS ?
				QUEUE_BLOCK_SLOTS : m_QueueUsed - checkedCount;

			if (bufferOwnsSlot(m_QueueBlocks[i], usedCount, mem)) return true;

			checkedCount += QUEUE_BLOCK_SLOTS;
		}

		return false;
	}

	void ScriptPool::ensureSparseSize(std::vector<uint32_t>& sparse, EntityHandle entity) {
		assert(!entity.isInvalid());

		std::size_t sparseSize = sparse.size();

		// Fill new positions in sparse array with UINT32_MAX
		if (entity.entityId >= static_cast<uint32_t>(sparseSize))
			sparse.resize(std::max(sparseSize * 2, static_cast<std::size_t>(entity.entityId + 1)), INVALID_ENTITY_INDEX);
	}

	bool ScriptPool::grow(void** buf, size_t used, size_t* capacity, size_t target) {
		if (!buf || !capacity) return false;

		size_t newCapacity = *capacity == 0 ? 8 : *capacity * 2;
		if (target > newCapacity) newCapacity = target;

		void* newBuffer = ::operator new(
			newCapacity * m_Stride,
			std::align_val_t(m_Info.alignment),
			std::nothrow
		);

		if (!newBuffer) return false;

		for (size_t i = 0; i < used; ++i) {
			void* src = getAddress(*buf, i);
			void* dst = static_cast<std::byte*>(newBuffer) + i * m_Stride;
			
			m_Info.moveConstructor(dst, src);
		}

		::operator delete(
			*buf,
			std::align_val_t(m_Info.alignment)
		);

		*buf = newBuffer;
		*capacity = newCapacity;

		return true;
	}
}