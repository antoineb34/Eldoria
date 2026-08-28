# Animation Loading

## Status

Initial production data-loading implementation.

The binary layout was validated against all 582 files in cache Index 2.
The probe decoded 16,926 unique frame IDs covering exactly 0 through 16,925
with no duplicates.

## Cache Location

Animation archives are stored in cache Index 2 (`IndexId::Animations`).

All 582 tested files were gzip-compressed at the cache layer.

## File Layout

After cache decompression:

```text
u16 frameCount

frame-header section
    repeated frameCount times:
        u16 globalFrameId
        u8  slotCount

flag section
    one u8 flag per referenced skeleton slot per frame

transform-value section
    signed-smart values selected by flag bits

delay section
    one u8 delay per frame

skeleton section
    u8 slotCount
    u8 transformType[slotCount]

    repeated slotCount times:
        u8 groupCount
        u8 groups[groupCount]

8-byte footer
    u16 frameHeaderBytes
    u16 flagBytes
    u16 transformValueBytes
    u16 delayBytes
```

The leading `frameCount` is not included in `frameHeaderBytes`.

## Transform Flags

```text
bit 0 = X encoded
bit 1 = Y encoded
bit 2 = Z encoded
```

Values use the RS signed-smart convention.

Missing components default to 128 for scale transforms and 0 otherwise.

## Transform Types

```text
0 = pivot/origin
1 = translation
2 = rotation
3 = scale
4 = unknown
5 = alpha
```

Type 4 remains intentionally uninterpreted.

## Production Boundary

```text
AnimationRepository
    -> cache Index 2
    -> AnimationFileParser
    -> AnimationFile (raw bytes + proven layout)
    -> AnimationDecoder
    -> AnimationAsset (frames + skeleton)
```

Model deformation, playback, interpolation and ElForge animation UI are not
part of this initial data-loading step.

## Open Questions

- Exact behavior of transform type 4.
- Exact semantic role of `anim_index`.
- Whether implicit pivot transforms should be materialized during decode or
  resolved later while applying a frame to a model.
