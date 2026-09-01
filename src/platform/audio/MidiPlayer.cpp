#include "MidiPlayer.h"

#include <algorithm>
#include <cstdlib>
#include <system_error>
#include <utility>

#if ELDORIA_HAS_FLUIDSYNTH
#include <fluidsynth.h>
#endif

namespace eld::audio {

namespace {

bool isRegularFile(
    const std::filesystem::path& path
) {
    std::error_code error;

    return !path.empty() &&
        std::filesystem::is_regular_file(
            path,
            error
        );
}

bool startsWithMidiHeader(
    const std::vector<std::uint8_t>& bytes
) {
    return bytes.size() >= 4 &&
        bytes[0] == 'M' &&
        bytes[1] == 'T' &&
        bytes[2] == 'h' &&
        bytes[3] == 'd';
}

}

class MidiPlayer::Impl {
public:
    bool initialize(
        const std::filesystem::path& requestedSoundFont
    ) {
        shutdown();

#if ELDORIA_HAS_FLUIDSYNTH
        std::filesystem::path selectedSoundFont =
            requestedSoundFont;

        if (selectedSoundFont.empty()) {
            const auto detected =
                MidiPlayer::findDefaultSoundFont();

            if (!detected.has_value()) {
                state_ = MidiPlayerState::Unavailable;
                statusMessage_ =
                    "FluidSynth is available, but no SoundFont was found. "
                    "Set ELDORIA_SOUNDFONT or install a default .sf2 SoundFont.";
                return false;
            }

            selectedSoundFont = *detected;
        }

        if (!isRegularFile(selectedSoundFont)) {
            state_ = MidiPlayerState::Unavailable;
            statusMessage_ =
                "SoundFont does not exist: " +
                selectedSoundFont.string();
            return false;
        }

        settings_ =
            new_fluid_settings();

        if (settings_ == nullptr) {
            return fail(
                "Failed to create FluidSynth settings"
            );
        }

        fluid_settings_setnum(
            settings_,
            "synth.gain",
            static_cast<double>(volume_)
        );

        synth_ =
            new_fluid_synth(
                settings_
            );

        if (synth_ == nullptr) {
            return fail(
                "Failed to create FluidSynth synthesizer"
            );
        }

        const int soundFontId =
            fluid_synth_sfload(
                synth_,
                selectedSoundFont.string().c_str(),
                1
            );

        if (soundFontId == FLUID_FAILED) {
            return fail(
                "FluidSynth failed to load SoundFont: " +
                selectedSoundFont.string()
            );
        }

        soundFontPath_ =
            std::move(selectedSoundFont);

        state_ = MidiPlayerState::Empty;
        statusMessage_ =
            "FluidSynth ready";

        return true;
#else
        (void)requestedSoundFont;

        state_ = MidiPlayerState::Unavailable;
        statusMessage_ =
            "This build does not include FluidSynth support";

        return false;
#endif
    }

    void shutdown() {
#if ELDORIA_HAS_FLUIDSYNTH
        destroyPlayer();

        if (audioDriver_ != nullptr) {
            delete_fluid_audio_driver(
                audioDriver_
            );
            audioDriver_ = nullptr;
        }

        if (synth_ != nullptr) {
            delete_fluid_synth(
                synth_
            );
            synth_ = nullptr;
        }

        if (settings_ != nullptr) {
            delete_fluid_settings(
                settings_
            );
            settings_ = nullptr;
        }
#endif

        midiBytes_.clear();
        soundFontPath_.clear();

        state_ = compiledWithFluidSynthImpl()
            ? MidiPlayerState::Empty
            : MidiPlayerState::Unavailable;

        statusMessage_.clear();
    }

    void unload() {
#if ELDORIA_HAS_FLUIDSYNTH
        destroyPlayer();
#endif

        midiBytes_.clear();
        paused_ = false;

        state_ = isAvailable()
            ? MidiPlayerState::Empty
            : MidiPlayerState::Unavailable;

        if (isAvailable()) {
            statusMessage_ =
                "No MIDI loaded";
        }
    }

