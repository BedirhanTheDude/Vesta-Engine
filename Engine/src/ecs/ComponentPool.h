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
			if (entity.isInvalid() || entity >= static_cast<uint32_t>(sparse.size()))
				return false;

			uint32_t index = sparse[entity];

			return index != INVALID_ENTITY_INDEX &&
				index < static_cast<uint32_t>(entities.size()) &&
				entities[index] == entity; // check entityhandle because swap and pop may have changed it (can't trust index)
		}

		T& add(EntityHandle entity) {
			// TODO: Add error message of adding a component to entity that already has it after proper console tooling
			assert(!has(entity) && !entity.isInvalid());

			T& ref = components.emplace_back();
			entities.push_back(entity);

			ensureSparseSize(entity);

			sparse[entity] = static_cast<uint32_t>(entities.size()) - 1;

			return ref;
		}

		T& add(EntityHandle entity, T value) {
			assert(!has(entity) && !entity.isInvalid());

			components.push_back(std::move(value));
			T& ref = components.back();
			entities.push_back(entity);

			ensureSparseSize(entity);

			sparse[entity] = static_cast<uint32_t>(entities.size()) - 1;

			return ref;
		}

		void remove(EntityHandle entity) override {
			if (!has(entity)) return;

			uint32_t denseIndex = sparse[entity];

			uint32_t last = static_cast<uint32_t>(components.size() - 1);

			if (denseIndex != last) {
				components[denseIndex] = std::move(components[last]);

				EntityHandle movedEntity = entities[last];

				entities[denseIndex] = movedEntity;
				sparse[movedEntity] = denseIndex;
			}

			components.pop_back();
			entities.pop_back();
			sparse[entity] = INVALID_ENTITY_INDEX;
		}

		T* get(EntityHandle entity) {
			if (!has(entity))
				return nullptr;

			return &components[sparse[entity]];
		}

		const T* get(EntityHandle entity) const {
			if (!has(entity))
				return nullptr;

			return &components[sparse[entity]];
		}

		void reset() override {
			components.clear();
			entities.clear();
			sparse.clear();
		}

		// IComponentPool: the caller has to check has() first, add() asserts on a duplicate
		void* addDefault(EntityHandle entity) override { return &add(entity); }
		void* getRaw(EntityHandle entity) override { return get(entity); }

	private:
		// components and entities are dense
		std::vector<T> components;
		std::vector<EntityHandle> entities; // entities[i] is the handle of the owner of components[i]
		std::vector<uint32_t> sparse; // entity handle -> dense index

		struct Iterator {
			ComponentPool* pool;
			std::size_t index;

			struct Item {
				EntityHandle entity;
				T& component;
			};

			Item operator*() const {
				return {
					pool->entities[index],
					pool->components[index]
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

			std::size_t sparseSize = sparse.size();

			if (entity >= static_cast<uint32_t>(sparseSize))
				sparse.resize(std::max(sparseSize * 2, static_cast<std::size_t>(entity.entityId + 1)), INVALID_ENTITY_INDEX);
		}

	public:
		Iterator begin() {
			return { this, 0 };
		}

		Iterator end() {
			return { this, components.size() };
		}
	};
}