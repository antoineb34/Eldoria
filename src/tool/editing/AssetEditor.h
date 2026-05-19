#pragma once
#include "assets/Assets.h"
#include <unordered_set>

class AssetEditor {
public:
    explicit AssetEditor(Assets& assets);

    void markItemDirty(int id);
    void saveItem(int id);   // encode + write back via FileStore

private:
    Assets& assets_;
    std::unordered_set<int> dirtyItems_;
};