    bool load(
        const std::vector<std::uint8_t>& midiBytes
    ) {
#if ELDORIA_HAS_FLUIDSYNTH
        if (synth_ == nullptr) {
            state_ = MidiPlayerState::Unavailable;
            statusMessage_ =
                "MIDI playback backend is not initialized";
            return false;
        }

        if (!startsWithMidiHeader(midiBytes)) {
            state_ = MidiPlayerState::Error;
            statusMessage_ =
                "MIDI playback requires a normalized MThd stream";
            return false;
        }

        midiBytes_ = midiBytes;

        if (!rebuildPlayer()) {
            return false;
        }

        state_ = MidiPlayerState::Stopped;
        statusMessage_ =
            "MIDI loaded";

        return true;
#else
        (void)midiBytes;
        state_ = MidiPlayerState::Unavailable;
        statusMessage_ =
            "This build does not include FluidSynth support";
        return false;
#endif
    }

    bool play() {
#if ELDORIA_HAS_FLUIDSYNTH
        if (player_ == nullptr) {
            statusMessage_ =
                "No MIDI is loaded";
            return false;
        }

        if (
            fluid_player_get_status(player_) ==
            FLUID_PLAYER_DONE
        ) {
            if (!rebuildPlayer()) {
                return false;
            }
        }

        if (
            fluid_player_play(player_) !=
            FLUID_OK
        ) {
            state_ = MidiPlayerState::Error;
            statusMessage_ =
                "FluidSynth failed to start MIDI playback";
            return false;
        }

        paused_ = false;
        state_ = MidiPlayerState::Playing;
        statusMessage_ =
            "Playing";

        return true;
#else
        return false;
#endif
    }

    void pause() {
#if ELDORIA_HAS_FLUIDSYNTH
        if (player_ == nullptr) {
            return;
        }

        if (
            fluid_player_get_status(player_) !=
            FLUID_PLAYER_PLAYING
        ) {
            return;
        }

        fluid_player_stop(player_);
        paused_ = true;
        state_ = MidiPlayerState::Paused;
        statusMessage_ =
            "Paused";
#endif
    }

    void stop() {
#if ELDORIA_HAS_FLUIDSYNTH
        if (player_ == nullptr) {
            return;
        }

        if (rebuildPlayer()) {
            state_ = MidiPlayerState::Stopped;
            statusMessage_ =
                "Stopped";
        }
#endif
    }

    bool seek(
        int tick
    ) {
#if ELDORIA_HAS_FLUIDSYNTH
        if (player_ == nullptr) {
            return false;
        }

        const int total = totalTicks();

        if (
            tick < 0 ||
            total < 0 ||
            tick > total
        ) {
            statusMessage_ =
                "MIDI seek position is out of range";
            return false;
        }

        if (
            fluid_player_seek(
                player_,
                tick
            ) != FLUID_OK
        ) {
            statusMessage_ =
                "FluidSynth rejected the MIDI seek request";
            return false;
        }

        statusMessage_ =
            "Seeked to tick " +
            std::to_string(tick);

        return true;
#else
        (void)tick;
        return false;
#endif
    }

    void setVolume(
        float volume
    ) {
        volume_ =
            std::clamp(
                volume,
                0.0f,
                1.0f
            );

#if ELDORIA_HAS_FLUIDSYNTH
        if (synth_ != nullptr) {
            fluid_synth_set_gain(
                synth_,
                volume_
            );
        }
#endif
    }

    float volume() const {
        return volume_;
    }

    bool isAvailable() const {
#if ELDORIA_HAS_FLUIDSYNTH
        return synth_ != nullptr;
#else
        return false;
#endif
    }

    bool hasMidi() const {
#if ELDORIA_HAS_FLUIDSYNTH
        return player_ != nullptr &&
            !midiBytes_.empty();
#else
        return false;
#endif
    }

    MidiPlayerState state() const {
#if ELDORIA_HAS_FLUIDSYNTH
        if (player_ != nullptr) {
            const int playerStatus =
                fluid_player_get_status(
                    player_
                );

            if (
                playerStatus ==
                FLUID_PLAYER_PLAYING
            ) {
                return MidiPlayerState::Playing;
            }

            if (paused_) {
                return MidiPlayerState::Paused;
            }

            if (
                playerStatus ==
                FLUID_PLAYER_DONE
            ) {
                return MidiPlayerState::Finished;
            }
        }
#endif

        return state_;
    }

