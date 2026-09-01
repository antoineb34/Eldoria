#include "CacheExplorer.h"

#include <exception>
#include <limits>
#include <optional>
#include <string>
#include <utility>

#include "FloorPreviewBuilder.h"
#include "FontPreviewBuilder.h"
#include "IdentityKitPreviewBuilder.h"
#include "InterfaceDumpBuilder.h"
#include "LocationPreviewBuilder.h"
#include "MapPreviewBuilder.h"
#include "NpcPreviewBuilder.h"
#include "SpotAnimationPreviewBuilder.h"

namespace eld::elforge {

void CacheExplorer::handleSelectionChanged() {
    resetAnimationPreview();

    midiPlayer_.unload();

    state_.activeAnimation.reset();
    state_.animationDumpStatus.clear();
    state_.animationDumpAllRequested = false;

    state_.activeMidi.reset();
    state_.midiExportStatus.clear();
    state_.midiPlaybackStatus.clear();
    state_.midiSeekPreviewTick = 0;
    state_.midiSeekActive = false;
    state_.activeMap.reset();
    state_.mapPreviewError.clear();
    state_.selectedMapTile.reset();
    state_.selectedMapLocIndex.reset();
    state_.activeModel.reset();
    state_.activeModelHandle.reset();
    state_.activeTexture.reset();
    state_.activeSprite.reset();
    state_.activeImage.reset();
    state_.activeFont.reset();
    state_.activeFloor.reset();
    state_.activeIdentityKit.reset();
    state_.activeLocation.reset();
    state_.activeNpc.reset();
    state_.activeItem.reset();
    state_.activeSequence.reset();
    state_.activeSpotAnimation.reset();
    state_.activeVarp.reset();
    state_.activeVarbit.reset();
    state_.activeParameter.reset();
    state_.activeMessage.reset();
    state_.activeMessageAnimation.reset();
    state_.activeInterface.reset();
    state_.activeInterfaceDump.clear();

    switch (state_.selection.type) {
        case CacheTreeNodeType::Root:
        case CacheTreeNodeType::Index:
        case CacheTreeNodeType::Archive:
        case CacheTreeNodeType::File:
        case CacheTreeNodeType::ArchiveFile:
            break;

        case CacheTreeNodeType::Animation: {
            if (
                state_.selection.fileId < 0 ||
                state_.selection.fileId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            try {
                const AnimationRelations relations(
                    animationRepository_,
                    animationFrameIndex_,
                    sequenceRepository_,
                    npcRepository_,
                    locationRepository_,
                    spotAnimationRepository_,
                    itemRepository_,
                    interfaceRepository_,
                    animationPresentationCatalog_
                );

                state_.activeAnimation =
                    relations.inspect(
                        static_cast<std::uint16_t>(
                            state_.selection.fileId
                        )
                    );
            }
            catch (const std::exception& exception) {
                state_.activeAnimation.reset();
                state_.animationDumpStatus =
                    std::string("Failed to inspect animation: ") +
                    exception.what();
            }

            break;
        }

        case CacheTreeNodeType::Midi: {
            if (
                state_.selection.fileId < 0 ||
                state_.selection.fileId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            try {
                state_.activeMidi =
                    midiRepository_.get(
                        static_cast<std::uint16_t>(
                            state_.selection.fileId
                        )
                    );

                if (midiPlayer_.isAvailable()) {
                    if (midiPlayer_.load(
                            state_.activeMidi->data.bytes
                        )) {
                        state_.midiPlaybackStatus =
                            "Ready to play";
                    }
                    else {
                        state_.midiPlaybackStatus =
                            midiPlayer_.statusMessage();
                    }
                }
                else {
                    state_.midiPlaybackStatus =
                        midiPlayer_.statusMessage();
                }
            }
            catch (const std::exception& exception) {
                const std::string message =
                    std::string("Failed to load MIDI: ") +
                    exception.what();

                state_.midiExportStatus = message;
                state_.midiPlaybackStatus = message;
            }

            break;
        }

        case CacheTreeNodeType::MapRegion: {
            if (
                state_.selection.regionId < 0 ||
                state_.selection.regionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                state_.mapPreviewError =
                    "Selected map region id is invalid";
                break;
            }

            try {
                const MapPreviewBuilder previewBuilder(
                    mapLoader_,
                    floorRepository_,
                    locationRepository_,
                    modelRepository_,
                    graphicsResources_
                );

                state_.activeMap =
                    previewBuilder.build(
                        static_cast<std::uint16_t>(
                            state_.selection.regionId
                        )
                    );

                resetMapView(
                    state_.mapPlane,
                    state_.mapYaw,
                    state_.mapPitch,
                    state_.mapDistance
                );

                state_.mapShowTerrain = true;
                state_.mapShowLocs = true;
                state_.mapViewportDirty = true;
            }
            catch (const std::exception& exception) {
                state_.activeMap.reset();
                state_.mapPreviewError =
                    exception.what();
            }

            break;
        }

        case CacheTreeNodeType::Model: {
            if (
                state_.selection.fileId < 0 ||
                state_.selection.fileId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const std::uint16_t modelId =
                static_cast<std::uint16_t>(
                    state_.selection.fileId
                );

            try {
                std::optional<eld::model::Model> model =
                    modelRepository_.find(
                        modelId
                    );

                if (model.has_value()) {
                    const eld::graphics::ModelHandle handle =
                        graphicsResources_.resolveModel(
                            modelId
                        );

                    state_.activeModel =
                        std::move(*model);

                    state_.activeModelHandle =
                        handle;
                }
            }
            catch (const std::exception&) {
                state_.activeModel.reset();
                state_.activeModelHandle.reset();
            }

            break;
        }

        case CacheTreeNodeType::Texture: {
            if (
                state_.selection.fileId < 0 ||
                state_.selection.fileId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const std::uint16_t textureId =
                static_cast<std::uint16_t>(
                    state_.selection.fileId
                );

            try {
                state_.activeTexture =
                    textureRepository_.find(
                        textureId
                    );
            }
            catch (const std::exception&) {
                state_.activeTexture.reset();
            }

            break;
        }

        case CacheTreeNodeType::Font: {
            if (
                state_.selection.archiveId != 1 ||
                state_.selection.name.empty()
            ) {
                break;
            }

            state_.activeFont =
                titleFontRepository_.find(
                    state_.selection.name
                );

            if (state_.activeFont.has_value()) {
                const FontPreviewBuilder previewBuilder;

                state_.activeImage =
                    previewBuilder.build(
                        *state_.activeFont
                    );
            }

            break;
        }

        case CacheTreeNodeType::DefinitionGroup:
            break;

        case CacheTreeNodeType::InterfaceDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    interfaceRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeInterface =
                        *definition;

                    state_.activeInterfaceDump =
                        InterfaceDumpBuilder::build(
                            *definition,
                            interfaceRepository_
                        );

                    if (
                        definition->type == 6 &&
                        definition->modelId.has_value()
                    ) {
                        try {
                            state_.activeModel =
                                modelRepository_.find(
                                    *definition->modelId
                                );

                            if (state_.activeModel.has_value()) {
                                state_.activeModelHandle =
                                    graphicsResources_.resolveModel(
                                        *definition->modelId
                                    );
                            }
                        }
                        catch (const std::exception&) {
                            state_.activeModel.reset();
                            state_.activeModelHandle.reset();
                        }
                    }

                    if (!state_.activeModelHandle.has_value()) {
                        state_.activeImage.reset();
                    }
                }
            }

            break;
        }

        case CacheTreeNodeType::MessageDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    messageRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeMessage = *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::MessageAnimationDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    messageAnimationRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeMessageAnimation =
                        *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::ParameterDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    parameterRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeParameter =
                        *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::VarpDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    varpRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeVarp = *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::VarbitDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    varbitRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeVarbit = *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::SpotAnimationDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::SpotAnimationDefinition* definition =
                spotAnimationRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeSpotAnimation =
                    *definition;

                const SpotAnimationPreviewBuilder previewBuilder;

                std::optional<eld::model::Model> preview =
                    previewBuilder.build(
                        *definition,
                        modelRepository_
                    );

                if (preview.has_value()) {
                    state_.activeModelHandle =
                        graphicsResources_.resolveModel(
                            preview->mesh
                        );

                    state_.activeModel =
                        std::move(*preview);

                    std::optional<eld::model::Model> animationSource =
                        previewBuilder.buildAnimationSource(*definition, modelRepository_);

                    if (animationSource.has_value()) {
                        animationSource_ = animationSource->mesh;
                        animationPreviewKind_ =
                            AnimationPreviewKind::SpotAnimation;

                        startAnimationPreview(
                            definition->sequenceId
                        );
                    }
                }
            }

            break;
        }

        case CacheTreeNodeType::SequenceDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::SequenceDefinition* definition =
                sequenceRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeSequence =
                    *definition;
            }

