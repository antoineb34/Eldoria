#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "AssetManager.h"

struct ModelReference {
    std::string kind;
    std::uint16_t definitionId = 0;
    std::string name;
    std::string usage;
    std::optional<std::uint8_t> modelType;
};

using ReferenceIndex =
    std::unordered_map<
        std::uint16_t,
        std::vector<ModelReference>
    >;


struct WorldSpawn {
    std::uint16_t regionId = 0;
    std::uint8_t plane = 0;
    std::uint8_t x = 0;
    std::uint8_t y = 0;
    std::uint8_t type = 0;
    std::uint8_t rotation = 0;
};

using WorldSpawnIndex =
    std::unordered_map<
        std::uint16_t,
        std::vector<WorldSpawn>
    >;

struct WorldIndex {
    WorldSpawnIndex byLocationId;

    std::size_t regions = 0;
    std::size_t decodedRegions = 0;
    std::size_t failedRegions = 0;
    std::size_t totalSpawns = 0;
};

static void addReference(
    ReferenceIndex& index,
    std::uint16_t modelId,
    std::string kind,
    std::uint16_t definitionId,
    const std::string& name,
    std::string usage,
    std::optional<std::uint8_t> modelType = std::nullopt
) {
    index[modelId].push_back(
        ModelReference {
            .kind = std::move(kind),
            .definitionId = definitionId,
            .name = name,
            .usage = std::move(usage),
            .modelType = modelType
        }
    );
}