    int currentTick() const {
#if ELDORIA_HAS_FLUIDSYNTH
        if (player_ != nullptr) {
            return fluid_player_get_current_tick(
                player_
            );
        }
#endif

        return 0;
    }

    int totalTicks() const {
#if ELDORIA_HAS_FLUIDSYNTH
        if (player_ != nullptr) {
            return fluid_player_get_total_ticks(
                player_
            );
        }
#endif

        return 0;
    }

    int currentBpm() const {
#if ELDORIA_HAS_FLUIDSYNTH
        if (player_ != nullptr) {
            return fluid_player_get_bpm(
                player_
            );
        }
#endif

        return 0;
    }

    const std::filesystem::path&
    soundFontPath() const {
        return soundFontPath_;
    }

    const std::string&
    statusMessage() const {
        return statusMessage_;
    }

private:
    static bool compiledWithFluidSynthImpl() {
#if ELDORIA_HAS_FLUIDSYNTH
        return true;
#else
        return false;
#endif
    }

#if ELDORIA_HAS_FLUIDSYNTH
    void destroyPlayer() {
        if (audioDriver_ != nullptr) {
            delete_fluid_audio_driver(
                audioDriver_
            );
            audioDriver_ = nullptr;
        }

        if (player_ != nullptr) {
            fluid_player_stop(player_);
            fluid_player_join(player_);

            delete_fluid_player(player_);
            player_ = nullptr;
        }

        paused_ = false;

        if (synth_ != nullptr) {
            fluid_synth_system_reset(
                synth_
            );
        }
    }

    bool rebuildPlayer() {
        destroyPlayer();

        if (
            synth_ == nullptr ||
            midiBytes_.empty()
        ) {
            state_ = MidiPlayerState::Error;
            statusMessage_ =
                "Cannot create MIDI player without a synth and MIDI data";
            return false;
        }

        player_ =
            new_fluid_player(
                synth_
            );

        if (player_ == nullptr) {
            state_ = MidiPlayerState::Error;
            statusMessage_ =
                "Failed to create FluidSynth MIDI player";
            return false;
        }

        if (
            fluid_player_add_mem(
                player_,
                midiBytes_.data(),
                midiBytes_.size()
            ) != FLUID_OK
        ) {
            delete_fluid_player(player_);
            player_ = nullptr;

            state_ = MidiPlayerState::Error;
            statusMessage_ =
                "FluidSynth rejected normalized MIDI bytes";
            return false;
        }

        audioDriver_ =
            new_fluid_audio_driver(
                settings_,
                synth_
            );

#if defined(__linux__)
        // Some FluidSynth builds default to PipeWire, whose embedded-driver
        // API expects the host to initialize PipeWire itself.  ElForge does
        // not own PipeWire, so fall back to the PulseAudio compatibility
        // service (normally provided by PipeWire on modern Linux) and then
        // ALSA before giving up.
        if (audioDriver_ == nullptr) {
            fluid_settings_setstr(
                settings_,
                "audio.driver",
                "pulseaudio"
            );

            audioDriver_ =
                new_fluid_audio_driver(
                    settings_,
                    synth_
                );
        }

        if (audioDriver_ == nullptr) {
            fluid_settings_setstr(
                settings_,
                "audio.driver",
                "alsa"
            );

            audioDriver_ =
                new_fluid_audio_driver(
                    settings_,
                    synth_
                );
        }
#endif

        if (audioDriver_ == nullptr) {
            delete_fluid_player(player_);
            player_ = nullptr;

            state_ = MidiPlayerState::Error;
            statusMessage_ =
                "FluidSynth failed to open an audio output device";
            return false;
        }

        paused_ = false;
        return true;
    }

    bool fail(
        std::string message
    ) {
        statusMessage_ =
            std::move(message);
        state_ = MidiPlayerState::Error;

        if (audioDriver_ != nullptr) {
            delete_fluid_audio_driver(
                audioDriver_
            );
            audioDriver_ = nullptr;
        }

        if (synth_ != nullptr) {
            delete_fluid_synth(
                synth_
            );
            synth_ = nullptr;
        }

        if (settings_ != nullptr) {
            delete_fluid_settings(
                settings_
            );
            settings_ = nullptr;
        }

        return false;
    }