            break;
        }

        case CacheTreeNodeType::ItemDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::ItemDefinition* definition =
                itemRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeItem = *definition;
                showItemInventoryPreview();
            }

            break;
        }

        case CacheTreeNodeType::NpcDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::NpcDefinition* definition =
                npcRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeNpc =
                    *definition;

                const NpcPreviewBuilder previewBuilder;

                std::optional<eld::model::Model> preview =
                    previewBuilder.build(
                        *definition,
                        modelRepository_
                    );

                if (preview.has_value()) {
                    state_.activeModelHandle =
                        graphicsResources_.resolveModel(
                            preview->mesh
                        );

                    state_.activeModel =
                        std::move(*preview);

                    animationSource_ =
                        state_.activeModel->mesh;

                    animationPreviewKind_ =
                        AnimationPreviewKind::Npc;

                    const std::optional<std::uint16_t>
                        initialSequence =
                            definition->idleAnimationId.has_value()
                                ? definition->idleAnimationId
                                : definition->walkAnimationId;

                    startAnimationPreview(
                        initialSequence
                    );
                }
            }

            break;
        }

        case CacheTreeNodeType::LocationDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::LocationDefinition* definition =
                locationRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeLocation =
                    *definition;

                const LocationPreviewBuilder previewBuilder;

                std::optional<eld::model::Model> preview =
                    previewBuilder.build(
                        *definition,
                        modelRepository_
                    );

                if (preview.has_value()) {
                    state_.activeModelHandle =
                        graphicsResources_.resolveModel(
                            preview->mesh
                        );

                    state_.activeModel =
                        std::move(*preview);

                    std::optional<eld::model::Model> animationSource =
                        previewBuilder.buildAnimationSource(*definition, modelRepository_);

                    if (animationSource.has_value()) {
                        animationSource_ = animationSource->mesh;
                        animationPreviewKind_ =
                            AnimationPreviewKind::Location;

                        startAnimationPreview(
                            definition->animationId
                        );
                    }
                }
            }

            break;
        }

        case CacheTreeNodeType::IdentityKitDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::IdentityKitDefinition* definition =
                identityKitRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeIdentityKit =
                    *definition;

                const IdentityKitPreviewBuilder previewBuilder;

                std::optional<eld::model::Model> preview =
                    previewBuilder.build(
                        *definition,
                        modelRepository_
                    );

                if (preview.has_value()) {
                    state_.activeModelHandle =
                        graphicsResources_.resolveModel(
                            preview->mesh
                        );

                    state_.activeModel =
                        std::move(*preview);
                }
            }

            break;
        }

        case CacheTreeNodeType::FloorDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::FloorDefinition* floor =
                floorRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (floor != nullptr) {
                state_.activeFloor =
                    *floor;

                if (floor->textureId.has_value()) {
                    state_.activeTexture =
                        textureRepository_.find(
                            *floor->textureId
                        );
                }

                if (
                    !state_.activeTexture.has_value() &&
                    (
                        floor->rgb.has_value() ||
                        floor->secondaryRgb.has_value()
                    )
                ) {
                    state_.activeImage =
                        FloorPreviewBuilder{}.build(
                            *floor
                        );
                }
            }

            break;
        }

        case CacheTreeNodeType::Image: {
            if (
                state_.selection.archiveId == 1 &&
                !state_.selection.name.empty()
            ) {
                state_.activeImage =
                    titleJpegRepository_.find(
                        state_.selection.name
                    );
            }

            break;
        }

        case CacheTreeNodeType::Sprite:
        case CacheTreeNodeType::SpriteFrame: {
            const int selectedFrame =
                state_.selection.frameId >= 0
                    ? state_.selection.frameId
                    : 0;

            if (
                selectedFrame >
                std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const auto frameId =
                static_cast<std::uint16_t>(
                    selectedFrame
                );

            if (
                state_.selection.archiveId == 1 &&
                !state_.selection.name.empty()
            ) {
                state_.activeSprite =
                    titleSpriteRepository_.find(
                        state_.selection.name,
                        frameId
                    );
            }
            else if (
                state_.selection.archiveId == 4 &&
                state_.selection.fileId >= 0 &&
                state_.selection.fileId <=
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                state_.activeSprite =
                    mediaSpriteRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.fileId
                        ),
                        frameId
                    );
            }

            break;
        }
    }
}

}
