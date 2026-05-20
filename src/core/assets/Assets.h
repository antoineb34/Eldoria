#pragma once

#include "assets/model/Model.h"
#include "filestore/FileStore.h"
#include <unordered_map>

class Assets {
public:
    explicit Assets(FileStore& store);

    const Model& getModel(int id);

    // Future: add these as you port each type
    // const Item& getItem(int id);

private:
    FileStore& store_;
    std::unordered_map<int, Model> models_;
};
