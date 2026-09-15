#pragma once

#include <cstdint>

class HierarchyPanel {
public:
    void show(bool* open = nullptr);

private:
    void drawNode(unsigned int eID);

    static const char* DND_ID;
    int64_t renamingEntityID = -1;
    char renameBuffer[256] = {};
};