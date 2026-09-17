#include "AssetManager.h"

namespace eld::asset {


AssetManager::AssetManager()
    : cache_("cache"),
      animations(cache_),
      animationFrames(animations),
      floors(cache_),
      fonts(cache_),
      identityKits(cache_),
      items(cache_),
      locations(cache_),
      maps(cache_),
      messageAnimations(cache_),
      messages(cache_),
      midi(cache_),
      textures(cache_),
      models(cache_),
      npcs(cache_),
      parameters(cache_),
      sequences(cache_, animationFrames),
      spotAnimations(cache_, models, sequences),
      sprites(cache_, eld::sprite::SpriteArchive::Media),
      varbits(cache_),
      varps(cache_),
      widgets(cache_)

{
}

}