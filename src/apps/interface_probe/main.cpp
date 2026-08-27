#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "cache/Cache.h"
#include "cache/Index.h"
#include "interface/InterfaceFile.h"
#include "interface/InterfaceRepository.h"
#include "model/ModelRepository.h"
#include "sprite/SpriteRepository.h"
#include "texture/TextureRepository.h"

#include "GraphicsResources.h"
#include "RenderPipeline.h"
#include "backend/software/SoftwareRenderBackend.h"
#include "scene/RenderScene.h"

#include "ImGuiTheme.h"
#include "sdl/SdlContext.h"

namespace {

constexpr std::uint16_t InterfaceArchiveFileId = 3;
constexpr std::uint16_t MediaArchiveFileId = 4;
constexpr std::uint16_t DefaultRootId = 2505;
constexpr int TypeCount = 9;
constexpr int SlotSize = 32;
constexpr float Pi = 3.14159265358979323846f;

struct Interfaces {
    eld::interface::InterfaceFile file;
    std::unordered_map<std::uint16_t, const eld::interface::InterfaceFileWidget*> byId;
    std::vector<std::uint16_t> containerIds;
};

struct SpriteRef {
    std::string group;
    std::uint16_t frame = 0;
};

struct SpriteTexture {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
};

struct State {
    int rootId = DefaultRootId;

    std::array<bool, TypeCount> showContent{true, true, true, true, true, true, true, true, true};
    std::array<bool, TypeCount> showOutline{true, true, true, true, true, true, true, true, true};

    std::unordered_map<std::uint16_t, int> scrollOffsets;

    bool clipContainers = true;
    bool respectHidden = true;
    bool showEmptyInventorySlots = true;
    bool showItemText = true;

    bool showModelText = false;
    bool showRawModelWireframe = true;

