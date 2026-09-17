#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "sequence/SequenceResource.h"

namespace eld::render {

class AnimationPlayer {
public:
    static constexpr std::uint32_t
        ClientCycleMilliseconds = 20;

    AnimationPlayer() = default;

    void setSequence(
        const eld::sequence::SequenceResource& sequence
    );

    void clear();

    const eld::sequence::SequenceResource*
    sequence() const;

    const eld::sequence::SequenceFrameData*
    currentSequenceFrame() const;

    std::optional<eld::animation::AnimationFrameResource>
    currentFrame() const;

    std::size_t frameIndex() const;
    std::size_t frameCount() const;

    std::uint32_t
    currentFrameDurationMilliseconds() const;

    bool update(
        std::uint64_t deltaMilliseconds
    );

    bool stepForward();
    bool stepBackward();

    void play();
    void pause();
    void setPlaying(bool playing);
    void restart();

    bool isPlaying() const;

    void setLooping(bool looping);
    bool isLooping() const;
    bool looping() const { return looping_; }

    void setSpeed(float speed);
    float speed() const;

private:
    std::size_t loopStart() const;
    bool advanceFrame();

    std::optional<
        eld::sequence::SequenceResource
    > sequence_;

    std::size_t frameIndex_ = 0;
    double elapsedMilliseconds_ = 0.0;

    bool playing_ = true;
    bool looping_ = true;
    float speed_ = 1.0f;
};

}
