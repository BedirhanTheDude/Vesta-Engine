#pragma once

#include <ecs/IComponentPool.h>
#include <scene/components/ComponentFactory.h>
#include <scene/components/BehaviourComponent.h>

#include <vector>
#include <cstdint>

class BehaviourComponent;

namespace ECS {
	class ScriptPool : public IComponentPool {
	public:
		explicit ScriptPool(const ScriptTypeInfo& info, size_t startSize = 0);
		~ScriptPool() override;

		bool has(EntityHandle entity) const override;
		void* addDefault(EntityHandle entity) override;
		void* getRaw(EntityHandle entity) override;
		void remove(EntityHandle entity) override;
		void reset() override;

		void* add(EntityHandle entity);
		void* get(EntityHandle entity);
		const void* get(EntityHandle entity) const;
		BehaviourComponent* getBehaviour(EntityHandle entity);
		const BehaviourComponent* getBehaviour(EntityHandle entity) const;

	private:
		ScriptTypeInfo m_Info;

		void* m_Buffer = nullptr;
		
		size_t m_Capacity = 0;
		size_t m_Used = 0;
		size_t m_Stride = 0;

		std::vector<EntityHandle> m_Entities;
		std::vector<uint32_t> m_Sparse;

		// for iterator
		BehaviourComponent* getBehaviour(uint32_t index);
		const BehaviourComponent* getBehaviour(uint32_t index) const;

		inline void* getAddress(size_t index);
		inline const void* getAddress(size_t index) const;
		void ensureSparseSize(EntityHandle entity);
		bool grow();

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