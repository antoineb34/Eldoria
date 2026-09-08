#pragma once

#include <cstdint>

#include "cache/Cache.h"
#include "animation/AnimationPipeline.h"
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
#include "model/ModelPipeline.h"
#include "repositories/NpcRepository.h"
#include "repositories/ParameterRepository.h"
#include "sequence/SequencePipeline.h"
#include "repositories/SpotAnimationRepository.h"
#include "sprite/SpritePipeline.h"
#include "texture/TexturePipeline.h"
#include "repositories/VarbitRepository.h"
#include "repositories/VarpRepository.h"
#include "repositories/WidgetRepository.h"

namespace eld::asset {

class AssetManager {
public:
    explicit AssetManager(const eld::cache::Cache& cache);

    eld::animation::AnimationPipeline animations;
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
    eld::texture::TexturePipeline textures;
    eld::model::ModelPipeline models;
    eld::npc::NpcRepository npcs;
    eld::parameter::ParameterRepository parameters;
    eld::sequence::SequencePipeline sequences;
    eld::spot_animation::SpotAnimationRepository spotAnimations;
    eld::sprite::SpritePipeline sprites;
    eld::varbit::VarbitRepository varbits;
    eld::varp::VarpRepository varps;
    eld::interface::WidgetRepository widgets;

};

}
