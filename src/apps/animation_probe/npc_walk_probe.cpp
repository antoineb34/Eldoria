#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "animation/AnimationFrameIndex.h"
#include "animation/AnimationRepository.h"
#include "animation/AnimationPlayer.h"
#include "animation/ModelAnimator.h"
#include "cache/Cache.h"
#include "cache/Index.h"
#include "definition/DefinitionRepository.h"
#include "definition/npc/NpcRepository.h"
#include "definition/sequence/SequenceRepository.h"
#include "model/ModelRepository.h"
#include "sdl/SdlContext.h"
#include "GraphicsResources.h"
#include "texture/TextureRepository.h"
#include "backend/software/SoftwareRenderBackend.h"
#include "camera/Camera.h"
#include "scene/Transform.h"

namespace {

struct Choice {
    const char* label = "";
    std::optional<std::uint16_t> sequenceId;
};

struct ViewState {
    std::size_t choice = 1;
    float yaw = 0.55f;
    float pitch = -0.30f;
};

std::optional<eld::model::ModelMesh> buildNpcMesh(
    const eld::definition::NpcDefinition& npc,
    const eld::model::ModelRepository& models
) {
    if (npc.modelIds.empty()) {
        return std::nullopt;
    }

    eld::model::ModelMesh merged;

    for (const std::uint16_t modelId : npc.modelIds) {
        const auto part = models.find(modelId);

        if (!part.has_value()) {
            return std::nullopt;
        }

        const std::uint32_t vertexOffset =
            static_cast<std::uint32_t>(
                merged.vertices.size()
            );

        merged.vertices.insert(
            merged.vertices.end(),
            part->mesh.vertices.begin(),
            part->mesh.vertices.end()
        );

        for (const eld::model::Face& source : part->mesh.faces) {
            eld::model::Face face = source;

            face.a += vertexOffset;
            face.b += vertexOffset;
            face.c += vertexOffset;

            merged.faces.push_back(
                std::move(face)
            );
        }
    }

    return merged;
}

struct DisplayFit {
    float centerX = 0.0f;
    float centerY = 0.0f;
    float centerZ = 0.0f;
    float cameraDistance = 500.0f;
};

DisplayFit calculateDisplayFit(
    const eld::model::ModelMesh& mesh
) {
    DisplayFit fit;

    if (mesh.vertices.empty()) {
        return fit;
    }

    float minX = mesh.vertices.front().x;
    float maxX = mesh.vertices.front().x;
    float minY = mesh.vertices.front().y;
    float maxY = mesh.vertices.front().y;
    float minZ = mesh.vertices.front().z;
    float maxZ = mesh.vertices.front().z;

    for (const eld::model::Vertex& vertex : mesh.vertices) {
        minX = std::min(minX, vertex.x);
        maxX = std::max(maxX, vertex.x);

        minY = std::min(minY, vertex.y);
        maxY = std::max(maxY, vertex.y);

        minZ = std::min(minZ, vertex.z);
        maxZ = std::max(maxZ, vertex.z);
    }

    fit.centerX = (minX + maxX) * 0.5f;
    fit.centerY = (minY + maxY) * 0.5f;
    fit.centerZ = (minZ + maxZ) * 0.5f;

    const float spanX = maxX - minX;
    const float spanY = maxY - minY;
    const float spanZ = maxZ - minZ;

    const float largestSpan =
        std::max({
            spanX,
            spanY,
            spanZ,
            1.0f
        });

    fit.cameraDistance =
        std::max(
            300.0f,
            largestSpan * 2.4f
        );

    return fit;
}

eld::model::ModelMesh makeDisplayMesh(
    const eld::model::ModelMesh& source,
    const DisplayFit& fit
) {
    eld::model::ModelMesh result = source;

    for (eld::model::Vertex& vertex : result.vertices) {
        vertex.x -= fit.centerX;
        vertex.y -= fit.centerY;
        vertex.z -= fit.centerZ;
    }

    return result;
}

bool hasVertexSkins(
    const eld::model::ModelMesh& mesh
) {
    for (const auto& vertex : mesh.vertices) {
        if (vertex.skin.has_value()) {
            return true;
        }
    }

    return false;
}

bool sequenceResolves(
    const eld::definition::SequenceDefinition& sequence,
    const eld::animation::AnimationFrameIndex& animations
) {
    if (sequence.frames.empty()) {
        return false;
    }

    for (const auto& frame : sequence.frames) {
        const eld::animation::ResolvedAnimationFrame resolved =
            animations.resolve(
                frame.primaryFrameId
            );

        if (
            resolved.frame == nullptr ||
            resolved.skeleton == nullptr
        ) {
            return false;
        }
    }

    return true;
}

bool usableNpc(
    const eld::definition::NpcDefinition& npc,
    const eld::definition::SequenceRepository& sequences,
    const eld::model::ModelRepository& models,
    const eld::animation::AnimationFrameIndex& animations
) {
    if (!npc.walkAnimationId.has_value()) {
        return false;
    }

    const auto mesh =
        buildNpcMesh(
            npc,
            models
        );

    if (
        !mesh.has_value() ||
        !hasVertexSkins(*mesh)
    ) {
        return false;
    }

    const auto* walk =
        sequences.find(
            *npc.walkAnimationId
        );

    return
        walk != nullptr &&
        sequenceResolves(
            *walk,
            animations
        );
}

void listCandidates(
    const eld::definition::NpcRepository& npcs,
    const eld::definition::SequenceRepository& sequences,
    const eld::model::ModelRepository& models,
    const eld::animation::AnimationFrameIndex& animations
) {
    std::cout
        << "NPCs with usable walk sequences\n"
        << "===============================\n";

    std::size_t count = 0;

    for (const auto& npc : npcs.list()) {
        if (
            !usableNpc(
                npc,
                sequences,
                models,
                animations
            )
        ) {
            continue;
        }

        std::cout
            << "NPC "
            << std::setw(4)
            << npc.id
            << "  "
            << npc.name
            << "  models="
            << npc.modelIds.size()
            << "  idle=";

        if (npc.idleAnimationId.has_value()) {
            std::cout << *npc.idleAnimationId;
        }
        else {
            std::cout << "none";
        }

        std::cout
            << "  walk="
            << *npc.walkAnimationId
            << '\n';

        if (++count >= 40) {
            break;
        }
    }

    std::cout
        << "\nRun one with:\n"
        << "  ./build/bin/animation_npc_probe ./cache <npc-id>\n";
}

const eld::definition::NpcDefinition* firstUsableNpc(
    const eld::definition::NpcRepository& npcs,
    const eld::definition::SequenceRepository& sequences,
    const eld::model::ModelRepository& models,
    const eld::animation::AnimationFrameIndex& animations
) {
    for (const auto& npc : npcs.list()) {
        if (
            usableNpc(
                npc,
                sequences,
                models,
                animations
            )
        ) {
            return npcs.find(npc.id);
        }
    }

    return nullptr;
}

std::array<Choice, 5> choicesFor(
    const eld::definition::NpcDefinition& npc
) {
    return {{
        {"idle", npc.idleAnimationId},
        {"walk", npc.walkAnimationId},
        {"turn around", npc.turnAroundAnimationId},
        {"turn left", npc.turnLeftAnimationId},
        {"turn right", npc.turnRightAnimationId}
    }};
}

const eld::definition::SequenceDefinition* sequenceFor(
    const std::array<Choice, 5>& choices,
    std::size_t choice,
    const eld::definition::SequenceRepository& sequences
) {
    if (
        choice >= choices.size() ||
        !choices[choice].sequenceId.has_value()
    ) {
        return nullptr;
    }

    return sequences.find(
        *choices[choice].sequenceId
    );
}

bool switchChoice(
    ViewState& state,
    eld::graphics::AnimationPlayer& player,
    std::size_t choice,
    const std::array<Choice, 5>& choices,
    const eld::definition::SequenceRepository& sequences
) {
    const auto* sequence =
        sequenceFor(
            choices,
            choice,
            sequences
        );

    if (
        sequence == nullptr ||
        sequence->frames.empty()
    ) {
        return false;
    }

    state.choice = choice;
    player.setSequence(*sequence);

    return true;
}

void printNpc(
    const eld::definition::NpcDefinition& npc,
    const std::array<Choice, 5>& choices,
    const eld::model::ModelMesh& mesh
) {
    std::cout
        << "Eldoria NPC Animation Probe\n"
        << "===========================\n"
        << "NPC:      "
        << npc.id
        << " / "
        << npc.name
        << '\n'
        << "parts:    "
        << npc.modelIds.size()
        << '\n'
        << "vertices: "
        << mesh.vertices.size()
        << '\n'
        << "faces:    "
        << mesh.faces.size()
        << "\n\n"
        << "Animations\n"
        << "----------\n";

    for (
        std::size_t i = 0;
        i < choices.size();
        ++i
    ) {
        std::cout
            << (i + 1)
            << " = "
            << choices[i].label
            << ": ";

        if (choices[i].sequenceId.has_value()) {
            std::cout
                << *choices[i].sequenceId;
        }
        else {
            std::cout
                << "unavailable";
        }

        std::cout << '\n';
    }

    std::cout
        << "\nControls\n"
        << "--------\n"
        << "1 idle | 2 walk | 3 turn-around | 4 turn-left | 5 turn-right\n"
        << "Space play/pause | Left/Right frame step\n"
        << "A/D yaw | W/S pitch | R reset view | Esc quit\n"
        << "\n"
        << "LEFT  = original assembled NPC\n"
        << "RIGHT = live animated NPC\n"
        << "\n"
        << "Timing/playback is owned by production AnimationPlayer;\n"
        << "one classic client cycle is currently 20 ms.\n";
}

void applyFrame(
    const eld::model::ModelMesh& original,
    eld::model::ModelMesh& animated,
    const eld::graphics::AnimationPlayer& player,
    eld::animation::ResolvedAnimationFrame& resolved,
    eld::graphics::AnimationApplyStats& stats
) {
    const auto* sequenceFrame =
        player.currentSequenceFrame();

    resolved =
        player.currentResolvedFrame();

    if (
        sequenceFrame == nullptr ||
        resolved.frame == nullptr ||
        resolved.skeleton == nullptr
    ) {
        throw std::runtime_error(
            "could not resolve current animation frame"
        );
    }

    animated = original;

    const eld::graphics::ModelAnimator animator;

    stats =
        animator.applyInPlace(
            animated,
            *resolved.frame,
            *resolved.skeleton
        );

}

void printFrame(
    const Choice& choice,
    const eld::graphics::AnimationPlayer& player,
    const eld::animation::ResolvedAnimationFrame& resolved,
    const eld::graphics::AnimationApplyStats& stats
) {
    const auto* sequence =
        player.sequence();

    const auto* frame =
        player.currentSequenceFrame();

    if (
        sequence == nullptr ||
        frame == nullptr
    ) {
        return;
    }

    std::cout
        << "\n"
        << choice.label
        << " "
        << player.frameIndex()
        << "/"
        << (sequence->frames.size() - 1)
        << "  global="
        << frame->primaryFrameId
        << "  archive="
        << resolved.archiveId
        << "  delay="
        << player.currentFrameDurationMilliseconds()
        << "ms"
        << "  pivots="
        << stats.implicitPivots
        << "  translateHits="
        << stats.translatedVertices
        << "  rotateHits="
        << stats.rotatedVertices
        << "  scaleHits="
        << stats.scaledVertices
        << "  alphaHits="
        << stats.alphaFaces
        << "  type4="
        << stats.ignoredUnknownType4
        << '\n';
}

int runProbe(
    const std::filesystem::path& cacheRoot,
    std::optional<std::uint16_t> requestedNpcId,
    bool listOnly
) {
    eld::cache::Cache cache(cacheRoot);

    eld::definition::DefinitionRepository definitions(
        cache.open(
            eld::cache::IndexId::Config
        ),
        2
    );

    eld::definition::NpcRepository npcs(
        definitions.get("npc")
    );

    eld::definition::SequenceRepository sequences(
        definitions.get("seq")
    );

    eld::model::ModelRepository models(
        cache.open(
            eld::cache::IndexId::Models
        )
    );

    eld::animation::AnimationRepository animationsRepository(
        cache.open(
            eld::cache::IndexId::Animations
        )
    );

    eld::texture::TextureRepository textureRepository(
        cache.open(
            eld::cache::IndexId::Config
        )
    );

    eld::graphics::GraphicsResources graphicsResources(
        models,
        textureRepository
    );

    std::cout
        << "building global frame index...\n";

    const eld::animation::AnimationFrameIndex animations(
        animationsRepository
    );

    std::cout
        << "global frames indexed: "
        << animations.frameCount()
        << "\n\n";

    if (listOnly) {
        listCandidates(
            npcs,
            sequences,
            models,
            animations
        );

        return 0;
    }

    const eld::definition::NpcDefinition* npc =
        requestedNpcId.has_value()
            ? npcs.find(*requestedNpcId)
            : firstUsableNpc(
                npcs,
                sequences,
                models,
                animations
            );

    if (npc == nullptr) {
        throw std::runtime_error(
            "NPC not found / no usable default NPC"
        );
    }

    const auto mesh =
        buildNpcMesh(
            *npc,
            models
        );

    if (!mesh.has_value()) {
        throw std::runtime_error(
            "could not assemble NPC model parts"
        );
    }

    const eld::model::ModelMesh original =
        *mesh;

    if (!hasVertexSkins(original)) {
        throw std::runtime_error(
            "assembled NPC has no vertex skin metadata"
        );
    }

    const auto choices =
        choicesFor(*npc);

    ViewState state;

    eld::graphics::AnimationPlayer player(
        animations
    );

    // Start on walk, otherwise idle.
    if (
        !switchChoice(
            state,
            player,
            1,
            choices,
            sequences
        ) &&
        !switchChoice(
            state,
            player,
            0,
            choices,
            sequences
        )
    ) {
        throw std::runtime_error(
            "NPC has no usable walk or idle sequence"
        );
    }

    printNpc(
        *npc,
        choices,
        original
    );

    const DisplayFit displayFit =
        calculateDisplayFit(
            original
        );

    const eld::model::ModelMesh originalDisplayMesh =
        makeDisplayMesh(
            original,
            displayFit
        );

    const eld::graphics::ModelHandle originalRenderHandle =
        graphicsResources.resolveModel(
            originalDisplayMesh
        );

    std::map<
        std::pair<std::uint16_t, std::size_t>,
        eld::graphics::ModelHandle
    > animatedRenderHandles;

    std::optional<eld::graphics::ModelHandle>
        currentAnimatedRenderHandle;

    eld::platform::SdlContext sdl(
        "Eldoria NPC Animation Probe",
        1500,
        850
    );

    SDL_Window* window = sdl.window();
    SDL_Renderer* renderer = sdl.renderer();

    if (
        window == nullptr ||
        renderer == nullptr
    ) {
        return 1;
    }

    eld::model::ModelMesh animated =
        original;

    eld::animation::ResolvedAnimationFrame resolved;
    eld::graphics::AnimationApplyStats stats;

    bool running = true;
    bool dirty = true;

    std::uint64_t lastTick =
        SDL_GetTicks();

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                continue;
            }