static ReferenceIndex buildReferenceIndex(
    eld::asset::AssetManager& assets
) {
    ReferenceIndex index;

    // -------------------------------------------------
    // Items
    // -------------------------------------------------
    for (const std::uint16_t itemId : assets.items.listIds()) {
        const auto& item =
            assets.items.data(itemId);

        if (item.inventoryModelId) {
            addReference(
                index,
                *item.inventoryModelId,
                "item",
                item.id,
                item.name,
                "inventory"
            );
        }

        for (std::size_t i = 0;
             i < item.maleModelIds.size();
             ++i) {
            if (item.maleModelIds[i]) {
                addReference(
                    index,
                    *item.maleModelIds[i],
                    "item",
                    item.id,
                    item.name,
                    "male_worn_" + std::to_string(i)
                );
            }
        }

        for (std::size_t i = 0;
             i < item.femaleModelIds.size();
             ++i) {
            if (item.femaleModelIds[i]) {
                addReference(
                    index,
                    *item.femaleModelIds[i],
                    "item",
                    item.id,
                    item.name,
                    "female_worn_" + std::to_string(i)
                );
            }
        }

        for (std::size_t i = 0;
             i < item.maleHeadModelIds.size();
             ++i) {
            if (item.maleHeadModelIds[i]) {
                addReference(
                    index,
                    *item.maleHeadModelIds[i],
                    "item",
                    item.id,
                    item.name,
                    "male_head_" + std::to_string(i)
                );
            }
        }

        for (std::size_t i = 0;
             i < item.femaleHeadModelIds.size();
             ++i) {
            if (item.femaleHeadModelIds[i]) {
                addReference(
                    index,
                    *item.femaleHeadModelIds[i],
                    "item",
                    item.id,
                    item.name,
                    "female_head_" + std::to_string(i)
                );
            }
        }
    }

    // -------------------------------------------------
    // NPCs
    // -------------------------------------------------
    for (const std::uint16_t npcId : assets.npcs.listIds()) {
        const auto& npc =
            assets.npcs.data(npcId);

        for (const std::uint16_t modelId : npc.modelIds) {
            addReference(
                index,
                modelId,
                "npc",
                npc.id,
                npc.name,
                "body"
            );
        }

        for (const std::uint16_t modelId : npc.headModelIds) {
            addReference(
                index,
                modelId,
                "npc",
                npc.id,
                npc.name,
                "head"
            );
        }

        for (std::size_t i = 0;
             i < npc.additionalModelIds.size();
             ++i) {
            if (npc.additionalModelIds[i]) {
                addReference(
                    index,
                    *npc.additionalModelIds[i],
                    "npc",
                    npc.id,
                    npc.name,
                    "additional_" + std::to_string(i)
                );
            }
        }
    }

    // -------------------------------------------------
    // Locations / objects
    // -------------------------------------------------
    for (const std::uint16_t locationId :
         assets.locations.listIds()) {

        const auto& location =
            assets.locations.data(locationId);

        for (const auto& model : location.models) {
            addReference(
                index,
                model.id,
                "location",
                location.id,
                location.name,
                "model",
                model.type
            );
        }
    }


    // -------------------------------------------------
    // Identity kits / player appearance
    // -------------------------------------------------
    for (const std::uint16_t kitId :
         assets.identityKits.listIds()) {

        const auto& kit =
            assets.identityKits.data(kitId);

        for (std::size_t i = 0;
             i < kit.modelIds.size();
             ++i) {

            std::string usage = "body";

            if (kit.bodyPartId) {
                usage += "_part_" +
                    std::to_string(*kit.bodyPartId);
            }

            usage += "_" + std::to_string(i);

            addReference(
                index,
                kit.modelIds[i],
                "identity_kit",
                kit.id,
                "IdentityKit",
                std::move(usage)
            );
        }

        for (std::size_t i = 0;
             i < kit.headModelIds.size();
             ++i) {

            if (kit.headModelIds[i]) {
                addReference(
                    index,
                    *kit.headModelIds[i],
                    "identity_kit",
                    kit.id,
                    "IdentityKit",
                    "head_" + std::to_string(i)
                );
            }
        }
    }

    // -------------------------------------------------
    // Spot animations / graphical effects
    // -------------------------------------------------
    for (const std::uint16_t spotId :
         assets.spotAnimations.listIds()) {

        const auto& spot =
            assets.spotAnimations.data(spotId);

        if (spot.modelId) {
            addReference(
                index,
                *spot.modelId,
                "spot_animation",
                spot.id,
                "SpotAnimation",
                "effect_model"
            );
        }
    }


    // -------------------------------------------------
    // Interface widgets
    // -------------------------------------------------
    for (const std::uint16_t widgetId :
         assets.widgets.listIds()) {

        const auto& widget =
            assets.widgets.data(widgetId);

        if (widget.modelId) {
            addReference(
                index,
                *widget.modelId,
                "widget",
                widget.id,
                "Widget",
                "primary_model"
            );
        }

        if (widget.secondaryModelId) {
            addReference(
                index,
                *widget.secondaryModelId,
                "widget",
                widget.id,
                "Widget",
                "secondary_model"
            );
        }
    }

    return index;
}


static WorldIndex buildWorldIndex(
    eld::asset::AssetManager& assets
) {
    WorldIndex world;

    const auto regionIds =
        assets.maps.listIds();

    world.regions = regionIds.size();

    for (const std::uint16_t regionId : regionIds) {
        try {
            const auto& spawns =
                assets.maps.locations(regionId);

            ++world.decodedRegions;
            world.totalSpawns += spawns.size();

            for (const auto& spawn : spawns) {
                world.byLocationId[spawn.id].push_back(
                    WorldSpawn {
                        .regionId = regionId,
                        .plane = spawn.plane,
                        .x = spawn.x,
                        .y = spawn.y,
                        .type = spawn.type,
                        .rotation = spawn.rotation
                    }
                );
            }
        }
        catch (const std::exception& e) {
            ++world.failedRegions;

            std::cerr
                << "Failed map region "
                << regionId
                << ": "
                << e.what()
                << '\n';
        }
    }

    return world;
}

static std::size_t countWorldSpawns(
    const WorldIndex& world,
    const ModelReference& ref
) {
    const auto found =
        world.byLocationId.find(
            ref.definitionId
        );

    if (found == world.byLocationId.end()) {
        return 0;
    }

    std::size_t count = 0;

    for (const auto& spawn : found->second) {
        if (
            !ref.modelType ||
            spawn.type == *ref.modelType
        ) {
            ++count;
        }
    }

    return count;
}

