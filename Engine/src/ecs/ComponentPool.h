#pragma once

#include "EntityHandle.h"
#include "IComponentPool.h"

#include <utility>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>

namespace ECS {
	template<typename T>
	class ComponentPool : public IComponentPool {
	public:
		bool has(EntityHandle entity) const override {
			if (entity.isInvalid() || entity >= static_cast<uint32_t>(m_Sparse.size()))
				return false;

			uint32_t index = m_Sparse[entity];

			return index != INVALID_ENTITY_INDEX &&
				index < static_cast<uint32_t>(m_Entities.size()) &&
				m_Entities[index] == entity; // check entityhandle because swap and pop may have changed it (can't trust index)
		}

		T& add(EntityHandle entity) {
			// TODO: Add error message of adding a component to entity that already has it after proper console tooling
			assert(!has(entity) && !entity.isInvalid());

			T& ref = m_Components.emplace_back();
			m_Entities.push_back(entity);

			ensureSparseSize(entity);

			m_Sparse[entity] = static_cast<uint32_t>(m_Entities.size()) - 1;

			return ref;
		}

		T& add(EntityHandle entity, T value) {
			assert(!has(entity) && !entity.isInvalid());

			m_Components.push_back(std::move(value));
			T& ref = m_Components.back();
			m_Entities.push_back(entity);

			ensureSparseSize(entity);

			m_Sparse[entity] = static_cast<uint32_t>(m_Entities.size()) - 1;

			return ref;
		}

		void remove(EntityHandle entity) override {
			if (!has(entity)) return;

			uint32_t denseIndex = m_Sparse[entity];

			uint32_t last = static_cast<uint32_t>(m_Components.size() - 1);

			if (denseIndex != last) {
				m_Components[denseIndex] = std::move(m_Components[last]);

				EntityHandle movedEntity = m_Entities[last];

				m_Entities[denseIndex] = movedEntity;
				m_Sparse[movedEntity] = denseIndex;
			}

			m_Components.pop_back();
			m_Entities.pop_back();
			m_Sparse[entity] = INVALID_ENTITY_INDEX;
		}

		T* get(EntityHandle entity) {
			if (!has(entity))
				return nullptr;

			return &m_Components[m_Sparse[entity]];
		}

		const T* get(EntityHandle entity) const {
			if (!has(entity))
				return nullptr;

			return &m_Components[m_Sparse[entity]];
		}

		void reset() override {
			m_Components.clear();
			m_Entities.clear();
			m_Sparse.clear();
		}

		// IComponentPool: the caller has to check has() first, add() asserts on a duplicate
		void* addDefault(EntityHandle entity) override { return &add(entity); }
		void* getRaw(EntityHandle entity) override { return get(entity); }

	private:
		// components and entities are dense
		std::vector<T> m_Components;
		std::vector<EntityHandle> m_Entities; // entities[i] is the handle of the owner of components[i]
		std::vector<uint32_t> m_Sparse; // entity handle -> dense index

		struct Iterator {
			ComponentPool* pool;
			std::size_t index;

			struct Item {
				EntityHandle entity;
				T& component;
			};

			Item operator*() const {
				return {
					pool->m_Entities[index],
					pool->m_Components[index]
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

		void ensureSparseSize(EntityHandle entity) {
			assert(!entity.isInvalid());

			std::size_t sparseSize = m_Sparse.size();

			if (entity >= static_cast<uint32_t>(sparseSize))
				m_Sparse.resize(std::max(sparseSize * 2, static_cast<std::size_t>(entity.entityId + 1)), INVALID_ENTITY_INDEX);
		}

	public:
		Iterator begin() {
			return { this, 0 };
		}

		Iterator end() {
			return { this, m_Components.size() };
		}
	};
}