#pragma once

#include <cstdint>

class Entity;

class HierarchyPanel {
public:
    void show(bool* open = nullptr);

private:
    void drawNode(const Entity& entity, uint32_t selectedEntityID, bool* pasteEntityFlag = nullptr);

    static const char* m_DND_ID;

    int64_t m_RenamingEntityID = -1;
    char m_RenameBuffer[256] = {};
};