#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "AnimationFrameTable.h"
#include "Sequence.h"

namespace eld::render {

class AnimationPlayer {
public:
    static constexpr std::uint32_t ClientCycleMilliseconds = 20;

    explicit AnimationPlayer(
        const eld::animation::AnimationFrameTable& frames
    );

    void setSequence(
        const eld::sequence::Sequence& sequence
    );

    void clear();

    const eld::sequence::Sequence* sequence() const;
    const eld::sequence::SequenceFrame* currentSequenceFrame() const;

    std::optional<eld::animation::AnimationFrameView>
    currentFrame() const;

    std::size_t frameIndex() const;
    std::size_t frameCount() const;

    std::uint32_t currentFrameDurationMilliseconds() const;

    bool update(
        std::uint64_t deltaMilliseconds
    );

    bool stepForward();
    bool stepBackward();

    void play();
    void pause();
    void setPlaying(bool playing);
    bool isPlaying() const;

    void setLooping(bool looping);

    [[nodiscard]] bool looping() const {
        return looping_;
    }

    bool isLooping() const;

    void setSpeed(float speed);
    float speed() const;

    void restart();

private:
    std::size_t loopStart() const;
    bool advanceFrame();

    const eld::animation::AnimationFrameTable* frames_ = nullptr;
    std::optional<eld::sequence::Sequence> sequence_;

    std::size_t frameIndex_ = 0;
    double elapsedMilliseconds_ = 0.0;

    bool playing_ = true;
    bool looping_ = true;
    float speed_ = 1.0f;
};

}