static void inspectModel(
    eld::asset::AssetManager& assets,
    const ReferenceIndex& references,
    const WorldIndex& world,
    std::uint16_t modelId
) {
    const auto* model =
        assets.models.find(modelId);

    if (!model) {
        std::cout
            << "Model "
            << modelId
            << " does not exist in the model cache.\n";
        return;
    }

    std::cout
        << "\n=== MODEL " << modelId << " ===\n"
        << "Vertices:         "
        << model->vertices.size()
        << '\n'
        << "Faces:            "
        << model->faces.size()
        << '\n'
        << "Texture mappings: "
        << model->textureMappings.size()
        << "\n\n";

    const auto found =
        references.find(modelId);

    if (found == references.end()) {
        std::cout
            << "References: none\n";
        return;
    }

    std::cout
        << "References: "
        << found->second.size()
        << "\n\n";

    for (const auto& ref : found->second) {
        std::cout
            << ref.kind
            << " "
            << ref.definitionId
            << " \""
            << ref.name
            << "\""
            << "\n  usage: "
            << ref.usage;

        if (ref.modelType) {
            std::cout
                << "\n  model type: "
                << static_cast<unsigned>(
                    *ref.modelType
                );
        }

        if (ref.kind == "location") {
            std::cout
                << "\n  world spawns: "
                << countWorldSpawns(
                    world,
                    ref
                );
        }

        std::cout << "\n\n";
    }
}


static std::string csvEscape(
    const std::string& value
) {
    const bool needsQuotes =
        value.find(',') != std::string::npos ||
        value.find('"') != std::string::npos ||
        value.find('\n') != std::string::npos ||
        value.find('\r') != std::string::npos;

    if (!needsQuotes) {
        return value;
    }

    std::string result;
    result.reserve(value.size() + 2);

    result.push_back('"');

    for (const char c : value) {
        if (c == '"') {
            result.push_back('"');
        }

        result.push_back(c);
    }

    result.push_back('"');

    return result;
}

