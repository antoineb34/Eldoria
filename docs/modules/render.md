# Render Module

## Purpose

`src/render/` owns rendering systems.

It answers:

```text
How do things become pixels?
```

The render module converts already-loaded data into visual output.

---

## Current State

`src/render/` currently contains the active rendering pipeline.

Current responsibilities include:

* render pipeline orchestration
* software render backend
* triangle rasterization
* projection
* color conversion
* mesh projection
* material resolution
* texture sampling
* depth sorting
* face assembly
* visibility filtering
* scene transforms

Current structure includes:

```text
src/render/
├── RenderPipeline.cpp
├── RenderPipeline.h
├── backend/
├── camera/
├── color/
├── geometry/
├── material/
├── pipeline/
└── scene/
```

---

## Direction

Future rendering systems should continue to live under `src/render/`.

Examples:

```text
src/render/backend/gpu/
src/render/debug/
src/render/text/
src/render/viewport/
src/render/resource/
```

If a feature is about drawing, projection, render state, renderable scenes, or backend behavior, it probably belongs in `render/`.

Examples:

```text
software rasterization
= render/backend/software/

GPU rendering
= render/backend/gpu/

camera projection
= render/camera/

render object assembly
= render/scene/

texture sampling for drawing
= render/material/
```

Applications should submit scenes.

They should not own rendering internals.

---

## Standard Pattern

Rendering should follow this flow:

```text
Loaded data
    ↓
RenderObject
    ↓
RenderScene
    ↓
RenderPipeline
    ↓
RenderBackend
    ↓
Pixels
```

Application code should prepare scene input.

The render module should handle how that scene becomes pixels.

---

## Owns

`render/` owns:

* render scenes
* render objects
* render cameras
* transforms
* projection
* render pipelines
* render backends
* rasterization
* visibility stages
* depth sorting
* material resolution
* texture sampling for rendering
* render debug/stat output, when added
* future GPU backend, when added

---

## Does Not Own

`render/` does not own:

* cache decoding
* model decoding
* texture file decoding
* map decoding
* gameplay rules
* networking behavior
* live world authority
* app-specific UI workflows
* ElForge tool state
* ElClient screen flow
* ElServer behavior

---

## Used By

`render/` may be used by:

* ElForge
* ElClient

`render/` should not be used by:

* ElServer

---

## Dependency Rules

`render/` may depend on:

* `data/`
* platform libraries required for backend output

`render/` should not depend on:

* `apps/`
* `game/`
* `net/`
* app-specific viewport state
* app-specific UI code

---

## Submodule Responsibilities

### `render/scene/`

Owns renderable scene input.

Examples:

* `RenderScene`
* `RenderObject`
* transforms
* render cameras

Scene types describe what should be drawn.

They should not load assets.

---

### `render/pipeline/`

Owns render pipeline stages.

Examples:

* face assembly
* visibility filtering
* depth sorting

Pipeline stages transform renderable input into backend-ready drawing work.

---

### `render/backend/`

Owns backend-specific drawing.

Examples:

* software framebuffer output
* triangle rasterization
* future GPU backend

Backends draw.

They should not decode assets or own application behavior.

---

### `render/camera/`

Owns projection and camera math used by rendering.

---

### `render/color/`

Owns render-facing color conversion and color helpers.

---

### `render/geometry/`

Owns render-facing geometry preparation.

---

### `render/material/`

Owns rendering material behavior.

Texture sampling used while drawing belongs here.

Texture file decoding belongs in `data/texture/`.

---

## Boundary Examples

```text
data/model/ModelAsset
= static model data loaded from cache

render/scene/RenderObject
= model submitted for drawing with a transform

render/RenderPipeline
= converts the scene into backend draw work

render/backend/software
= draws pixels using CPU software rendering
```

```text
data/texture/TextureAsset
= decoded texture data

render/material/TextureSampler
= samples texture data during rendering
```

```text
apps/elforge/viewport
= tool-specific viewport controls

render/camera
= reusable render camera/projection logic
```

---

## Common Mistakes

Do not:

* decode models in `render/`
* decode textures from cache in `render/`
* put ElForge viewport controls in `render/`
* put gameplay logic in `render/`
* put world authority in `render/`
* make `render/` depend on applications
* create a second render pipeline when the existing one can be extended
* bypass `RenderScene` / `RenderObject` for normal rendering work

---

## When Adding New Render Code

Before adding code to `src/render/`:

1. Identify whether the behavior is truly rendering.
2. Check whether an existing stage already owns the behavior.
3. Extend the existing pipeline/backend/material/scene structure when possible.
4. Keep asset loading in `data/`.
5. Keep viewport UI state in the application.
6. Keep backend-specific details inside backend folders.

Good examples:

```text
src/render/backend/gpu/
src/render/debug/
src/render/material/
src/render/pipeline/
```

Bad examples:

```text
src/render/model_decoder/
src/render/elforge_viewport_controls/
src/render/game_combat_overlay/
```

---

## Golden Rule

`render/` owns how things are drawn.

It consumes loaded data.

It does not load, decode, author, or own game meaning.
