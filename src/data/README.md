# data

Purpose: owns static content and the process of loading it.

## Responsibilities

`data` owns source-format access and decoding:

- DAT/IDX cache access and safe writes
- JAG archive parsing
- binary readers/writers and compression
- models and animation frames
- definitions
- maps, terrain, and location spawns
- interfaces
- indexed images, sprites, textures, JPEGs, and fonts
- MIDI source data

The normal asset flow is:

```text
cache bytes
    -> file/parser structures
    -> decoded static asset
    -> repository access
```

Not every format requires a separate parser and decoder class. Those layers are
split only when they represent genuinely different responsibilities.

## Boundaries

`data` describes what static source content is and how it is loaded.

It does not own:

- live world state
- gameplay rules
- rendering behavior
- audio playback
- networking behavior
- application UI
- server authority

Dependency rule: `data` stays independent from runnable apps and must not
depend on `world`, `game`, `network`, `graphics`, `render`, UI, or
platform-specific application code.

## Remaining source-format work

The major visual/gameplay cache formats are decoded. Remaining legacy cache
areas are intentionally tracked separately:

- word/chat-filter data
- classic sound-effect data
- version/checksum manifest metadata

These are source-data concerns and belong here when implemented.