    std::string interfaceDump;
    std::string interfaceDumpPath;
};


const char* typeName(std::uint8_t type) {
    switch (type) {
        case 0: return "container";
        case 1: return "type1";
        case 2: return "inventory";
        case 3: return "rect";
        case 4: return "text";
        case 5: return "sprite";
        case 6: return "model";
        case 7: return "inv-text";
        case 8: return "tooltip";
        default: return "unknown";
    }
}

ImU32 typeColor(std::uint8_t type) {
    switch (type) {
        case 0: return IM_COL32(150, 150, 150, 255);
        case 1: return IM_COL32(235, 235, 235, 255);
        case 2: return IM_COL32(255, 170, 55, 255);
        case 3: return IM_COL32(255, 80, 80, 255);
        case 4: return IM_COL32(90, 170, 255, 255);
        case 5: return IM_COL32(90, 240, 130, 255);
        case 6: return IM_COL32(195, 125, 255, 255);
        case 7: return IM_COL32(250, 230, 110, 255);
        case 8: return IM_COL32(255, 150, 85, 255);
        default: return IM_COL32(255, 255, 255, 255);
    }
}

ImU32 interfaceColor(std::uint32_t color, std::uint8_t opacity = 0) {
    const auto r = static_cast<std::uint8_t>((color >> 16) & 0xff);
    const auto g = static_cast<std::uint8_t>((color >> 8) & 0xff);
    const auto b = static_cast<std::uint8_t>(color & 0xff);
    const int a = opacity == 0 ? 255 : std::clamp(256 - static_cast<int>(opacity), 0, 255);
    return IM_COL32(r, g, b, static_cast<std::uint8_t>(a));
}


bool enabled(const std::array<bool, TypeCount>& values, std::uint8_t type) {
    return type >= values.size() || values[type];
}

void setAll(std::array<bool, TypeCount>& values, bool value) {
    for (bool& entry : values) {
        entry = value;
    }
}

std::string trim(std::string value) {
    while (!value.empty() && value.front() == ' ') {
        value.erase(value.begin());
    }

    while (!value.empty() && value.back() == ' ') {
        value.pop_back();
    }

    return value;
}

bool endsWith(const std::string& value, const std::string& suffix) {
    return value.size() >= suffix.size() &&
        value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string spriteGroupName(std::string group) {
    group = trim(group);

    if (group.empty() || endsWith(group, ".dat")) {
        return group;
    }

    return group + ".dat";
}

std::optional<SpriteRef> parseSpriteRef(const std::string& text) {
    if (text.empty()) {
        return std::nullopt;
    }

    SpriteRef ref;
    const std::size_t comma = text.find(',');

    if (comma == std::string::npos) {
        ref.group = spriteGroupName(text);
    } else {
        ref.group = spriteGroupName(text.substr(0, comma));

        try {
            const std::string frameText = trim(text.substr(comma + 1));

            if (!frameText.empty()) {
                const int frame = std::stoi(frameText);

                if (frame < 0 || frame > 65535) {
                    return std::nullopt;
                }

                ref.frame = static_cast<std::uint16_t>(frame);
            }
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    if (ref.group.empty()) {
        return std::nullopt;
    }

    return ref;
}

std::string spriteKey(const SpriteRef& ref) {
    return ref.group + "," + std::to_string(ref.frame);
}

std::string optionalU16Text(const std::optional<std::uint16_t>& value) {
    return value.has_value() ? std::to_string(*value) : "none";
}

class SpriteTextureCache {
public:
    SpriteTextureCache(SDL_Renderer* renderer, eld::sprite::SpriteRepository& repository)
        : renderer_(renderer), repository_(repository) {
    }

    ~SpriteTextureCache() {
        for (auto& [key, texture] : textures_) {
            if (texture.texture != nullptr) {
                SDL_DestroyTexture(texture.texture);
            }
        }
    }

    SpriteTexture* find(const std::string& spriteText) {
        const std::optional<SpriteRef> ref = parseSpriteRef(spriteText);

        if (!ref.has_value()) {
            return nullptr;
        }

        const std::string key = spriteKey(*ref);

        if (auto cached = textures_.find(key); cached != textures_.end()) {
            return &cached->second;
        }

        std::optional<eld::sprite::Sprite> sprite =
            repository_.find(ref->group, ref->frame);

        if (!sprite.has_value()) {
            std::printf(
                "missing sprite: %s frame=%u\n",
                ref->group.c_str(),
                static_cast<unsigned>(ref->frame)
            );

            return nullptr;
        }

        SpriteTexture texture = makeTexture(sprite->image);

        if (texture.texture == nullptr) {
            return nullptr;
        }

        auto inserted = textures_.emplace(key, texture);
        return &inserted.first->second;
    }

private:
    SpriteTexture makeTexture(const eld::image::Image& image) const {
        const std::size_t pixelCount =
            static_cast<std::size_t>(image.width) *
            static_cast<std::size_t>(image.height);

        if (image.width == 0 || image.height == 0 || image.pixels.size() < pixelCount) {
            return {};
        }

        std::vector<std::uint8_t> rgba;
        rgba.reserve(pixelCount * 4);

        for (const eld::image::RgbaPixel& pixel : image.pixels) {
            rgba.push_back(pixel.red);
            rgba.push_back(pixel.green);
            rgba.push_back(pixel.blue);
            rgba.push_back(pixel.alpha);
        }

        SDL_Texture* texture =
            SDL_CreateTexture(
                renderer_,
                SDL_PIXELFORMAT_RGBA32,
                SDL_TEXTUREACCESS_STATIC,
                static_cast<int>(image.width),
                static_cast<int>(image.height)
            );

        if (texture == nullptr) {
            return {};
        }

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        if (!SDL_UpdateTexture(texture, nullptr, rgba.data(), static_cast<int>(image.width) * 4)) {
            SDL_DestroyTexture(texture);
            return {};
        }

        return SpriteTexture{
            .texture = texture,
            .width = static_cast<int>(image.width),
            .height = static_cast<int>(image.height)
        };
    }

    SDL_Renderer* renderer_ = nullptr;
    eld::sprite::SpriteRepository& repository_;
    std::unordered_map<std::string, SpriteTexture> textures_;
};


Interfaces loadInterfaces(eld::cache::Store store) {
    eld::interface::InterfaceRepository repository(store, InterfaceArchiveFileId);

    Interfaces interfaces;
    interfaces.file = repository.getFile();

    for (const eld::interface::InterfaceFileWidget& widget : interfaces.file.widgets) {
        interfaces.byId.emplace(widget.id, &widget);

        if (widget.type == 0 && !widget.children.empty()) {
            interfaces.containerIds.push_back(widget.id);
        }
    }

    std::sort(interfaces.containerIds.begin(), interfaces.containerIds.end());

    interfaces.containerIds.erase(
        std::unique(interfaces.containerIds.begin(), interfaces.containerIds.end()),
        interfaces.containerIds.end()
    );

    return interfaces;
}

const eld::interface::InterfaceFileWidget* findWidget(const Interfaces& interfaces, int id) {
    if (id < 0 || id > 65535) {
        return nullptr;
    }

    const auto found = interfaces.byId.find(static_cast<std::uint16_t>(id));
    return found == interfaces.byId.end() ? nullptr : found->second;
}

int scrollMax(const eld::interface::InterfaceFileWidget& widget) {
    if (widget.type != 0 || widget.scrollHeight <= widget.height) {
        return 0;
    }

    return static_cast<int>(widget.scrollHeight - widget.height);
}

int scrollOffset(const State& state, const eld::interface::InterfaceFileWidget& widget) {
    const auto found = state.scrollOffsets.find(widget.id);

    if (found == state.scrollOffsets.end()) {
        return 0;
    }

    return std::clamp(found->second, 0, scrollMax(widget));
}

int containerIndex(const Interfaces& interfaces, int id) {
    if (id < 0 || id > 65535) {
        return -1;
    }

    const auto found =
        std::find(
            interfaces.containerIds.begin(),
            interfaces.containerIds.end(),
            static_cast<std::uint16_t>(id)
        );

    if (found == interfaces.containerIds.end()) {
        return -1;
    }

    return static_cast<int>(std::distance(interfaces.containerIds.begin(), found));
}

void goToContainer(const Interfaces& interfaces, State& state, int delta) {
    if (interfaces.containerIds.empty()) {
        return;
    }

    int index = containerIndex(interfaces, state.rootId);

    if (index < 0) {
        const auto lower =
            std::lower_bound(
                interfaces.containerIds.begin(),
                interfaces.containerIds.end(),
                static_cast<std::uint16_t>(std::clamp(state.rootId, 0, 65535))
            );

        index = lower == interfaces.containerIds.end()
            ? 0
            : static_cast<int>(std::distance(interfaces.containerIds.begin(), lower));
    }

    const int count = static_cast<int>(interfaces.containerIds.size());
    index = (index + delta + count) % count;
    state.rootId = static_cast<int>(interfaces.containerIds[static_cast<std::size_t>(index)]);
}

void goToFirstContainer(const Interfaces& interfaces, State& state) {
    if (!interfaces.containerIds.empty()) {
        state.rootId = static_cast<int>(interfaces.containerIds.front());
    }
}

void goToLastContainer(const Interfaces& interfaces, State& state) {
    if (!interfaces.containerIds.empty()) {
        state.rootId = static_cast<int>(interfaces.containerIds.back());
    }
}

ImVec2 widgetSize(const eld::interface::InterfaceFileWidget& widget) {
    if (widget.type == 2) {
        const int columns = static_cast<int>(widget.width);
        const int rows = static_cast<int>(widget.height);

        return ImVec2(
            static_cast<float>(columns * SlotSize + static_cast<int>(widget.inventoryPaddingX) * std::max(columns - 1, 0)),
            static_cast<float>(rows * SlotSize + static_cast<int>(widget.inventoryPaddingY) * std::max(rows - 1, 0))
        );
    }

    if (widget.type == 7) {
        const int columns = static_cast<int>(widget.width);
        const int rows = static_cast<int>(widget.height);

        return ImVec2(
            static_cast<float>(columns * SlotSize + static_cast<int>(widget.itemPaddingX) * std::max(columns - 1, 0)),
            static_cast<float>(rows * SlotSize + static_cast<int>(widget.itemPaddingY) * std::max(rows - 1, 0))
        );
    }

    return ImVec2(
        std::max(static_cast<float>(widget.width), 1.0f),
        std::max(static_cast<float>(widget.height), 1.0f)
    );
}

std::vector<std::string> splitText(const std::string& text) {
    std::vector<std::string> lines;
    std::string current;

    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\\' && i + 1 < text.size() && text[i + 1] == 'n') {
            lines.push_back(current);
            current.clear();
            ++i;
            continue;
        }

        if (text[i] == '\n') {
            lines.push_back(current);
            current.clear();
            continue;
        }

        current.push_back(text[i]);
    }

    lines.push_back(current);
    return lines;
}

void appendOptionalU16(
    std::ostringstream& out,
    const char* name,
    const std::optional<std::uint16_t>& value
) {
    out << name << "=" << (value.has_value() ? std::to_string(*value) : "none") << " ";
}

void dumpWidget(
    const Interfaces& interfaces,
    const eld::interface::InterfaceFileWidget& widget,
    int x,
    int y,
    int depth,
    std::ostringstream& out,
    std::unordered_set<std::uint16_t>& stack
) {
    if (stack.contains(widget.id)) {
        return;
    }

    stack.insert(widget.id);

    const std::string indent(static_cast<std::size_t>(depth) * 2, ' ');

    out
        << indent
        << "widget "
        << "id=" << widget.id << " "
        << "type=" << static_cast<int>(widget.type) << " "
        << "typeName=" << typeName(widget.type) << " "
        << "absX=" << x << " "
        << "absY=" << y << " "
        << "width=" << widget.width << " "
        << "height=" << widget.height << " "
        << "actionType=" << static_cast<int>(widget.actionType) << " "
        << "contentType=" << widget.contentType << " "
        << "opacity=" << static_cast<int>(widget.opacity) << " ";

    appendOptionalU16(out, "parentId", widget.parentId);
    appendOptionalU16(out, "hoverId", widget.hoverId);

    out
        << "hidden=" << (widget.hidden ? "true" : "false") << " "
        << "scrollHeight=" << widget.scrollHeight << " "
        << "children=" << widget.children.size()
        << "\n";

    switch (widget.type) {
        case 2: {
            out
                << indent << "  type2 "
                << "inventorySwap=" << (widget.inventorySwap ? "true" : "false") << " "
                << "inventoryInterface=" << (widget.inventoryInterface ? "true" : "false") << " "
                << "inventoryUsable=" << (widget.inventoryUsable ? "true" : "false") << " "
                << "inventoryReplace=" << (widget.inventoryReplace ? "true" : "false") << " "
                << "paddingX=" << static_cast<int>(widget.inventoryPaddingX) << " "
                << "paddingY=" << static_cast<int>(widget.inventoryPaddingY) << " "
                << "itemSlots=" << widget.itemIds.size() << " "
                << "amountSlots=" << widget.itemAmounts.size() << " "
                << "spriteSlots=" << widget.inventorySprites.size() << " "
                << "actions=" << widget.actions.size()
                << "\n";

            for (const eld::interface::InterfaceFileSpriteSlot& sprite : widget.inventorySprites) {
                out
                    << indent << "    inventorySprite "
                    << "slot=" << static_cast<int>(sprite.slot) << " "
                    << "x=" << sprite.x << " "
                    << "y=" << sprite.y << " "
                    << "sprite=\"" << sprite.sprite << "\"\n";
            }

            for (std::size_t i = 0; i < widget.actions.size(); ++i) {
                if (!widget.actions[i].empty()) {
                    out << indent << "    action[" << i << "]=\"" << widget.actions[i] << "\"\n";
                }
            }

            break;
        }

        case 3:
            out
                << indent << "  type3 "
                << "filled=" << (widget.filled ? "true" : "false") << " "
                << "color=0x" << std::hex << widget.color << std::dec << " "
                << "secondaryColor=0x" << std::hex << widget.secondaryColor << std::dec << " "
                << "hoverColor=0x" << std::hex << widget.hoverColor << std::dec << " "
                << "secondaryHoverColor=0x" << std::hex << widget.secondaryHoverColor << std::dec
                << "\n";
            break;

        case 4:
            out
                << indent << "  type4 "
                << "centeredText=" << (widget.centeredText ? "true" : "false") << " "
                << "fontId=" << static_cast<int>(widget.fontId) << " "
                << "textShadow=" << (widget.textShadow ? "true" : "false") << " "
                << "color=0x" << std::hex << widget.color << std::dec << " "
                << "text=\"" << widget.text << "\" "
                << "secondaryText=\"" << widget.secondaryText << "\"\n";
            break;

        case 5:
            out
                << indent << "  type5 "
                << "sprite=\"" << widget.sprite << "\" "
                << "secondarySprite=\"" << widget.secondarySprite << "\"\n";
            break;

        case 6:
            out << indent << "  type6 ";
            appendOptionalU16(out, "modelId", widget.modelId);
            appendOptionalU16(out, "secondaryModelId", widget.secondaryModelId);
            appendOptionalU16(out, "animationId", widget.animationId);
            appendOptionalU16(out, "secondaryAnimationId", widget.secondaryAnimationId);
            out
                << "modelZoom=" << widget.modelZoom << " "
                << "modelRotationX=" << widget.modelRotationX << " "
                << "modelRotationY=" << widget.modelRotationY << "\n";
            break;

        case 7:
            out
                << indent << "  type7 "
                << "itemPaddingX=" << widget.itemPaddingX << " "
                << "itemPaddingY=" << widget.itemPaddingY << " "
                << "centeredText=" << (widget.centeredText ? "true" : "false") << " "
                << "fontId=" << static_cast<int>(widget.fontId) << " "
                << "textShadow=" << (widget.textShadow ? "true" : "false") << " "
                << "color=0x" << std::hex << widget.color << std::dec << "\n";
            break;

        case 8:
            out << indent << "  type8 tooltip=\"" << widget.tooltip << "\"\n";
            break;

        default:
            break;
    }

    if (!widget.selectedAction.empty() || !widget.spellName.empty()) {
        out
            << indent << "  spell "
            << "selectedAction=\"" << widget.selectedAction << "\" "
            << "spellName=\"" << widget.spellName << "\" "
            << "spellTargets=" << widget.spellTargets << "\n";
    }

    for (const eld::interface::InterfaceFileChild& child : widget.children) {
        const auto found = interfaces.byId.find(child.id);

        if (found == interfaces.byId.end()) {
            out
                << indent << "  missingChild "
                << "id=" << child.id << " "
                << "x=" << child.x << " "
                << "y=" << child.y << "\n";
            continue;
        }

        out
            << indent << "  childRef "
            << "id=" << child.id << " "
            << "x=" << child.x << " "
            << "y=" << child.y << "\n";

        dumpWidget(
            interfaces,
            *found->second,
            x + child.x,
            y + child.y,
            depth + 1,
            out,
            stack
        );
    }

    stack.erase(widget.id);
}

std::string makeInterfaceDump(const Interfaces& interfaces, int rootId) {
    const eld::interface::InterfaceFileWidget* root = findWidget(interfaces, rootId);

    std::ostringstream out;

    out
        << "interface dump\n"
        << "root=" << rootId << "\n"
        << "declaredCount=" << interfaces.file.declaredCount << "\n"
        << "loadedWidgets=" << interfaces.file.widgets.size() << "\n"
        << "containers=" << interfaces.containerIds.size() << "\n"
        << "--------------------\n";

    if (root == nullptr) {
        out << "root not found\n";
        return out.str();
    }

    std::unordered_set<std::uint16_t> stack;
    dumpWidget(interfaces, *root, 0, 0, 0, out, stack);
    return out.str();
}

std::filesystem::path writeInterfaceDumpFile(const std::string& dump, int rootId) {
    const std::filesystem::path path =
        std::filesystem::current_path() /
        ("interface_dump_" + std::to_string(rootId) + ".txt");

    std::ofstream file(path);
    file << dump;
    return path;
}

float modelAngle(std::uint16_t value) {
    return static_cast<float>(value) * ((Pi * 2.0f) / 2048.0f);
}

eld::math::Vec3 interfaceModelRotation(
    std::uint16_t pitchValue,
    std::uint16_t yawValue
) {
    const float pitch =
        modelAngle(pitchValue);

    const float yaw =
        modelAngle(yawValue);

    // RS317 applies yaw first, then pitch.
    //
    // Graphics has already converted source Y -> -Y, so the exact
    // target linear transform is:
    //
    //     Ry(yaw) * Rx(-pitch)
    //
    // Eldoria::Transform currently emits:
    //
    //     Rx(x) * Ry(y) * Rz(z)
    //
    // Convert the RS rotation matrix into that generic Euler order.
    const float sinPitch = std::sin(pitch);
    const float cosPitch = std::cos(pitch);
    const float sinYaw = std::sin(yaw);
    const float cosYaw = std::cos(yaw);

    const float y =
        std::asin(
            std::clamp(
                sinYaw * cosPitch,
                -1.0f,
                1.0f
            )
        );

    const float x =
        std::atan2(
            -sinPitch,
            cosPitch * cosYaw
        );

    const float z =
        std::atan2(
            -sinPitch * sinYaw,
            cosYaw
        );

    return {
        x,
        y,
        z
    };
}

struct RawProjectedVertex {
    ImVec2 point;
    float depth = 0.0f;
    bool valid = false;
};

std::vector<RawProjectedVertex> projectRawModel317(
    const eld::model::ModelMesh& mesh,
    const eld::interface::InterfaceFileWidget& widget,
    const ImVec2& min,
    const ImVec2& max
) {
    std::vector<RawProjectedVertex> projected;
    projected.reserve(mesh.vertices.size());

    const float centerX =
        min.x + (max.x - min.x) * 0.5f;

    const float centerY =
        min.y + (max.y - min.y) * 0.5f;

    const float pitch =
        modelAngle(widget.modelRotationX);

    const float yaw =
        modelAngle(widget.modelRotationY);

    const float sinPitch = std::sin(pitch);
    const float cosPitch = std::cos(pitch);
    const float sinYaw = std::sin(yaw);
    const float cosYaw = std::cos(yaw);

    const float zoom =
        static_cast<float>(widget.modelZoom);

    const float translateY =
        sinPitch * zoom;

    const float translateZ =
        cosPitch * zoom;

    for (const eld::model::Vertex& vertex : mesh.vertices) {
        float x = static_cast<float>(vertex.x);
        float y = static_cast<float>(vertex.y);
        float z = static_cast<float>(vertex.z);

        if (widget.modelRotationY != 0) {
            const float newX =
                z * sinYaw +
                x * cosYaw;

            z =
                z * cosYaw -
                x * sinYaw;

            x = newX;
        }

        y += translateY;
        z += translateZ;

        const float newY =
            y * cosPitch -
            z * sinPitch;

        z =
            y * sinPitch +
            z * cosPitch;

        y = newY;

        if (z <= 1.0f) {
            projected.push_back({
                .point = ImVec2(
                    -100000.0f,
                    -100000.0f
                ),
                .depth = z,
                .valid = false
            });

            continue;
        }

        projected.push_back({
            .point = ImVec2(
                centerX + (x * 512.0f) / z,
                centerY + (y * 512.0f) / z
            ),
            .depth = z,
            .valid = true
        });
    }

    return projected;
}

void drawRawModelWireframe317(
    const eld::model::ModelMesh& mesh,
    const eld::interface::InterfaceFileWidget& widget,
    const ImVec2& min,
    const ImVec2& max,
    ImDrawList* drawList
) {
    const std::vector<RawProjectedVertex> projected =
        projectRawModel317(
            mesh,
            widget,
            min,
            max
        );

    constexpr ImU32 WireColor =
        IM_COL32(0, 255, 255, 220);

    int drawn = 0;

    for (const eld::model::Face& face : mesh.faces) {
        if (
            face.a >= projected.size() ||
            face.b >= projected.size() ||
            face.c >= projected.size()
        ) {
            continue;
        }

        const RawProjectedVertex& a =
            projected.at(face.a);

        const RawProjectedVertex& b =
            projected.at(face.b);

        const RawProjectedVertex& c =
            projected.at(face.c);

        if (!a.valid || !b.valid || !c.valid) {
            continue;
        }

        drawList->AddLine(
            a.point,
            b.point,
            WireColor,
            1.0f
        );

        drawList->AddLine(
            b.point,
            c.point,
            WireColor,
            1.0f
        );

        drawList->AddLine(
            c.point,
            a.point,
            WireColor,
            1.0f
        );

        if (++drawn > 6000) {
            break;
        }
    }
}

class InterfaceModelRenderer {
public:
    InterfaceModelRenderer(
        SDL_Renderer* renderer,
        eld::graphics::GraphicsResources& resources,
        eld::model::ModelRepository& modelRepository
    )
        : renderer_(renderer),
          resources_(resources),
          modelRepository_(modelRepository),
          backend_(nullptr) {
        backend_.setClearColor({0, 0, 0, 0});
    }

    ~InterfaceModelRenderer() {
        for (auto& [key, texture] : textures_) {
            if (texture.texture != nullptr) {
                SDL_DestroyTexture(texture.texture);
            }
        }
    }

    SpriteTexture* find(
        const eld::interface::InterfaceFileWidget& widget
    ) {
        if (!widget.modelId.has_value() || renderer_ == nullptr) {
            return nullptr;
        }

        const std::string key =
            std::to_string(widget.id) + ":" +
            std::to_string(*widget.modelId) + ":" +
            std::to_string(widget.modelZoom) + ":" +
            std::to_string(widget.modelRotationX) + ":" +
            std::to_string(widget.modelRotationY);

        if (auto cached = textures_.find(key); cached != textures_.end()) {
            return &cached->second;
        }

        if (missing_.contains(key)) {
            return nullptr;
        }

        try {
            const eld::graphics::ModelHandle model =
                resources_.resolveModel(
                    *widget.modelId
                );

            eld::render::RenderScene scene;

            scene.camera.position = {0.0f, 0.0f, 0.0f};
            scene.camera.rotation = {0.0f, 0.0f, 0.0f};

            // Preserve the 317 interface projection scale:
            // screen = center + coordinate * 512 / depth.
            scene.camera.verticalFov =
                2.0f * std::atan(
                    static_cast<float>(RenderTargetSize) /
                    1024.0f
                );

            scene.camera.nearPlane = 1.0f;
            scene.camera.farPlane = 10000.0f;
            scene.camera.viewportWidth = RenderTargetSize;
            scene.camera.viewportHeight = RenderTargetSize;

            eld::render::RenderObject object;
            object.model = model;

            object.transform.rotation =
                interfaceModelRotation(
                    widget.modelRotationX,
                    widget.modelRotationY
                );

            // The old 317:
            //   yaw
            //   translate(sin(pitch)*zoom, cos(pitch)*zoom)
            //   pitch
            // reduces to rotated geometry translated +zoom on Z.
            object.transform.position = {
                0.0f,
                0.0f,
                static_cast<float>(widget.modelZoom)
            };

            scene.objects.push_back(object);

            pipeline_.render(
                scene,
                resources_,
                backend_
            );

            const eld::render::ColorBuffer& color =
                backend_.framebuffer().color();

            SDL_Texture* texture =
                SDL_CreateTexture(
                    renderer_,
                    SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_STATIC,
                    static_cast<int>(RenderTargetSize),
                    static_cast<int>(RenderTargetSize)
                );

            if (texture == nullptr) {
                missing_.insert(key);
                return nullptr;
            }

            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

            if (!SDL_UpdateTexture(
                    texture,
                    nullptr,
                    color.data(),
                    static_cast<int>(
                        RenderTargetSize *
                        sizeof(eld::render::ColorPixel)
                    )
                )) {
                SDL_DestroyTexture(texture);
                missing_.insert(key);
                return nullptr;
            }

            auto inserted =
                textures_.emplace(
                    key,
                    SpriteTexture{
                        .texture = texture,
                        .width = static_cast<int>(RenderTargetSize),
                        .height = static_cast<int>(RenderTargetSize)
                    }
                );

            return &inserted.first->second;
        }
        catch (const std::exception& ex) {
            std::printf(
                "failed to render interface model: widget=%u model=%u error=%s\n",
                static_cast<unsigned>(widget.id),
                static_cast<unsigned>(*widget.modelId),
                ex.what()
            );

            missing_.insert(key);
            return nullptr;
        }
    }

    const eld::model::ModelMesh* findRaw(
        const eld::interface::InterfaceFileWidget& widget
    ) {
        if (!widget.modelId.has_value()) {
            return nullptr;
        }

        const std::uint16_t id =
            *widget.modelId;

        if (
            auto cached =
                rawMeshes_.find(id);
            cached != rawMeshes_.end()
        ) {
            return &cached->second;
        }

        if (rawMissing_.contains(id)) {
            return nullptr;
        }

        try {
            std::optional<eld::model::Model> model =
                modelRepository_.find(id);

            if (!model.has_value()) {
                rawMissing_.insert(id);
                return nullptr;
            }

            auto inserted =
                rawMeshes_.emplace(
                    id,
                    model->mesh
                );

            return &inserted.first->second;
        }
        catch (const std::exception& ex) {
            std::printf(
                "failed to load raw interface model: model=%u error=%s\n",
                static_cast<unsigned>(id),
                ex.what()
            );

            rawMissing_.insert(id);
            return nullptr;
        }
    }

private:
    static constexpr std::uint32_t RenderTargetSize = 512;

    SDL_Renderer* renderer_ = nullptr;
    eld::graphics::GraphicsResources& resources_;
    eld::model::ModelRepository& modelRepository_;

    eld::render::RenderPipeline pipeline_;
    eld::render::SoftwareRenderBackend backend_;

    std::unordered_map<std::string, SpriteTexture> textures_;
    std::unordered_set<std::string> missing_;

    std::unordered_map<
        std::uint16_t,
        eld::model::ModelMesh
    > rawMeshes_;

    std::unordered_set<std::uint16_t> rawMissing_;
};

void drawType2CustomSprites(
    const eld::interface::InterfaceFileWidget& widget,
    const ImVec2& min,
    SpriteTextureCache& spriteTextures,
    ImDrawList* drawList
) {
    for (const eld::interface::InterfaceFileSpriteSlot& slot : widget.inventorySprites) {
        SpriteTexture* texture = spriteTextures.find(slot.sprite);

        if (texture == nullptr || texture->texture == nullptr) {
            continue;
        }

        const ImVec2 spriteMin(min.x + static_cast<float>(slot.x), min.y + static_cast<float>(slot.y));
        const ImVec2 spriteMax(spriteMin.x + static_cast<float>(texture->width), spriteMin.y + static_cast<float>(texture->height));

        drawList->AddImage(
            reinterpret_cast<ImTextureID>(texture->texture),
            spriteMin,
            spriteMax
        );
    }
}

void drawType2(
    const State& state,
    const eld::interface::InterfaceFileWidget& widget,
    const ImVec2& min,
    SpriteTextureCache& spriteTextures,
    ImDrawList* drawList
) {
    const int columns = static_cast<int>(widget.width);
    const int rows = static_cast<int>(widget.height);

    if (columns <= 0 || rows <= 0) {
        return;
    }

    drawType2CustomSprites(widget, min, spriteTextures, drawList);

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const int slot = row * columns + column;

            const ImVec2 slotMin(
                min.x + static_cast<float>(column * (SlotSize + static_cast<int>(widget.inventoryPaddingX))),
                min.y + static_cast<float>(row * (SlotSize + static_cast<int>(widget.inventoryPaddingY)))
            );

            const ImVec2 slotMax(slotMin.x + SlotSize, slotMin.y + SlotSize);

            std::uint16_t itemId = 0;
            std::uint16_t itemAmount = 0;

            if (static_cast<std::size_t>(slot) < widget.itemIds.size()) {
                itemId = widget.itemIds[static_cast<std::size_t>(slot)];
            }

            if (static_cast<std::size_t>(slot) < widget.itemAmounts.size()) {
                itemAmount = widget.itemAmounts[static_cast<std::size_t>(slot)];
            }

            if (itemId == 0) {
                if (state.showEmptyInventorySlots) {
                    drawList->AddRect(slotMin, slotMax, IM_COL32(255, 170, 55, 70));
                }

                continue;
            }

            drawList->AddRectFilled(slotMin, slotMax, IM_COL32(255, 170, 55, 65));
            drawList->AddRect(slotMin, slotMax, IM_COL32(255, 190, 80, 180));

            if (state.showItemText) {
                const std::string idText = std::to_string(itemId);
                drawList->AddText(ImVec2(slotMin.x + 2.0f, slotMin.y + 2.0f), IM_COL32(255, 255, 255, 255), idText.c_str());

                if (itemAmount > 1) {
                    const std::string amountText = "x" + std::to_string(itemAmount);
                    drawList->AddText(ImVec2(slotMin.x + 2.0f, slotMin.y + 16.0f), IM_COL32(255, 255, 120, 255), amountText.c_str());
                }
            }
        }
    }
}

