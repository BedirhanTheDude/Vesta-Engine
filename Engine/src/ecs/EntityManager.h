#pragma once

#include "EntityHandle.h"

#include <vector>
#include <cstdint>

namespace ECS {
    class EntityManager
    {
    public:
        EntityHandle create();
        void destroy(EntityHandle entity);

        bool isAlive(EntityHandle entity) const;
        uint32_t getGeneration(uint32_t id) const;

        void reset();
    private:
        std::vector<uint8_t> alive; // boolean
        std::vector<uint32_t> generations; // current generation per index, bumped on destroy
        std::vector<uint32_t> freeList; // indices freed by destroy(), available for reuse by create()
    };
}