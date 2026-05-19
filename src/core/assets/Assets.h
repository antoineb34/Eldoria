#pragma once
#include "assets/item/Item.h"
#include "assets/model/Model.h"
#include "filestore/FileStore.h"
#include <unordered_map>

class Assets {
public:
    explicit Assets(FileStore& store);

    const Item&  getItem(int id);
    const Model& getModel(int id);

private:
    FileStore& store_;
    std::unordered_map<int, Item>  items_;
    std::unordered_map<int, Model> models_;
};