void drawType3(
    const eld::interface::InterfaceFileWidget& widget,
    const ImVec2& min,
    const ImVec2& max,
    ImDrawList* drawList
) {
    if (widget.filled) {
        drawList->AddRectFilled(min, max, interfaceColor(widget.color, widget.opacity));
    } else {
        drawList->AddRect(min, max, interfaceColor(widget.color, widget.opacity));
    }
}

void drawType4(
    const eld::interface::InterfaceFileWidget& widget,
    const ImVec2& min,
    const ImVec2& max,
    ImDrawList* drawList
) {
    if (widget.text.empty()) {
        return;
    }

    const ImU32 color = interfaceColor(widget.color);
    const float lineHeight = ImGui::GetFontSize();
    float y = min.y;

    for (const std::string& line : splitText(widget.text)) {
        float x = min.x;

        if (widget.centeredText) {
            const ImVec2 textSize = ImGui::CalcTextSize(line.c_str());
            x = min.x + ((max.x - min.x) - textSize.x) * 0.5f;
        }

        if (widget.textShadow) {
            drawList->AddText(ImVec2(x + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 255), line.c_str());
        }

        drawList->AddText(ImVec2(x, y), color, line.c_str());
        y += lineHeight;
    }
}

void drawType5(
    const eld::interface::InterfaceFileWidget& widget,
    const ImVec2& min,
    const ImVec2& max,
    SpriteTextureCache& spriteTextures,
    ImDrawList* drawList
) {
    const std::string spriteText = widget.sprite.empty() ? widget.secondarySprite : widget.sprite;
    SpriteTexture* texture = spriteTextures.find(spriteText);

    if (texture == nullptr || texture->texture == nullptr) {
        return;
    }

    const ImVec2 imageMax(
    min.x + static_cast<float>(texture->width),
    min.y + static_cast<float>(texture->height)
    );

    drawList->AddImage(
        reinterpret_cast<ImTextureID>(texture->texture),
        min,
        imageMax
    );
}

