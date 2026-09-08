#include "AnimationPlayer.h"

#include <algorithm>

namespace eld::render {

void AnimationPlayer::setSequence(
    const eld::sequence::SequenceResource& sequence
) {
    sequence_.emplace(sequence);
    frameIndex_ = 0;
    elapsedMilliseconds_ = 0.0;
}

void AnimationPlayer::clear() {
    sequence_.reset();
    frameIndex_ = 0;
    elapsedMilliseconds_ = 0.0;
}

const eld::sequence::SequenceResource*
AnimationPlayer::sequence() const {
    return
        sequence_.has_value()
            ? &*sequence_
            : nullptr;
}

const eld::sequence::SequenceFrameData*
AnimationPlayer::currentSequenceFrame() const {
    if (
        !sequence_.has_value() ||
        sequence_->data.frames.empty() ||
        frameIndex_ >= sequence_->data.frames.size()
    ) {
        return nullptr;
    }

    return &sequence_->data.frames[frameIndex_];
}

std::optional<eld::animation::AnimationFrameResource>
AnimationPlayer::currentFrame() const {
    if (
        !sequence_.has_value() ||
        frameIndex_ >= sequence_->resolvedFrames.size()
    ) {
        return std::nullopt;
    }

    return sequence_->resolvedFrames[
        frameIndex_
    ].primary.resource();
}

std::size_t AnimationPlayer::frameIndex() const {
    return frameIndex_;
}

std::size_t AnimationPlayer::frameCount() const {
    return
        sequence_.has_value()
            ? sequence_->data.frames.size()
            : 0;
}

std::uint32_t
AnimationPlayer::currentFrameDurationMilliseconds() const {
    const auto* sequenceFrame =
        currentSequenceFrame();

    if (sequenceFrame == nullptr) {
        return ClientCycleMilliseconds;
    }

    std::uint32_t units =
        sequenceFrame->duration;

    if (units == 0) {
        const auto frame =
            currentFrame();

        if (frame.has_value()) {
            units =
                frame->frame.delay;
        }
    }

    if (units == 0) {
        units = 1;
    }

    return
        units *
        ClientCycleMilliseconds;
}

bool AnimationPlayer::update(
    std::uint64_t deltaMilliseconds
) {
    if (
        !playing_ ||
        !sequence_.has_value() ||
        sequence_->data.frames.empty()
    ) {
        return false;
    }

    elapsedMilliseconds_ +=
        static_cast<double>(deltaMilliseconds) *
        static_cast<double>(speed_);

    bool frameChanged = false;

    while (playing_) {
        const double duration =
            static_cast<double>(
                currentFrameDurationMilliseconds()
            );

        if (elapsedMilliseconds_ < duration) {
            break;
        }

        elapsedMilliseconds_ -= duration;

        if (!advanceFrame()) {
            elapsedMilliseconds_ = 0.0;
            break;
        }

        frameChanged = true;
    }

    return frameChanged;
}

bool AnimationPlayer::stepForward() {
    if (
        !sequence_.has_value() ||
        sequence_->data.frames.empty()
    ) {
        return false;
    }

    elapsedMilliseconds_ = 0.0;

    return advanceFrame();
}

bool AnimationPlayer::stepBackward() {
    if (
        !sequence_.has_value() ||
        sequence_->data.frames.empty()
    ) {
        return false;
    }

    if (frameIndex_ > 0) {
        --frameIndex_;
    }
    else {
        frameIndex_ =
            sequence_->data.frames.size() - 1;
    }

    elapsedMilliseconds_ = 0.0;

    return true;
}

void AnimationPlayer::play() {
    playing_ = true;
}

void AnimationPlayer::pause() {
    playing_ = false;
}

void AnimationPlayer::setPlaying(
    bool playing
) {
    playing_ = playing;
}

bool AnimationPlayer::isPlaying() const {
    return playing_;
}

void AnimationPlayer::setLooping(
    bool looping
) {
    looping_ = looping;
}

bool AnimationPlayer::isLooping() const {
    return looping_;
}

void AnimationPlayer::setSpeed(
    float speed
) {
    speed_ =
        std::max(
            speed,
            0.01f
        );
}

float AnimationPlayer::speed() const {
    return speed_;
}

void AnimationPlayer::restart() {
    frameIndex_ = 0;
    elapsedMilliseconds_ = 0.0;
}

std::size_t AnimationPlayer::loopStart() const {
    if (
        sequence_.has_value() &&
        sequence_->data.frameStep.has_value() &&
        *sequence_->data.frameStep > 0 &&
        *sequence_->data.frameStep <= sequence_->data.frames.size()
    ) {
        return
            sequence_->data.frames.size() -
            *sequence_->data.frameStep;
    }

    return 0;
}

bool AnimationPlayer::advanceFrame() {
    if (
        !sequence_.has_value() ||
        sequence_->data.frames.empty()
    ) {
        return false;
    }

    ++frameIndex_;

    if (frameIndex_ < sequence_->data.frames.size()) {
        return true;
    }

    if (!looping_) {
        frameIndex_ =
            sequence_->data.frames.size() - 1;

        playing_ = false;
        return false;
    }

    frameIndex_ = loopStart();
    return true;
}

}
