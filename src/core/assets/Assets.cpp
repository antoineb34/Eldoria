#include "assets/Assets.h"
#include "assets/model/ModelCodec.h"

Assets::Assets(FileStore& store) : store_(store) {}

const Model& Assets::getModel(int id) {
    auto it = models_.find(id);
    if (it != models_.end()) return it->second;

    Buffer buf = store_.readGzippedFile(1, id);
    return models_.emplace(id, ModelCodec::decode(id, buf)).first->second;
}