void drawType6(
    const State& state,
    const eld::interface::InterfaceFileWidget& widget,
    const ImVec2& min,
    const ImVec2& max,
    InterfaceModelRenderer& modelRenderer,
    ImDrawList* drawList
) {
    SpriteTexture* texture = modelRenderer.find(widget);

    const bool foundModel =
        texture != nullptr &&
        texture->texture != nullptr;

    if (foundModel) {
        const ImVec2 center(
            min.x + (max.x - min.x) * 0.5f,
            min.y + (max.y - min.y) * 0.5f
        );

        const ImVec2 imageMin(
            center.x - static_cast<float>(texture->width) * 0.5f,
            center.y - static_cast<float>(texture->height) * 0.5f
        );

        const ImVec2 imageMax(
            imageMin.x + static_cast<float>(texture->width),
            imageMin.y + static_cast<float>(texture->height)
        );

        // ImGui only composites pixels produced by Eldoria::Render.
        drawList->AddImage(
            reinterpret_cast<ImTextureID>(texture->texture),
            imageMin,
            imageMax
        );
    }

    if (state.showRawModelWireframe) {
        const eld::model::ModelMesh* raw =
            modelRenderer.findRaw(widget);

        if (raw != nullptr) {
            drawRawModelWireframe317(
                *raw,
                widget,
                min,
                max,
                drawList
            );
        }
    }

    if (!state.showModelText) {
        return;
    }

    const std::string line1 =
        "w" + std::to_string(widget.id) +
        " m" + optionalU16Text(widget.modelId);

    const std::string line2 =
        foundModel ? "renderer" : "missing";

    const std::string line3 =
        "z" + std::to_string(widget.modelZoom) +
        " rx" + std::to_string(widget.modelRotationX) +
        " ry" + std::to_string(widget.modelRotationY);

    drawList->AddText(
        ImVec2(min.x + 1.0f, min.y - 34.0f),
        IM_COL32(255, 255, 255, 255),
        line1.c_str()
    );

    drawList->AddText(
        ImVec2(min.x + 1.0f, min.y - 22.0f),
        foundModel
            ? IM_COL32(190, 255, 190, 255)
            : IM_COL32(255, 120, 120, 255),
        line2.c_str()
    );

    drawList->AddText(
        ImVec2(min.x + 1.0f, min.y - 10.0f),
        IM_COL32(230, 205, 255, 255),
        line3.c_str()
    );
}

