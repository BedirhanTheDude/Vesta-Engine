#pragma once

#include <cstdint>

class Entity;

class HierarchyPanel {
public:
    void show(bool* open = nullptr);

private:
    void drawNode(
        const Entity& entity,
        uint32_t selectedEntityID
    );

    static const char* DND_ID;

    int64_t renamingEntityID = -1;
    char renameBuffer[256] = {};
};