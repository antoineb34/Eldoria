#include "views/midi/MidiViewPanel.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "midi/MidiPlayer.h"

#include "midi/MidiFile.h"
#include "views/midi/MidiViewState.h"

namespace eld::elforge {

void MidiViewPanel::renderControls(
    const eld::midi::MidiFile* midi,
    MidiViewState& viewState,
    eld::audio::MidiPlayer& midiPlayer
) {
    if (midi == nullptr) {
        return;
    }

    ImGui::Text(
        "MIDI %u",
        static_cast<unsigned int>(
            midi->id
        )
    );

    ImGui::SameLine();

    ImGui::TextDisabled(
        "| %s",
        eld::audio::midiPlayerStateName(
            midiPlayer.state()
        )
    );

    if (
        midiPlayer.isAvailable() &&
        midiPlayer.hasMidi()
    ) {
        const eld::audio::MidiPlayerState playbackState =
            midiPlayer.state();

        const bool canPlay =
            playbackState !=
                eld::audio::MidiPlayerState::Playing;

        const bool canPause =
            playbackState ==
                eld::audio::MidiPlayerState::Playing;

        if (!canPlay) {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Play")) {
            if (!midiPlayer.play()) {
                viewState.playbackStatus =
                    midiPlayer.statusMessage();
            }
            else {
                viewState.playbackStatus =
                    "Playing";
            }
        }

        if (!canPlay) {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (!canPause) {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Pause")) {
            midiPlayer.pause();

            viewState.playbackStatus =
                midiPlayer.statusMessage();
        }

        if (!canPause) {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Stop")) {
            midiPlayer.stop();

            viewState.seekTick = 0;
            viewState.seekActive = false;

            viewState.playbackStatus =
                midiPlayer.statusMessage();
        }

        float volume =
            midiPlayer.volume();

        ImGui::SetNextItemWidth(
            std::min(
                ImGui::GetContentRegionAvail().x,
                360.0f
            )
        );

        if (ImGui::SliderFloat(
                "Volume",
                &volume,
                0.0f,
                1.0f,
                "%.2f"
            )) {
            midiPlayer.setVolume(
                volume
            );
        }

        const int currentTick =
            midiPlayer.currentTick();

        const int totalTicks =
            midiPlayer.totalTicks();

        if (!viewState.seekActive) {
            viewState.seekTick =
                currentTick;
        }

        if (totalTicks > 0) {
            ImGui::SetNextItemWidth(
                std::max(
                    ImGui::GetContentRegionAvail().x -
                        8.0f,
                    100.0f
                )
            );

            ImGui::SliderInt(
                "Position",
                &viewState.seekTick,
                0,
                totalTicks,
                "%d ticks"
            );

            if (ImGui::IsItemActive()) {
                viewState.seekActive = true;
            }

            if (
                ImGui::IsItemDeactivatedAfterEdit()
            ) {
                midiPlayer.seek(
                    viewState.seekTick
                );

                viewState.playbackStatus =
                    midiPlayer.statusMessage();

                viewState.seekActive = false;
            }

            ImGui::Text(
                "Tick %d / %d",
                currentTick,
                totalTicks
            );

            const int bpm =
                midiPlayer.currentBpm();

            if (bpm > 0) {
                ImGui::SameLine();

                ImGui::TextDisabled(
                    "| %d BPM",
                    bpm
                );
            }
        }
    }
    else {
        ImGui::TextWrapped(
            "%s",
            midiPlayer.statusMessage().empty()
                ? "MIDI playback is unavailable"
                : midiPlayer.statusMessage().c_str()
        );
    }

    if (!midiPlayer.soundFontPath().empty()) {
        ImGui::TextDisabled(
            "FluidSynth | %s",
            midiPlayer.soundFontPath()
                .string()
                .c_str()
        );
    }

    if (!viewState.playbackStatus.empty()) {
        ImGui::TextDisabled(
            "%s",
            viewState.playbackStatus.c_str()
        );
    }
}


void MidiViewPanel::renderVisualization(
    MidiViewState& viewState,
    eld::audio::MidiPlayer& midiPlayer,
    const ImVec2& size
) {
    const ImVec2 origin =
        ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton(
        "##MidiActivityViewport",
        size
    );

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    const ImU32 background =
        ImGui::GetColorU32(
            ImGuiCol_FrameBg
        );

    const ImU32 border =
        ImGui::GetColorU32(
            ImGuiCol_Border
        );

    const ImU32 waveform =
        ImGui::GetColorU32(
            ImGuiCol_PlotHistogram
        );

    const ImU32 centerLine =
        ImGui::GetColorU32(
            ImGuiCol_Separator
        );

    const ImU32 playhead =
        ImGui::GetColorU32(
            ImGuiCol_SliderGrabActive
        );

    const ImVec2 end{
        origin.x + size.x,
        origin.y + size.y
    };

    drawList->AddRectFilled(
        origin,
        end,
        background
    );

    drawList->AddRect(
        origin,
        end,
        border
    );

    const float centerY =
        origin.y +
        size.y * 0.5f;

    drawList->AddLine(
        ImVec2(origin.x, centerY),
        ImVec2(end.x, centerY),
        centerLine
    );

    for (int grid = 1; grid < 8; ++grid) {
        const float x =
            origin.x +
            size.x *
                static_cast<float>(grid) /
                8.0f;

        drawList->AddLine(
            ImVec2(x, origin.y),
            ImVec2(x, end.y),
            centerLine
        );
    }

    if (!viewState.activity.empty()) {
        const float binWidth =
            size.x /
            static_cast<float>(
                viewState.activity.size()
            );

        const float maximumHalfHeight =
            std::max(
                size.y * 0.40f,
                1.0f
            );

        for (
            std::size_t index = 0;
            index < viewState.activity.size();
            ++index
        ) {
            const float x0 =
                origin.x +
                static_cast<float>(index) *
                    binWidth;

            const float x1 =
                std::max(
                    x0 + 1.0f,
                    origin.x +
                        static_cast<float>(
                            index + 1
                        ) *
                            binWidth -
                        1.0f
                );

            const float halfHeight =
                viewState.activity[index] *
                maximumHalfHeight;

            drawList->AddRectFilled(
                ImVec2(
                    x0,
                    centerY - halfHeight
                ),
                ImVec2(
                    x1,
                    centerY + halfHeight
                ),
                waveform
            );
        }
    }

    const int playerTotalTicks =
        midiPlayer.totalTicks();

    const int totalTicks =
        playerTotalTicks > 0
            ? playerTotalTicks
            : viewState.totalTicks;

    const int currentTick =
        std::clamp(
            midiPlayer.currentTick(),
            0,
            std::max(
                totalTicks,
                0
            )
        );

    if (totalTicks > 0) {
        const float fraction =
            static_cast<float>(
                currentTick
            ) /
            static_cast<float>(
                totalTicks
            );

        const float playheadX =
            origin.x +
            size.x * fraction;

        drawList->AddLine(
            ImVec2(
                playheadX,
                origin.y
            ),
            ImVec2(
                playheadX,
                end.y
            ),
            playhead,
            2.0f
        );

        if (
            ImGui::IsItemHovered() &&
            ImGui::IsMouseClicked(
                ImGuiMouseButton_Left
            ) &&
            midiPlayer.isAvailable() &&
            midiPlayer.hasMidi()
        ) {
            const float mouseFraction =
                std::clamp(
                    (
                        ImGui::GetIO().MousePos.x -
                        origin.x
                    ) /
                    std::max(
                        size.x,
                        1.0f
                    ),
                    0.0f,
                    1.0f
                );

            const int seekTick =
                static_cast<int>(
                    std::lround(
                        mouseFraction *
                        static_cast<float>(
                            totalTicks
                        )
                    )
                );

            if (midiPlayer.seek(seekTick)) {
                viewState.seekTick =
                    seekTick;

                viewState.playbackStatus =
                    midiPlayer.statusMessage();
            }
        }
    }

    drawList->AddText(
        ImVec2(
            origin.x + 10.0f,
            origin.y + 8.0f
        ),
        ImGui::GetColorU32(
            ImGuiCol_Text
        ),
        "MIDI NOTE ACTIVITY"
    );

    drawList->AddText(
        ImVec2(
            origin.x + 10.0f,
            origin.y + 26.0f
        ),
        ImGui::GetColorU32(
            ImGuiCol_TextDisabled
        ),
        "note-on density / velocity  |  click to seek"
    );
}

}
