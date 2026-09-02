#pragma once

#include <cstddef>
#include <cstdint>

#include "animation/AnimationFrameIndex.h"
#include "sequence/SequenceDefinition.h"

namespace eld::render {

class AnimationPlayer {
public:
    static constexpr std::uint32_t ClientCycleMilliseconds = 20;

    explicit AnimationPlayer(
        const eld::animation::AnimationFrameIndex& frames
    );

    void setSequence(
        const eld::definition::SequenceDefinition& sequence
    );

    void clear();

    const eld::definition::SequenceDefinition* sequence() const;
    const eld::definition::SequenceFrame* currentSequenceFrame() const;

    eld::animation::ResolvedAnimationFrame currentResolvedFrame() const;

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

    const eld::animation::AnimationFrameIndex* frames_ = nullptr;
    const eld::definition::SequenceDefinition* sequence_ = nullptr;

    std::size_t frameIndex_ = 0;
    double elapsedMilliseconds_ = 0.0;

    bool playing_ = true;
    bool looping_ = true;
    float speed_ = 1.0f;
};

}
