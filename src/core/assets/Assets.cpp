#include "assets/Assets.h"
#include "assets/item/ItemCodec.h"
#include "assets/model/ModelCodec.h"

Assets::Assets(FileStore& store) : store_(store) {}

const Item& Assets::getItem(int id) {
    auto it = items_.find(id);
    if (it != items_.end()) return it->second;
    Buffer buf = store_.readFile(/*archive*/ 0, id); // adjust archive id
    return items_.emplace(id, ItemCodec::decode(buf)).first->second;
}

const Model& Assets::getModel(int id) {
    auto it = models_.find(id);
    if (it != models_.end()) return it->second;
    Buffer buf = store_.readFile(/*archive*/ 1, id); // adjust archive id
    return models_.emplace(id, ModelCodec::decode(buf)).first->second;
}