static void exportDataset(
    eld::asset::AssetManager& assets,
    const ReferenceIndex& references,
    const WorldIndex& world,
    const std::filesystem::path& outputDirectory
) {
    std::filesystem::create_directories(
        outputDirectory
    );

    // -------------------------------------------------
    // models.csv
    // -------------------------------------------------

    {
        std::ofstream file(
            outputDirectory / "models.csv"
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to create models.csv"
            );
        }

        file
            << "model_id,"
            << "vertices,"
            << "faces,"
            << "texture_mappings,"
            << "reference_count\n";

        auto modelIds =
            assets.models.listIds();

        std::sort(
            modelIds.begin(),
            modelIds.end()
        );

        for (const std::uint16_t modelId :
             modelIds) {

            const auto& model =
                assets.models.get(modelId);

            const auto found =
                references.find(modelId);

            const std::size_t referenceCount =
                found == references.end()
                    ? 0
                    : found->second.size();

            file
                << modelId << ','
                << model.vertices.size() << ','
                << model.faces.size() << ','
                << model.textureMappings.size() << ','
                << referenceCount
                << '\n';
        }
    }


    // -------------------------------------------------
    // vertices.csv
    // -------------------------------------------------

    {
        std::ofstream file(
            outputDirectory / "vertices.csv"
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to create vertices.csv"
            );
        }

        file
            << "model_id,"
            << "vertex_index,"
            << "x,"
            << "y,"
            << "z,"
            << "skin\n";

        auto modelIds =
            assets.models.listIds();

        std::sort(
            modelIds.begin(),
            modelIds.end()
        );

        for (const std::uint16_t modelId :
             modelIds) {

            const auto& model =
                assets.models.get(modelId);

            for (std::size_t i = 0;
                 i < model.vertices.size();
                 ++i) {

                const auto& vertex =
                    model.vertices[i];

                file
                    << modelId << ','
                    << i << ','
                    << vertex.x << ','
                    << vertex.y << ','
                    << vertex.z << ',';

                if (vertex.skin) {
                    file
                        << static_cast<unsigned>(
                            *vertex.skin
                        );
                }

                file << '\n';
            }
        }
    }

    // -------------------------------------------------
    // faces.csv
    // -------------------------------------------------

    {
        std::ofstream file(
            outputDirectory / "faces.csv"
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to create faces.csv"
            );
        }

        file
            << "model_id,"
            << "face_index,"
            << "a,"
            << "b,"
            << "c,"
            << "color,"
            << "priority,"
            << "alpha,"
            << "render_type,"
            << "skin,"
            << "texture_id,"
            << "texture_mapping_index\n";

        auto modelIds =
            assets.models.listIds();

        std::sort(
            modelIds.begin(),
            modelIds.end()
        );

        for (const std::uint16_t modelId :
             modelIds) {

            const auto& model =
                assets.models.get(modelId);

            for (std::size_t i = 0;
                 i < model.faces.size();
                 ++i) {

                const auto& face =
                    model.faces[i];

                file
                    << modelId << ','
                    << i << ','
                    << face.a << ','
                    << face.b << ','
                    << face.c << ','
                    << face.color << ','
                    << static_cast<unsigned>(
                        face.priority
                    )
                    << ','
                    << static_cast<unsigned>(
                        face.alpha
                    )
                    << ','
                    << static_cast<unsigned>(
                        face.renderType
                    )
                    << ',';

                if (face.skin) {
                    file
                        << static_cast<unsigned>(
                            *face.skin
                        );
                }

                file << ',';

                if (face.textureId) {
                    file << *face.textureId;
                }

                file << ',';

                if (face.textureMappingIndex) {
                    file
                        << *face.textureMappingIndex;
                }

                file << '\n';
            }
        }
    }

    // -------------------------------------------------
    // references.csv
    // -------------------------------------------------

    {
        std::ofstream file(
            outputDirectory / "references.csv"
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to create references.csv"
            );
        }

        file
            << "model_id,"
            << "model_exists,"
            << "kind,"
            << "definition_id,"
            << "name,"
            << "usage,"
            << "model_type\n";

        std::vector<std::uint16_t> modelIds;

        modelIds.reserve(
            references.size()
        );

        for (const auto& [modelId, refs] :
             references) {
            modelIds.push_back(modelId);
        }

        std::sort(
            modelIds.begin(),
            modelIds.end()
        );

        for (const std::uint16_t modelId :
             modelIds) {

            const auto& refs =
                references.at(modelId);

            for (const auto& ref : refs) {
                file
                    << modelId << ','
                    << (
                        assets.models.contains(modelId)
                            ? 1
                            : 0
                    )
                    << ','
                    << csvEscape(ref.kind)
                    << ','
                    << ref.definitionId
                    << ','
                    << csvEscape(ref.name)
                    << ','
                    << csvEscape(ref.usage)
                    << ',';

                if (ref.modelType) {
                    file
                        << static_cast<unsigned>(
                            *ref.modelType
                        );
                }

                file << '\n';
            }
        }
    }

    // -------------------------------------------------
    // world_spawns.csv
    // -------------------------------------------------

    {
        std::ofstream file(
            outputDirectory / "world_spawns.csv"
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to create world_spawns.csv"
            );
        }

        file
            << "location_id,"
            << "region_id,"
            << "plane,"
            << "x,"
            << "y,"
            << "type,"
            << "rotation\n";

        std::vector<std::uint16_t> locationIds;

        locationIds.reserve(
            world.byLocationId.size()
        );

        for (const auto& [locationId, spawns] :
             world.byLocationId) {
            locationIds.push_back(locationId);
        }

        std::sort(
            locationIds.begin(),
            locationIds.end()
        );

        for (const std::uint16_t locationId :
             locationIds) {

            const auto& spawns =
                world.byLocationId.at(locationId);

            for (const auto& spawn : spawns) {
                file
                    << locationId << ','
                    << spawn.regionId << ','
                    << static_cast<unsigned>(
                        spawn.plane
                    )
                    << ','
                    << static_cast<unsigned>(
                        spawn.x
                    )
                    << ','
                    << static_cast<unsigned>(
                        spawn.y
                    )
                    << ','
                    << static_cast<unsigned>(
                        spawn.type
                    )
                    << ','
                    << static_cast<unsigned>(
                        spawn.rotation
                    )
                    << '\n';
            }
        }
    }

    std::cout
        << "\nDataset exported to: "
        << outputDirectory.string()
        << '\n';
}

