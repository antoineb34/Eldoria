# Model Animation

## Runtime boundary

```text
Index 2 animation archives
    -> data/animation/AnimationRepository
    -> data/animation/AnimationFrameIndex
    -> global frame id -> AnimationFrame + Skeleton

ModelMesh + AnimationFrame + Skeleton
    -> graphics/animation/ModelAnimator
    -> deformed ModelMesh
    -> GraphicsResources
    -> Render
```

`render/` contains no RS317 animation semantics.

## Applied transform semantics

- `0`: pivot/origin.
- `1`: translation.
- `2`: classic fixed-point rotation around the active pivot, in `Z -> X -> Y` order. Components use `(value & 0xff) * 8` in the 2048-entry trig domain.
- `3`: scale around the active pivot with a `128` baseline.
- `4`: unknown and intentionally ignored.
- `5`: face alpha. The X component changes each selected face alpha by `x * 8`, clamped to `0..255`.

The application step also reconstructs the classic implicit-pivot rule: before a non-pivot explicit transform, the nearest skipped type-0 slot since the previous explicit transform is executed with `(0,0,0)`.

## Skin groups

`ModelSkinGroups` derives vertex and face groups from `Vertex::skin` and `Face::skin`. Skeleton group ids address those skin groups; they are not raw vertex/face indices.

## Probe status

The working NPC animation probe uses the production `AnimationFrameIndex`, `AnimationPlayer` and `ModelAnimator`. The older `visual_probe.cpp` remains only as a scratchpad/regression reference and is no longer included by the NPC path.

## Playback

`eld::graphics::AnimationPlayer` owns reusable sequence playback state.

It tracks the selected sequence, current frame, elapsed frame time,
play/pause state, playback speed and looping. It does not own a model or any
render state.

Frame duration uses the same rule validated by the NPC animation probe:

```text
SequenceFrame.duration when non-zero
otherwise AnimationFrame.delay
otherwise 1 client cycle
```

One classic client cycle is currently represented as 20 milliseconds.

Looping uses `SequenceDefinition::frameStep` when it is present and valid;
otherwise playback wraps to frame zero. Full gameplay-specific sequence
precedence, blending and maximum-loop behavior remain outside this initial
player.