void drawContent(
    const State& state,
    const eld::interface::InterfaceFileWidget& widget,
    const ImVec2& min,
    const ImVec2& max,
    SpriteTextureCache& spriteTextures,
    InterfaceModelRenderer& modelRenderer,
    ImDrawList* drawList
) {
    if (!enabled(state.showContent, widget.type)) {
        return;
    }

    switch (widget.type) {
        case 2: drawType2(state, widget, min, spriteTextures, drawList); break;
        case 3: drawType3(widget, min, max, drawList); break;
        case 4: drawType4(widget, min, max, drawList); break;
        case 5: drawType5(widget, min, max, spriteTextures, drawList); break;
        case 6: drawType6(state, widget, min, max, modelRenderer, drawList); break;
        default: break;
    }
}

void drawWidget(
    const Interfaces& interfaces,
    const State& state,
    const eld::interface::InterfaceFileWidget& widget,
    int x,
    int y,
    const ImVec2& origin,
    SpriteTextureCache& spriteTextures,
    InterfaceModelRenderer& modelRenderer,
    ImDrawList* drawList,
    std::unordered_set<std::uint16_t>& stack
) {
    if (stack.contains(widget.id) || (state.respectHidden && widget.hidden)) {
        return;
    }

    stack.insert(widget.id);

    const ImVec2 size = widgetSize(widget);
    const ImVec2 min(origin.x + static_cast<float>(x), origin.y + static_cast<float>(y));
    const ImVec2 max(min.x + size.x, min.y + size.y);

    drawContent(state, widget, min, max, spriteTextures, modelRenderer, drawList);

    if (enabled(state.showOutline, widget.type)) {
        drawList->AddRect(min, max, typeColor(widget.type));
    }

    const bool clip =
        state.clipContainers &&
        widget.type == 0 &&
        widget.width > 0 &&
        widget.height > 0;

    if (clip) {
        drawList->PushClipRect(min, max, true);
    }

    const int scroll = widget.type == 0 ? scrollOffset(state, widget) : 0;

    for (const eld::interface::InterfaceFileChild& child : widget.children) {
        const auto found = interfaces.byId.find(child.id);

        if (found == interfaces.byId.end()) {
            continue;
        }

        drawWidget(
            interfaces,
            state,
            *found->second,
            x + child.x,
            y + child.y - scroll,
            origin,
            spriteTextures,
            modelRenderer,
            drawList,
            stack
        );
    }

    if (clip) {
        drawList->PopClipRect();
    }

    stack.erase(widget.id);
}

