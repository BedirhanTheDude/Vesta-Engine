#pragma once

#include <ecs/IComponentPool.h>
#include <scene/components/ComponentFactory.h>
#include <scene/components/BehaviourComponent.h>

#include <vector>
#include <cstdint>

class BehaviourComponent;

namespace ECS {
	class ScriptPool {
	public:
		explicit ScriptPool(const ScriptTypeInfo& info, size_t startSize = 0);
		~ScriptPool();

		inline bool has(EntityHandle entity) const {
			if (const QueueEntry* q = latestQueued(entity)) return q->type == QUEUE_ADD;
			return hasInBuffer(entity);
		}

		void* add(EntityHandle entity);
		void remove(EntityHandle entity);
		void* get(EntityHandle entity);
		void reset();
		
		BehaviourComponent* getBehaviour(EntityHandle entity);
		BehaviourComponent* asBehaviour(void* mem);

		void setIsTicking(bool value) { m_IsTicking = value; }
		void processQueue();

	private:
		static constexpr size_t QUEUE_BLOCK_SLOTS = 16; // scripts per Queue block
		static constexpr uint8_t QUEUE_ADD = 0, QUEUE_REMOVE = 1, QUEUE_CANCELLED = 2;

		struct QueueEntry {
			EntityHandle entity;
			uint32_t slot;
			uint8_t type; // ADD: 0, REMOVE: 1, CANCELLED: 2
		};

		ScriptTypeInfo m_Info;

		bool m_IsTicking = false;

		void* m_Buffer = nullptr;

		std::vector<void*> m_QueueBlocks; // vector of addresses of Queue blocks, blocks are never reallocated
		// the vector itself can reallocate, we don't care about that, this is so that pointers to scripts stay valid in within a frame
		// I wanted to make the original m_Buffer paged like this but it would introduce gaps in memory and hurt cache locality as time passed

		size_t m_Capacity = 0;
		size_t m_Used = 0;
		size_t m_QueueUsed = 0;
		size_t m_Stride = 0;

		std::vector<EntityHandle> m_Entities;
		std::vector<uint32_t> m_Sparse;
		std::vector<QueueEntry> m_Queue;

		std::vector<uint32_t> m_QueueSparse; // entityId -> index into m_Queue of that entity's LATEST entry

		void pushEntry(const QueueEntry& entry);

		void* addToBuffer(EntityHandle entity);
		void removeFromBuffer(EntityHandle entity);

		// for iterator
		BehaviourComponent* getBehaviour(uint32_t index);

		inline void* ScriptPool::getQueueSlot(size_t index) {
			return static_cast<std::byte*>(m_QueueBlocks[index / QUEUE_BLOCK_SLOTS])
				+ (index % QUEUE_BLOCK_SLOTS) * m_Stride;
		}

		inline void* ScriptPool::getAddress(void* buf, size_t index) {
			return static_cast<std::byte*>(buf) + index * m_Stride;
		}

		inline const QueueEntry* latestQueued(EntityHandle e) const {
			if (m_Queue.empty() || e.entityId >= m_QueueSparse.size()) return nullptr;
			uint32_t i = m_QueueSparse[e.entityId];
			if (i == INVALID_ENTITY_INDEX || i >= m_Queue.size() || !(m_Queue[i].entity == e)) return nullptr; // also rejects old generations
			return &m_Queue[i];
		}

		bool grow(void** buf, size_t used, size_t* capacity, size_t target = 0);
		void ensureSparseSize(std::vector<uint32_t>& sparse, EntityHandle entity);

		inline bool bufferOwnsSlot(const void* buf, size_t count, const void* mem) const {
			if (!buf) return false;
			auto addr = reinterpret_cast<std::uintptr_t>(mem);
			auto base = reinterpret_cast<std::uintptr_t>(buf);
			if (addr < base || addr >= base + count * m_Stride) return false;
			return (addr - base) % m_Stride == 0;
		}

		inline bool hasInBuffer(EntityHandle entity) const {
			if (entity.isInvalid() || entity.entityId >= static_cast<uint32_t>(m_Sparse.size()))
				return false;

			uint32_t index = m_Sparse[entity.entityId];

			return index != INVALID_ENTITY_INDEX &&
				index < static_cast<uint32_t>(m_Entities.size()) &&
				m_Entities[index] == entity;
		}

		// does NOT check scripts removed from the queue mid frame
		bool queueOwnsSlot(const void* mem) const;

		struct Iterator {
			ScriptPool* pool;
			std::size_t index;

			struct Item {
				EntityHandle entity;
				BehaviourComponent& component;
			};

			Item operator*() const {
				return {
					pool->m_Entities[index],
					*pool->getBehaviour(index)
				};
			}

			Iterator& operator++() {
				++index;
				return *this;
			}

			bool operator!=(const Iterator& other) const {
				return pool != other.pool || index != other.index;
			}
		};

	public:
		Iterator begin() { return { this, 0 }; }
		Iterator end() { return { this, m_Used }; }
	};
}