            if (event.type != SDL_EVENT_KEY_DOWN) {
                continue;
            }

            if (event.key.key == SDLK_ESCAPE) {
                running = false;
                continue;
            }

            switch (event.key.scancode) {
                case SDL_SCANCODE_1:
                case SDL_SCANCODE_KP_1:
                    dirty |=
                        switchChoice(
                            state,
                            player,
                            0,
                            choices,
                            sequences
                        );
                    break;

                case SDL_SCANCODE_2:
                case SDL_SCANCODE_KP_2:
                    dirty |=
                        switchChoice(
                            state,
                            player,
                            1,
                            choices,
                            sequences
                        );
                    break;

                case SDL_SCANCODE_3:
                case SDL_SCANCODE_KP_3:
                    dirty |=
                        switchChoice(
                            state,
                            player,
                            2,
                            choices,
                            sequences
                        );
                    break;

                case SDL_SCANCODE_4:
                case SDL_SCANCODE_KP_4:
                    dirty |=
                        switchChoice(
                            state,
                            player,
                            3,
                            choices,
                            sequences
                        );
                    break;

                case SDL_SCANCODE_5:
                case SDL_SCANCODE_KP_5:
                    dirty |=
                        switchChoice(
                            state,
                            player,
                            4,
                            choices,
                            sequences
                        );
                    break;

                case SDL_SCANCODE_SPACE:
                    player.setPlaying(
                        !player.isPlaying()
                    );
                    break;

                case SDL_SCANCODE_LEFT:
                    player.pause();
                    dirty |=
                        player.stepBackward();
                    break;

                case SDL_SCANCODE_RIGHT:
                    player.pause();
                    dirty |=
                        player.stepForward();
                    break;

                case SDL_SCANCODE_A:
                    state.yaw -= 0.12f;
                    break;

                case SDL_SCANCODE_D:
                    state.yaw += 0.12f;
                    break;

                case SDL_SCANCODE_W:
                    state.pitch -= 0.10f;
                    break;

                case SDL_SCANCODE_S:
                    state.pitch += 0.10f;
                    break;

                case SDL_SCANCODE_R:
                    state.yaw = 0.55f;
                    state.pitch = -0.30f;
                    break;

                default:
                    break;
            }
        }

        const std::uint64_t now =
            SDL_GetTicks();

        const std::uint64_t delta =
            now - lastTick;

        lastTick = now;

        dirty |=
            player.update(delta);

        const auto* sequence =
            player.sequence();

        if (
            sequence == nullptr ||
            sequence->frames.empty()
        ) {
            throw std::runtime_error(
                "selected sequence is unavailable"
            );
        }

        if (dirty) {
            applyFrame(
                original,
                animated,
                player,
                resolved,
                stats
            );

            const auto& choice =
                choices[state.choice];

            printFrame(
                choice,
                player,
                resolved,
                stats
            );

            const std::pair<std::uint16_t, std::size_t> renderKey{
                *choice.sequenceId,
                player.frameIndex()
            };

            const auto existingRenderHandle =
                animatedRenderHandles.find(
                    renderKey
                );

            if (
                existingRenderHandle !=
                animatedRenderHandles.end()
            ) {
                currentAnimatedRenderHandle =
                    existingRenderHandle->second;
            }
            else {
                const eld::model::ModelMesh displayAnimated =
                    makeDisplayMesh(
                        animated,
                        displayFit
                    );

                const eld::graphics::ModelHandle handle =
                    graphicsResources.resolveModel(
                        displayAnimated
                    );

                animatedRenderHandles.emplace(
                    renderKey,
                    handle
                );

                currentAnimatedRenderHandle =
                    handle;
            }

            std::ostringstream title;
            title
                << "Eldoria NPC Probe | "
                << npc->name
                << " ["
                << npc->id
                << "] | "
                << choice.label
                << " | seq "
                << *choice.sequenceId
                << " | frame "
                << player.frameIndex()
                << "/"
                << (sequence->frames.size() - 1)
                << " | "
                << (
                    player.isPlaying()
                        ? "PLAY"
                        : "PAUSED"
                );

            SDL_SetWindowTitle(
                window,
                title.str().c_str()
            );

            dirty = false;
        }

        int windowWidth = 0;
        int windowHeight = 0;

        SDL_GetWindowSize(
            window,
            &windowWidth,
            &windowHeight
        );

        SDL_SetRenderDrawColor(
            renderer,
            18,
            20,
            24,
            255
        );

        SDL_RenderClear(
            renderer
        );

        if (!currentAnimatedRenderHandle.has_value()) {
            SDL_RenderPresent(
                renderer
            );

            SDL_Delay(
                5
            );

            continue;
        }

        const int leftWidth =
            std::max(
                1,
                windowWidth / 2
            );

        const int rightWidth =
            std::max(
                1,
                windowWidth -
                leftWidth
            );

        const int renderHeight =
            std::max(
                1,
                windowHeight
            );

        eld::render::Camera leftCamera;

        leftCamera.position = {
            0.0f,
            0.0f,
            -displayFit.cameraDistance
        };

        leftCamera.rotation = {
            0.0f,
            0.0f,
            0.0f
        };

        leftCamera.verticalFov =
            1.04719755f;

        leftCamera.nearPlane =
            1.0f;

        leftCamera.farPlane =
            std::max(
                10000.0f,
                displayFit.cameraDistance *
                    8.0f
            );

        leftCamera.viewportWidth =
            static_cast<std::uint32_t>(
                leftWidth
            );

        leftCamera.viewportHeight =
            static_cast<std::uint32_t>(
                renderHeight
            );

        eld::render::Camera rightCamera =
            leftCamera;

        rightCamera.viewportWidth =
            static_cast<std::uint32_t>(
                rightWidth
            );

        eld::render::Transform modelTransform;

        modelTransform.position = {
            0.0f,
            0.0f,
            0.0f
        };

        modelTransform.rotation = {
            state.pitch,
            state.yaw,
            0.0f
        };

        modelTransform.scale = {
            1.0f,
            1.0f,
            1.0f
        };

        eld::render::SoftwareRenderBackend backend(
            renderer
        );

        backend.setClearColor({
            24,
            27,
            32,
            255
        });

        backend.setOutputPosition(
            0,
            0
        );

        backend.beginFrame(
            leftCamera
        );

        backend.draw(
            originalRenderHandle,
            modelTransform,
            graphicsResources
        );

        backend.endFrame();

        backend.setOutputPosition(
            leftWidth,
            0
        );

        backend.beginFrame(
            rightCamera
        );

        backend.draw(
            *currentAnimatedRenderHandle,
            modelTransform,
            graphicsResources
        );

        backend.endFrame();

        SDL_SetRenderDrawColor(
            renderer,
            80,
            84,
            94,
            255
        );

        SDL_RenderLine(
            renderer,
            static_cast<float>(
                leftWidth
            ),
            0.0f,
            static_cast<float>(
                leftWidth
            ),
            static_cast<float>(
                renderHeight
            )
        );

        SDL_RenderPresent(
            renderer
        );

        SDL_Delay(
            5
        );
    }

    return 0;
}

