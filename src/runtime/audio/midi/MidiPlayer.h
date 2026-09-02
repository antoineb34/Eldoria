#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace eld::audio {

enum class MidiPlayerState : std::uint8_t {
    Unavailable,
    Empty,
    Stopped,
    Playing,
    Paused,
    Finished,
    Error
};

class MidiPlayer {
public:
    MidiPlayer();
    ~MidiPlayer();

    MidiPlayer(const MidiPlayer&) = delete;
    MidiPlayer& operator=(const MidiPlayer&) = delete;

    MidiPlayer(MidiPlayer&&) noexcept;
    MidiPlayer& operator=(MidiPlayer&&) noexcept;

    bool initialize(
        const std::filesystem::path& soundFontPath = {}
    );

    void shutdown();
    void unload();

    bool load(
        const std::vector<std::uint8_t>& midiBytes
    );

    bool play();
    void pause();
    void stop();

    bool seek(
        int tick
    );

    void setVolume(
        float volume
    );

    float volume() const;

    bool isAvailable() const;
    bool hasMidi() const;

    MidiPlayerState state() const;

    int currentTick() const;
    int totalTicks() const;
    int currentBpm() const;

    const std::filesystem::path& soundFontPath() const;
    const std::string& statusMessage() const;

    static bool compiledWithFluidSynth();

    static std::optional<std::filesystem::path>
    findDefaultSoundFont();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

const char* midiPlayerStateName(
    MidiPlayerState state
);

}