void drawContainerNav(const Interfaces& interfaces, State& state) {
    ImGui::TextUnformatted("Container nav");

    const int current = containerIndex(interfaces, state.rootId);

    if (current >= 0) {
        ImGui::Text("%d / %d", current + 1, static_cast<int>(interfaces.containerIds.size()));
    } else {
        ImGui::Text("? / %d", static_cast<int>(interfaces.containerIds.size()));
    }

    if (ImGui::Button("|<")) goToFirstContainer(interfaces, state);
    ImGui::SameLine();
    if (ImGui::Button("<")) goToContainer(interfaces, state, -1);
    ImGui::SameLine();
    if (ImGui::Button(">")) goToContainer(interfaces, state, 1);
    ImGui::SameLine();
    if (ImGui::Button(">|")) goToLastContainer(interfaces, state);

    const eld::interface::InterfaceFileWidget* widget = findWidget(interfaces, state.rootId);

    if (widget == nullptr) {
        ImGui::TextUnformatted("root not found");
        return;
    }

    ImGui::Text(
        "id=%u type=%u children=%d",
        static_cast<unsigned>(widget->id),
        static_cast<unsigned>(widget->type),
        static_cast<int>(widget->children.size())
    );

    ImGui::Text(
        "size=%ux%u scroll=%u hidden=%s",
        static_cast<unsigned>(widget->width),
        static_cast<unsigned>(widget->height),
        static_cast<unsigned>(widget->scrollHeight),
        widget->hidden ? "yes" : "no"
    );

    if (widget->type == 0) {
        const int maxScroll = scrollMax(*widget);
        int& scroll = state.scrollOffsets[widget->id];
        scroll = std::clamp(scroll, 0, maxScroll);
        ImGui::SliderInt("root scroll", &scroll, 0, maxScroll);
    }
}