std::optional<std::uint16_t> parseNpcId(
    const char* text
) {
    try {
        const unsigned long value =
            std::stoul(text);

        if (value > 65535UL) {
            return std::nullopt;
        }

        return
            static_cast<std::uint16_t>(
                value
            );
    }
    catch (...) {
        return std::nullopt;
    }
}

}

int main(
    int argc,
    char** argv
) {
    if (
        argc < 2 ||
        argc > 3
    ) {
        std::cerr
            << "usage: "
            << (
                argc > 0
                    ? argv[0]
                    : "animation_npc_probe"
            )
            << " <cache-root> [npc-id|--list]\n";

        return 1;
    }

    bool listOnly = false;
    std::optional<std::uint16_t> npcId;

    if (argc == 3) {
        const std::string argument =
            argv[2];

        if (argument == "--list") {
            listOnly = true;
        }
        else {
            npcId =
                parseNpcId(argv[2]);

            if (!npcId.has_value()) {
                std::cerr
                    << "invalid NPC id: "
                    << argument
                    << '\n';

                return 1;
            }
        }
    }

    try {
        return runProbe(
            std::filesystem::path(argv[1]),
            npcId,
            listOnly
        );
    }
    catch (const std::exception& exception) {
        std::cerr
            << "animation_npc_probe failed: "
            << exception.what()
            << '\n';

        return 1;
    }
}
