#include "editing/AssetEditor.h"

AssetEditor::AssetEditor(Assets& assets) : assets_(assets) {}

void AssetEditor::markItemDirty(int id) { dirtyItems_.insert(id); }

void AssetEditor::saveItem(int id) {
    // TODO: encode item, write bytes back via FileStore
    dirtyItems_.erase(id);
}
