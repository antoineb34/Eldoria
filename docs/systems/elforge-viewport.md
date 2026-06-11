# ElForge Viewport System

## Purpose

The ElForge viewport system displays loaded assets inside the ElForge tool UI.

It answers:

```text
How does ElForge show a loaded model inside an ImGui panel?
```

This system belongs to:

```text
src/apps/elforge/
```

It is application-specific.

It bridges:

```text
ElForge UI
    ↓
ElForge state
    ↓
render/ scene
    ↓
render/ backend
```

---

## Current State

The viewport currently supports:

* ImGui viewport panel layout
* viewport bounds calculation
* viewport background fill
* SDL clipping to viewport bounds
* active model display
* keyboard transform controls
* render camera viewport synchronization
* `RenderObject` creation
* `RenderScene` creation
* software backend rendering
* textured-face debug highlight toggle

Current main file:

```text
src/apps/elforge/panels/CacheViewportPanel.cpp
```

Related state:

```text
src/apps/elforge/CacheExplorerState.h
```

---

## Data Flow

```text
Cache tree / model selection
    ↓
CacheExplorerState::activeModel
    ↓
CacheViewportPanel
    ↓
viewport bounds from ImGui
    ↓
camera viewport update
    ↓
keyboard transform controls
    ↓
RenderObject
    ↓
RenderScene
    ↓
SoftwareRenderBackend
    ↓
RenderPipeline
    ↓
SDL renderer output
```

---

## Important Responsibilities

### `CacheViewportPanel::render()`

Owns ImGui layout for the viewport area.

Responsibilities:

* create the child panel
* display viewport label
* determine viewport screen position
* determine viewport size
* store viewport bounds in `CacheExplorerState`
* reserve the viewport area with `ImGui::Dummy`

This function defines **where** the viewport is.

It does not render the model.

---

### `CacheViewportPanel::renderViewport()`

Owns drawing the active model into the viewport.

Responsibilities:

* return early if no active model exists
* copy viewport bounds into render camera
* set SDL clip rect
* fill viewport background
* update viewport controls
* build a `RenderObject`
* build a `RenderScene`
* create software backend
* configure debug highlight mode
* call `RenderPipeline::render()`
* reset SDL clip rect

This function bridges ElForge state into the render module.

---

### `updateViewportControls()`

Owns temporary tool-side keyboard controls.

Current controls:

```text
Left / Right
    rotate Y

Up / Down
    rotate X

Q / E
    rotate Z

Equals / Minus
    zoom scale

W / S
    move Y

A / D
    move X

R
    reset transform

T
    toggle textured-face highlight
```

These controls are ElForge tool behavior.

They do not belong in `render/`.

---

## Important State

The viewport currently uses `CacheExplorerState`.

Relevant state includes:

```text
activeModel
camera
modelTransform
viewportX
viewportY
viewportWidth
viewportHeight
debugHighlightTexturedFaces
```

`CacheExplorerState` is application state.

It should not leak into shared modules.

---

## Relationship To Render

The viewport prepares render input.

It does not own rendering internals.

Current render bridge:

```text
activeModel
    ↓
RenderObject.model

modelTransform
    ↓
RenderObject.transform

camera
    ↓
RenderScene.camera

RenderScene
    ↓
RenderPipeline
```

The viewport may create render objects and render scenes.

The viewport should not:

* project meshes
* assemble render packets
* sort faces
* rasterize triangles
* sample textures

Those belong in `src/render/`.

---

## Ownership

The ElForge viewport system owns:

* viewport panel layout
* viewport bounds
* tool-side model transform controls
* selected model display
* debug viewport toggles
* converting app state into a render scene
* calling the render pipeline

The ElForge viewport system does not own:

* model loading internals
* texture loading internals
* render pipeline internals
* rasterization
* cache sector reading
* gameplay behavior
* reusable render architecture

---

## Extension Points

Add viewport behavior here when the behavior is about:

* ElForge viewport UI
* model inspection controls
* tool debug toggles
* viewport-specific input handling
* converting selected assets into render scenes

Examples:

Good:

```text
add reset camera button
add viewport control help text
add wireframe debug toggle if render supports it
add model bounds display toggle
```

Bad:

```text
decode model vertices
load texture archive files
implement triangle rasterization
add render backend architecture
add gameplay object interaction
```

---

## Common Mistakes

Do not:

* put model decoding in the viewport
* put texture decoding in the viewport
* put rasterization in the viewport
* put generic render pipeline logic in ElForge
* make `render/` depend on `CacheExplorerState`
* make `data/` depend on ElForge viewport state
* bypass `RenderScene` for normal rendering
* duplicate render pipeline behavior in the panel

---

## When Adding New Code

Before changing the viewport system:

1. Determine whether the behavior is tool UI, rendering, or data loading.
2. Keep tool UI and inspection controls in ElForge.
3. Keep rendering internals in `render/`.
4. Keep data loading in `data/`.
5. Convert app state into `RenderScene` instead of bypassing the render system.
6. Avoid making shared modules depend on ElForge state.

---

## Verification

Useful verification steps:

```bash
cmake --build build
```

Manual checks:

```text
Open ElForge.
Select a model from the cache tree.
Verify the model appears in the viewport.
Rotate with arrow keys.
Move with WASD.
Zoom with plus/minus.
Reset with R.
Toggle textured-face highlight with T.
Resize the window and verify the viewport still draws in the correct area.
```

If render integration changes, also verify:

```text
SDL clipping resets after viewport rendering.
Other UI panels still draw correctly.
The viewport background stays inside the viewport bounds.
```

---

## Golden Rule

ElForge viewport code prepares tool state for rendering.

It should not become a renderer, decoder, or asset loader.
