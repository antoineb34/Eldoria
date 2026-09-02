#include "views/midi/MidiViewPanel.h"

#include <algorithm>
#include <cmath>
#include <string>

#include <imgui.h>

#include "midi/MidiPlayer.h"

#include "explorer/CacheExplorerState.h"
#include "ui/WorkspaceUi.h"

#include "midi/MidiFile.h"
#include "views/midi/MidiViewState.h"

namespace eld::elforge {


void MidiViewPanel::renderWorkspace(
    CacheExplorerState& state,
    const eld::midi::MidiFile* midi,
    MidiViewState& viewState,
    eld::audio::MidiPlayer& midiPlayer,
    const ImVec2& controlsPosition,
    const ImVec2& controlsSize
) {
    if (midi == nullptr) {
        return;
    }


    const viewport_workspace::BottomRow layout =
        viewport_workspace::bottomRow(
            controlsSize.x
        );

    ui::workspace::beginDockedHud(
        "##MidiBottomHud",
        controlsPosition,
        controlsSize
    );

    const float cardY =
        ui::workspace::dockedCardY(
            controlsPosition,
            controlsSize
        );


    // --------------------------------------------------------
    // LEFT — playback
    // --------------------------------------------------------

    ui::workspace::beginCard(
        "##MidiPlaybackCard",
        ImVec2(
            controlsPosition.x +
                layout.leftX,
            cardY
        ),
        ImVec2(
            viewport_workspace::
                LeftWidth,
            viewport_workspace::
                CardHeight
        )
    );

    ui::workspace::centeredText(
        "PLAYBACK",
        7.0f,
        true
    );

    const eld::audio::MidiPlayerState playbackState =
        midiPlayer.state();

    constexpr float Gap =
        6.0f;

    constexpr float ButtonWidth =
        68.0f;

    constexpr float RowWidth =
        ButtonWidth *
            3.0f +
        Gap *
            2.0f;

    ImGui::SetCursorPos(
        ImVec2(
            (
                viewport_workspace::
                    LeftWidth -
                RowWidth
            ) *
                0.5f,
            28.0f
        )
    );

    if (
        ui::workspace::pillButton(
            "MidiPlay",
            "PLAY",
            playbackState ==
                eld::audio::
                    MidiPlayerState::Playing,
            ImVec2(
                ButtonWidth,
                24.0f
            )
        )
    ) {
        if (!midiPlayer.play()) {
            viewState.playbackStatus =
                midiPlayer.statusMessage();
        }
        else {
            viewState.playbackStatus =
                "Playing";
        }
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "MidiPause",
            "PAUSE",
            playbackState ==
                eld::audio::
                    MidiPlayerState::Paused,
            ImVec2(
                ButtonWidth,
                24.0f
            )
        )
    ) {
        midiPlayer.pause();

        viewState.playbackStatus =
            midiPlayer.statusMessage();
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "MidiStop",
            "STOP",
            playbackState ==
                eld::audio::
                    MidiPlayerState::Stopped,
            ImVec2(
                ButtonWidth,
                24.0f
            )
        )
    ) {
        midiPlayer.stop();

        viewState.seekTick = 0;
        viewState.seekActive = false;

        viewState.playbackStatus =
            midiPlayer.statusMessage();
    }


    float volume =
        midiPlayer.volume();

    ImGui::SetCursorPos(
        ImVec2(
            26.0f,
            64.0f
        )
    );

    ImGui::TextDisabled(
        "VOL"
    );

    ImGui::SameLine(
        0.0f,
        8.0f
    );

    ImGui::SetNextItemWidth(
        178.0f
    );

    if (
        ImGui::SliderFloat(
            "##MidiWorkspaceVolume",
            &volume,
            0.0f,
            1.0f,
            "%.2f"
        )
    ) {
        midiPlayer.setVolume(
            volume
        );
    }

    ui::workspace::endCard();


    // --------------------------------------------------------
    // CENTER — transport position
    // --------------------------------------------------------

    if (
        layout.centerWidth >=
        150.0f
    ) {
        ui::workspace::beginCard(
            "##MidiPositionCard",
            ImVec2(
                controlsPosition.x +
                    layout.centerX,
                cardY
            ),
            ImVec2(
                layout.centerWidth,
                viewport_workspace::
                    CardHeight
            )
        );

        ui::workspace::centeredText(
            "POSITION",
            7.0f,
            true
        );

        const int currentTick =
            midiPlayer.currentTick();

        const int totalTicks =
            std::max(
                midiPlayer.totalTicks(),
                viewState.totalTicks
            );

        ui::workspace::centeredText(
            std::to_string(
                currentTick
            ) +
                " / " +
                std::to_string(
                    totalTicks
                ),
            30.0f
        );

        const int bpm =
            midiPlayer.currentBpm();

        ui::workspace::centeredText(
            bpm > 0
                ? std::to_string(
                      bpm
                  ) +
                      " BPM"
                : "tempo --",
            52.0f,
            true
        );

        ui::workspace::centeredText(
            "click waveform to seek",
            70.0f,
            true
        );

        ui::workspace::endCard();
    }


    // --------------------------------------------------------
    // RIGHT — MIDI identity / status
    // --------------------------------------------------------

    ui::workspace::beginCard(
        "##MidiStatusCard",
        ImVec2(
            controlsPosition.x +
                layout.rightX,
            cardY
        ),
        ImVec2(
            layout.rightWidth,
            viewport_workspace::
                CardHeight
        )
    );

    ui::workspace::centeredText(
        "MIDI",
        7.0f,
        true
    );

    ui::workspace::centeredText(
        "#" +
            std::to_string(
                midi->id
            ),
        29.0f
    );

    ui::workspace::centeredText(
        eld::audio::midiPlayerStateName(
            midiPlayer.state()
        ),
        50.0f,
        true
    );

    std::string status =
        viewState.playbackStatus;

    if (status.empty()) {
        status =
            midiPlayer.statusMessage();
    }

    if (status.empty()) {
        status =
            midiPlayer.isAvailable()
                ? "ready"
                : "playback unavailable";
    }

    if (status.size() > 42) {
        status =
            status.substr(
                0,
                39
            ) +
            "...";
    }

    ui::workspace::centeredText(
        status,
        69.0f,
        true
    );

    ui::workspace::endCard();

    ui::workspace::endDockedHud();
}

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