void drawTypeControls(State& state) {
    ImGui::TextUnformatted("Types");
    ImGui::Separator();

    if (ImGui::Button("content on")) setAll(state.showContent, true);
    ImGui::SameLine();
    if (ImGui::Button("off##content")) setAll(state.showContent, false);

    if (ImGui::Button("outline on")) setAll(state.showOutline, true);
    ImGui::SameLine();
    if (ImGui::Button("off##outline")) setAll(state.showOutline, false);

    ImGui::Columns(3, "type-controls", false);
    ImGui::TextUnformatted("type");
    ImGui::NextColumn();
    ImGui::TextUnformatted("content");
    ImGui::NextColumn();
    ImGui::TextUnformatted("outline");
    ImGui::NextColumn();
    ImGui::Separator();

    for (std::size_t type = 0; type < state.showContent.size(); ++type) {
        const auto widgetType = static_cast<std::uint8_t>(type);
        const ImVec2 colorMin = ImGui::GetCursorScreenPos();

        ImGui::GetWindowDrawList()->AddRectFilled(
            colorMin,
            ImVec2(colorMin.x + 10.0f, colorMin.y + 10.0f),
            typeColor(widgetType)
        );

        ImGui::Dummy(ImVec2(14.0f, 0.0f));
        ImGui::SameLine();
        ImGui::Text("%zu %s", type, typeName(widgetType));
        ImGui::NextColumn();

        const std::string contentId = "##content-" + std::to_string(type);
        ImGui::Checkbox(contentId.c_str(), &state.showContent[type]);
        ImGui::NextColumn();

        const std::string outlineId = "##outline-" + std::to_string(type);
        ImGui::Checkbox(outlineId.c_str(), &state.showOutline[type]);
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
}

void drawDumpPanel(const Interfaces& interfaces, State& state) {
    if (ImGui::Button("dump interface tree")) {
        state.interfaceDump = makeInterfaceDump(interfaces, state.rootId);

        const std::filesystem::path path =
            writeInterfaceDumpFile(state.interfaceDump, state.rootId);

        state.interfaceDumpPath = path.string();

        ImGui::SetClipboardText(state.interfaceDump.c_str());
        std::printf("%s\n", state.interfaceDump.c_str());
    }

    if (!state.interfaceDumpPath.empty()) {
        ImGui::TextWrapped("dumped: %s", state.interfaceDumpPath.c_str());
    }

    if (!state.interfaceDump.empty()) {
        ImGui::BeginChild(
            "interface-dump-preview",
            ImVec2(0.0f, 220.0f),
            true,
            ImGuiWindowFlags_HorizontalScrollbar
        );

        ImGui::TextUnformatted(state.interfaceDump.c_str());
        ImGui::EndChild();
    }
}

void drawLeftPanel(const Interfaces& interfaces, State& state) {
    ImGui::InputInt("root", &state.rootId);
    ImGui::Spacing();

    drawContainerNav(interfaces, state);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Checkbox("clip containers", &state.clipContainers);
    ImGui::Checkbox("respect hidden", &state.respectHidden);
    ImGui::Checkbox("empty inventory slots", &state.showEmptyInventorySlots);
    ImGui::Checkbox("item text", &state.showItemText);

    ImGui::Spacing();
    ImGui::TextUnformatted("Models");
    ImGui::Separator();

    ImGui::Checkbox("model text", &state.showModelText);
    ImGui::Checkbox(
        "raw 317 wireframe",
        &state.showRawModelWireframe
    );
    ImGui::TextUnformatted("cyan = old raw 317 projection");

    ImGui::Spacing();
    drawTypeControls(state);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    drawDumpPanel(interfaces, state);
}

void drawCanvas(
    const Interfaces& interfaces,
    const State& state,
    SpriteTextureCache& spriteTextures,
    InterfaceModelRenderer& modelRenderer
) {
    const ImVec2 canvasStart = ImGui::GetCursorScreenPos();
    const ImVec2 canvasSize(1600.0f, 1000.0f);
    const ImVec2 origin(canvasStart.x + 80.0f, canvasStart.y + 80.0f);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
        canvasStart,
        ImVec2(canvasStart.x + canvasSize.x, canvasStart.y + canvasSize.y),
        IM_COL32(18, 20, 24, 255)
    );

    const eld::interface::InterfaceFileWidget* root = findWidget(interfaces, state.rootId);

    if (root == nullptr) {
        drawList->AddText(origin, IM_COL32(255, 120, 120, 255), "root not found");
    } else {
        std::unordered_set<std::uint16_t> stack;

        drawWidget(
            interfaces,
            state,
            *root,
            0,
            0,
            origin,
            spriteTextures,
            modelRenderer,
            drawList,
            stack
        );
    }

    ImGui::Dummy(canvasSize);
}