    fluid_settings_t* settings_ = nullptr;
    fluid_synth_t* synth_ = nullptr;
    fluid_audio_driver_t* audioDriver_ = nullptr;
    fluid_player_t* player_ = nullptr;
#endif

    std::vector<std::uint8_t> midiBytes_;
    std::filesystem::path soundFontPath_;

    float volume_ = 0.5f;
    bool paused_ = false;

    MidiPlayerState state_ =
        compiledWithFluidSynthImpl()
            ? MidiPlayerState::Empty
            : MidiPlayerState::Unavailable;

    std::string statusMessage_;
};

MidiPlayer::MidiPlayer()
    : impl_(std::make_unique<Impl>()) {
}

MidiPlayer::~MidiPlayer() = default;

MidiPlayer::MidiPlayer(
    MidiPlayer&&
) noexcept = default;

MidiPlayer& MidiPlayer::operator=(
    MidiPlayer&&
) noexcept = default;

bool MidiPlayer::initialize(
    const std::filesystem::path& soundFontPath
) {
    return impl_->initialize(
        soundFontPath
    );
}

void MidiPlayer::shutdown() {
    impl_->shutdown();
}

void MidiPlayer::unload() {
    impl_->unload();
}

bool MidiPlayer::load(
    const std::vector<std::uint8_t>& midiBytes
) {
    return impl_->load(
        midiBytes
    );
}

bool MidiPlayer::play() {
    return impl_->play();
}

void MidiPlayer::pause() {
    impl_->pause();
}

void MidiPlayer::stop() {
    impl_->stop();
}

bool MidiPlayer::seek(
    int tick
) {
    return impl_->seek(tick);
}

void MidiPlayer::setVolume(
    float volume
) {
    impl_->setVolume(volume);
}

float MidiPlayer::volume() const {
    return impl_->volume();
}

bool MidiPlayer::isAvailable() const {
    return impl_->isAvailable();
}

bool MidiPlayer::hasMidi() const {
    return impl_->hasMidi();
}

MidiPlayerState MidiPlayer::state() const {
    return impl_->state();
}

int MidiPlayer::currentTick() const {
    return impl_->currentTick();
}

int MidiPlayer::totalTicks() const {
    return impl_->totalTicks();
}

int MidiPlayer::currentBpm() const {
    return impl_->currentBpm();
}

const std::filesystem::path&
MidiPlayer::soundFontPath() const {
    return impl_->soundFontPath();
}

const std::string&
MidiPlayer::statusMessage() const {
    return impl_->statusMessage();
}

bool MidiPlayer::compiledWithFluidSynth() {
#if ELDORIA_HAS_FLUIDSYNTH
    return true;
#else
    return false;
#endif
}

std::optional<std::filesystem::path>
MidiPlayer::findDefaultSoundFont() {
    if (const char* configured =
            std::getenv("ELDORIA_SOUNDFONT")) {
        const std::filesystem::path path(
            configured
        );

        if (isRegularFile(path)) {
            return path;
        }
    }

    const std::filesystem::path candidates[] = {
        "content/audio/default.sf2",
        "/usr/share/soundfonts/default.sf2",
        "/usr/share/sounds/sf2/default.sf2",
        "/usr/share/soundfonts/FluidR3_GM.sf2"
    };

    for (
        const std::filesystem::path& path :
        candidates
    ) {
        if (isRegularFile(path)) {
            return path;
        }
    }

    return std::nullopt;
}

const char* midiPlayerStateName(
    MidiPlayerState state
) {
    switch (state) {
        case MidiPlayerState::Unavailable:
            return "Unavailable";
        case MidiPlayerState::Empty:
            return "Empty";
        case MidiPlayerState::Stopped:
            return "Stopped";
        case MidiPlayerState::Playing:
            return "Playing";
        case MidiPlayerState::Paused:
            return "Paused";
        case MidiPlayerState::Finished:
            return "Finished";
        case MidiPlayerState::Error:
            return "Error";
    }

    return "Unknown";
}

}
