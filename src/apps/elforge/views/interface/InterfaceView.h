#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <unordered_set>
#include <utility>

#include "views/interface/InterfaceViewState.h"

#include "repositories/WidgetRepository.h"

namespace eld::elforge {

class InterfaceView {
public:
    std::optional<InterfaceViewState> build(
        const eld::interface::Widget& root,
        const eld::interface::WidgetRepository& repository
    ) const {
        std::unordered_set<std::uint16_t> stack;

        std::optional<InterfaceViewNode> rootNode =
            buildNode(
                root,
                repository,
                0,
                0,
                stack
            );

        if (!rootNode.has_value()) {
            return std::nullopt;
        }

        InterfaceViewState state;

        state.rootId =
            root.id;

        const auto [width, height] =
            viewSize(
                root
            );

        state.width =
            width;

        state.height =
            height;

        state.root =
            std::move(*rootNode);

        return state;
    }

private:

    static std::pair<int, int> viewSize(
        const eld::interface::Widget& widget
    ) {
        constexpr int SlotSize = 32;

        if (widget.type == 2) {
            const int columns =
                static_cast<int>(
                    widget.width
                );

            const int rows =
                static_cast<int>(
                    widget.height
                );

            return {
                std::max(
                    columns * SlotSize +
                        static_cast<int>(
                            widget.inventoryPaddingX
                        ) *
                        std::max(columns - 1, 0),
                    1
                ),
                std::max(
                    rows * SlotSize +
                        static_cast<int>(
                            widget.inventoryPaddingY
                        ) *
                        std::max(rows - 1, 0),
                    1
                )
            };
        }

        if (widget.type == 7) {
            const int columns =
                static_cast<int>(
                    widget.width
                );

            const int rows =
                static_cast<int>(
                    widget.height
                );

            return {
                std::max(
                    columns * SlotSize +
                        static_cast<int>(
                            widget.itemPaddingX
                        ) *
                        std::max(columns - 1, 0),
                    1
                ),
                std::max(
                    rows * SlotSize +
                        static_cast<int>(
                            widget.itemPaddingY
                        ) *
                        std::max(rows - 1, 0),
                    1
                )
            };
        }

        return {
            std::max(
                static_cast<int>(
                    widget.width
                ),
                1
            ),
            std::max(
                static_cast<int>(
                    widget.height
                ),
                1
            )
        };
    }

    static constexpr float Pi =
        3.14159265358979323846f;

    static float modelAngle(
        std::uint16_t value
    ) {
        return
            static_cast<float>(value) *
            ((Pi * 2.0f) / 2048.0f);
    }

    static eld::math::Vec3 modelRotation(
        std::uint16_t pitchValue,
        std::uint16_t yawValue
    ) {
        const float pitch =
            modelAngle(pitchValue);

        const float yaw =
            modelAngle(yawValue);

        // RS317 applies yaw first, then pitch.
        //
        // Graphics has already converted source Y -> -Y. The exact
        // target linear transform is therefore:
        //
        //     Ry(yaw) * Rx(-pitch)
        //
        // Eldoria's generic Transform emits Rx * Ry * Rz, so convert
        // the source rotation matrix into that Euler order here.
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

    static std::optional<InterfaceViewModel>
    buildModel(
        const eld::interface::Widget& widget
    ) {
        if (
            widget.type != 6 ||
            !widget.modelId.has_value()
        ) {
            return std::nullopt;
        }

        InterfaceViewModel model;

        model.modelId =
            *widget.modelId;

        model.rotation =
            modelRotation(
                widget.modelRotationX,
                widget.modelRotationY
            );

        model.depth =
            static_cast<float>(
                widget.modelZoom
            );

        return model;
    }

    std::optional<InterfaceViewNode> buildNode(
        const eld::interface::Widget& widget,
        const eld::interface::WidgetRepository& repository,
        int x,
        int y,
        std::unordered_set<std::uint16_t>& stack
    ) const {
        if (stack.contains(widget.id)) {
            return std::nullopt;
        }

        stack.insert(
            widget.id
        );

        InterfaceViewNode node;

        node.widget =
            widget;

        node.x = x;
        node.y = y;

        node.model =
            buildModel(
                widget
            );

        node.children.reserve(
            widget.children.size()
        );

        for (
            const eld::interface::WidgetChild& child :
            widget.children
        ) {
            const eld::interface::Widget* childWidget =
                repository.find(
                    child.id
                );

            if (childWidget == nullptr) {
                continue;
            }

            std::optional<InterfaceViewNode> childNode =
                buildNode(
                    *childWidget,
                    repository,
                    child.x,
                    child.y,
                    stack
                );

            if (childNode.has_value()) {
                node.children.push_back(
                    std::move(*childNode)
                );
            }
        }

        stack.erase(
            widget.id
        );

        return node;
    }
};

}
