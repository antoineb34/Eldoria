#include "AssetManager.h"

namespace eld::asset {


AssetManager::AssetManager(const eld::cache::Cache& cache)
    : animations(cache),
      animationFrames(animations),
      floors(cache),
      fonts(cache),
      identityKits(cache),
      items(cache),
      locations(cache),
      maps(cache),
      messageAnimations(cache),
      messages(cache),
      midi(cache),
      textures(cache),
      models(cache, textures),
      npcs(cache),
      parameters(cache),
      sequences(cache, animationFrames),
      spotAnimations(cache, models, sequences),
      sprites(cache, eld::sprite::SpriteArchive::Media),
      varbits(cache),
      varps(cache),
      widgets(cache) {
}


}