int main(int argc, char** argv)
{
    try {
        eld::asset::AssetManager assets;

        std::cout
            << "Building semantic reference index...\n";

        const ReferenceIndex references =
            buildReferenceIndex(assets);

        std::cout
            << "Scanning world location spawns...\n";

        const WorldIndex world =
            buildWorldIndex(assets);

        std::size_t referenceCount = 0;

        for (const auto& [modelId, refs] : references) {
            referenceCount += refs.size();
        }

        const auto modelIds =
            assets.models.listIds();

        std::size_t referencedModels = 0;
        std::size_t unreferencedModels = 0;
        std::size_t danglingModelIds = 0;

        for (const std::uint16_t id : modelIds) {
            if (references.contains(id)) {
                ++referencedModels;
            }
            else {
                ++unreferencedModels;
            }
        }

        for (const auto& [modelId, refs] : references) {
            if (!assets.models.contains(modelId)) {
                ++danglingModelIds;

                std::cout
                    << "\nDangling model "
                    << modelId
                    << '\n';

                for (const auto& ref : refs) {
                    std::cout
                        << "  "
                        << ref.kind
                        << " "
                        << ref.definitionId
                        << " \""
                        << ref.name
                        << "\""
                        << " usage="
                        << ref.usage;

                    if (ref.modelType) {
                        std::cout
                            << " type="
                            << static_cast<unsigned>(
                                *ref.modelType
                            );
                    }

                    std::cout << '\n';
                }
            }
        }

        std::cout
            << "\n=== SEMANTIC INDEX ===\n"
            << "Models:              "
            << modelIds.size()
            << '\n'
            << "Item definitions:    "
            << assets.items.count()
            << '\n'
            << "NPC definitions:     "
            << assets.npcs.count()
            << '\n'
            << "Location definitions:"
            << ' '
            << assets.locations.count()
            << '\n'
            << "Total references:    "
            << referenceCount
            << '\n'
            << "Referenced models:   "
            << referencedModels
            << '\n'
            << "Unreferenced models: "
            << unreferencedModels
            << '\n'
            << "Dangling model IDs:  "
            << danglingModelIds
            << '\n'
            << "\n=== WORLD INDEX ===\n"
            << "Map regions:          "
            << world.regions
            << '\n'
            << "Decoded regions:      "
            << world.decodedRegions
            << '\n'
            << "Failed regions:       "
            << world.failedRegions
            << '\n'
            << "Location spawns:      "
            << world.totalSpawns
            << '\n'
            << "Spawned location IDs: "
            << world.byLocationId.size()
            << '\n';

        if (
            argc == 2 &&
            std::string(argv[1]) == "export"
        ) {
            exportDataset(
                assets,
                references,
                world,
                "dataset"
            );

            return 0;
        }

        if (
            argc == 3 &&
            std::string(argv[1]) == "inspect-model"
        ) {
            const unsigned long parsed =
                std::stoul(argv[2]);

            if (parsed > 65535) {
                std::cerr
                    << "Model ID out of range.\n";
                return 1;
            }

            inspectModel(
                assets,
                references,
                world,
                static_cast<std::uint16_t>(parsed)
            );
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr
            << "dataset_builder failed: "
            << e.what()
            << '\n';

        return 1;
    }
}