void drawUi(
    const Interfaces& interfaces,
    State& state,
    SpriteTextureCache& spriteTextures,
    InterfaceModelRenderer& modelRenderer
) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin(
        "Interface Debug",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings
    );

    ImGui::BeginChild("left", ImVec2(340.0f, 0.0f), true);
    drawLeftPanel(interfaces, state);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true);
    drawCanvas(interfaces, state, spriteTextures, modelRenderer);
    ImGui::EndChild();

    ImGui::End();
}

int runApp(const std::filesystem::path& cacheRoot) {
    eld::cache::Cache cache(cacheRoot);

    eld::cache::Store configStore = cache.open(eld::cache::IndexId::Config);
    eld::cache::Store modelStore = cache.open(eld::cache::IndexId::Models);

    Interfaces interfaces = loadInterfaces(configStore);

    eld::sprite::SpriteRepository spriteRepository(configStore, MediaArchiveFileId);
    eld::texture::TextureRepository textureRepository(configStore);
    eld::model::ModelRepository modelRepository(modelStore);

    eld::graphics::GraphicsResources graphicsResources(
        modelRepository,
        textureRepository
    );

    State state;

    eld::platform::SdlContext sdl("Interface Debug", 1400, 900);

    SDL_Window* window = sdl.window();
    SDL_Renderer* renderer = sdl.renderer();

    if (!window || !renderer) {
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
    eld::platform::imgui::applyImGuiTheme();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    SpriteTextureCache spriteTextures(renderer, spriteRepository);
    InterfaceModelRenderer modelRenderer(
        renderer,
        graphicsResources,
        modelRepository
    );

    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        SDL_SetRenderDrawColor(renderer, 18, 20, 24, 255);
        SDL_RenderClear(renderer);

        drawUi(interfaces, state, spriteTextures, modelRenderer);

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::fprintf(
                stderr,
                "usage: %s <cache-root>\n",
                argc > 0 ? argv[0] : "interface_probe"
            );

            return 1;
        }

        return runApp(std::filesystem::path(argv[1]));
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "fatal: %s\n", ex.what());
        return 1;
    }
}
