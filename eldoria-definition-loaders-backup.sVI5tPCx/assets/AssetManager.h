#pragma once

#include <cstdint>

#include "cache/Cache.h"
#include "animation/AnimationLoader.h"
#include "animation/AnimationFrameTable.h"
#include "repositories/FloorRepository.h"
#include "repositories/FontRepository.h"
#include "repositories/IdentityKitRepository.h"
#include "repositories/ItemRepository.h"
#include "repositories/LocationRepository.h"
#include "repositories/MapRepository.h"
#include "repositories/MessageAnimationRepository.h"
#include "repositories/MessageRepository.h"
#include "repositories/MidiRepository.h"
#include "model/ModelLoader.h"
#include "repositories/NpcRepository.h"
#include "repositories/ParameterRepository.h"
#include "sequence/SequenceLoader.h"
#include "spot_animation/SpotAnimationLoader.h"
#include "sprite/SpriteLoader.h"
#include "texture/TextureLoader.h"
#include "repositories/VarbitRepository.h"
#include "repositories/VarpRepository.h"
#include "repositories/WidgetRepository.h"

namespace eld::asset {

class AssetManager {
public:
    explicit AssetManager(const eld::cache::Cache& cache);

    eld::animation::AnimationLoader animations;
    eld::animation::AnimationFrameTable animationFrames;
    eld::floor::FloorRepository floors;
    eld::font::FontRepository fonts;
    eld::identity_kit::IdentityKitRepository identityKits;
    eld::item::ItemRepository items;
    eld::location::LocationRepository locations;
    eld::map::MapRepository maps;
    eld::message_animation::MessageAnimationRepository messageAnimations;
    eld::message::MessageRepository messages;
    eld::midi::MidiRepository midi;
    eld::texture::TextureLoader textures;
    eld::model::ModelLoader models;
    eld::npc::NpcRepository npcs;
    eld::parameter::ParameterRepository parameters;
    eld::sequence::SequenceLoader sequences;
    eld::spot_animation::SpotAnimationLoader spotAnimations;
    eld::sprite::SpriteLoader sprites;
    eld::varbit::VarbitRepository varbits;
    eld::varp::VarpRepository varps;
    eld::interface::WidgetRepository widgets;

};

}
