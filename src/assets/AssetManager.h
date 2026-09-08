#pragma once

#include <cstdint>

#include "animation/AnimationFrameTable.h"
#include "animation/AnimationLoader.h"
#include "cache/Cache.h"
#include "floor/FloorLoader.h"
#include "font/FontLoader.h"
#include "identity_kit/IdentityKitLoader.h"
#include "interface/WidgetLoader.h"
#include "item/ItemLoader.h"
#include "location/LocationLoader.h"
#include "map/MapLoader.h"
#include "message/MessageLoader.h"
#include "message_animation/MessageAnimationLoader.h"
#include "midi/MidiLoader.h"
#include "model/ModelLoader.h"
#include "npc/NpcLoader.h"
#include "parameter/ParameterLoader.h"
#include "sequence/SequenceLoader.h"
#include "spot_animation/SpotAnimationLoader.h"
#include "sprite/SpriteLoader.h"
#include "texture/TextureLoader.h"
#include "varbit/VarbitLoader.h"
#include "varp/VarpLoader.h"

namespace eld::asset {

class AssetManager {
public:
  explicit AssetManager(const eld::cache::Cache &cache);

  eld::animation::AnimationLoader animations;
  eld::animation::AnimationFrameTable animationFrames;
  eld::floor::FloorLoader floors;
  eld::font::FontLoader fonts;
  eld::identity_kit::IdentityKitLoader identityKits;
  eld::item::ItemLoader items;
  eld::location::LocationLoader locations;
  eld::map::MapLoader maps;
  eld::message_animation::MessageAnimationLoader messageAnimations;
  eld::message::MessageLoader messages;
  eld::midi::MidiLoader midi;
  eld::texture::TextureLoader textures;
  eld::model::ModelLoader models;
  eld::npc::NpcLoader npcs;
  eld::parameter::ParameterLoader parameters;
  eld::sequence::SequenceLoader sequences;
  eld::spot_animation::SpotAnimationLoader spotAnimations;
  eld::sprite::SpriteLoader sprites;
  eld::varbit::VarbitLoader varbits;
  eld::varp::VarpLoader varps;
  eld::interface::WidgetLoader widgets;
};

} // namespace eld::asset
