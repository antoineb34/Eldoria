# Rendering System

## Purpose

The rendering system turns loaded asset data into pixels.

It answers:

```text
How does a ModelAsset become rendered output?
```

The rendering system belongs to:

```text
src/render/
```

---

## Current State

The rendering system currently supports:

* render scenes
* render objects
* render cameras
* object transforms
* mesh projection
* face packet assembly
* back-face visibility filtering
* depth sorting
* software rendering backend
* solid triangle rasterization
* textured triangle rasterization
* SDL texture output

Current files include:

```text
src/render/
├── RenderPipeline.cpp
├── RenderPipeline.h
├── backend/
├── camera/
├── color/
├── geometry/
├── material/
├── math/
├── pipeline/
└── scene/
```

---

## Data Flow

```text
ModelAsset
    ↓
RenderObject
    ↓
RenderScene
    ↓
RenderPipeline
    ↓
MeshProjector
    ↓
FaceAssembler
    ↓
VisibilityStage
    ↓
DepthSorter
    ↓
IRenderBackend
    ↓
SoftwareRenderBackend
    ↓
TriangleRasterizer
    ↓
Framebuffer
    ↓
SDL texture
    ↓
pixels
```

---

## Important Types

### `RenderScene`

Top-level render input.

Contains:

```text
RenderCamera
RenderObject list
```

A scene describes what should be drawn for one frame.

---

### `RenderObject`

Renderable object instance.

Contains:

```text
ModelAsset pointer
Transform
```

`RenderObject` references loaded model data.

It does not own model loading.

---

### `RenderCamera`

Camera and viewport configuration used during rendering.

Contains viewport dimensions and camera/projection state.

---

### `Transform`

Per-object transform.

Used to position, rotate, and scale objects before projection.

---

### `RenderPipeline`

Main pipeline coordinator.

Responsibilities:

* begin backend frame
* project object meshes
* assemble render packets
* apply visibility filtering
* sort packets by depth
* send draw work to backend
* end backend frame

The pipeline coordinates stages.

It should not decode assets or own application UI.

---

### `MeshProjector`

Projects model vertices into render space.

Flow:

```text
model vertex
    ↓
local space
    ↓
world space
    ↓
view space
    ↓
projected space
    ↓
screen space
```

Current behavior flips model Y during local conversion.

---

### `FaceAssembler`

Builds render packets from model faces.

Responsibilities:

* validate face indices
* ensure projected vertices are valid
* copy render-facing face metadata into `RenderPacket`
* calculate depth values

---

### `VisibilityStage`

Filters invisible packets.

Current behavior:

* performs screen-space face orientation check
* removes back-facing triangles

---

### `DepthSorter`

Sorts render packets.

Current behavior:

```text
larger depthAvg first
```

---

### `IRenderBackend`

Backend interface.

Public methods:

```text
beginFrame(camera)
drawObject(object, mesh, queue)
endFrame()
```

The pipeline targets this interface rather than a concrete backend.

---

### `SoftwareRenderBackend`

Current active backend.

Responsibilities:

* manage software framebuffer
* manage SDL output texture
* draw solid triangles
* draw textured triangles
* upload framebuffer to SDL texture
* render SDL texture to viewport

---

### `TriangleRasterizer`

Draws triangles into the software framebuffer.

Current backend uses it for:

* solid triangles
* textured triangles

---

## Pipeline Flow

`RenderPipeline::render()` is the main rendering path.

Flow:

```text
backend.beginFrame(scene.camera)

for each object:
    projector.project(object, camera)
    faceAssembler.assemble(object, mesh)
    visibilityStage.apply(queue, mesh)
    depthSorter.sort(queue)
    backend.drawObject(object, mesh, queue)

backend.endFrame()
```

The pipeline is object-oriented.

Each object is projected, packetized, filtered, sorted, and drawn.

---

## Software Backend Flow

`SoftwareRenderBackend` receives backend-ready draw work.

Flow:

```text
beginFrame
    resize framebuffer
    clear framebuffer
    recreate SDL texture

drawObject
    for each render packet:
        if textured packet:
            draw textured triangle
        else:
            draw solid triangle

endFrame
    copy framebuffer pixels
    SDL_UpdateTexture
    SDL_RenderTexture
```

Textured rendering requires:

```text
renderType 2 or 3
valid texture UV mapping index
texture present in ModelAsset::textures
```

If those conditions are not met, the backend draws a solid triangle.

---

## Ownership

The rendering system owns:

* scene rendering
* object projection
* face packet assembly
* visibility filtering
* depth sorting
* render backend abstraction
* software rasterization
* render-time texture sampling
* framebuffer output
* SDL texture upload for the software backend

The rendering system does not own:

* model loading
* texture file loading
* cache access
* gameplay rules
* world authority
* ElForge viewport controls
* ElClient screen flow
* ElServer behavior

---

## Extension Points

Add rendering behavior here when the behavior is about:

* projection
* visibility
* sorting
* render packets
* rasterization
* render backend behavior
* material sampling
* render debug output
* future GPU backend

Examples:

Good:

```text
add render stats
improve depth sorting
add GPU backend
add material resolver behavior
improve textured triangle rasterization
```

Bad:

```text
decode model files
decode texture archive files
add ElForge model-selection UI
add gameplay overlays
load cache files directly
```

---

## Common Mistakes

Do not:

* decode models in `render/`
* decode textures from cache in `render/`
* bypass `RenderScene` / `RenderObject` for normal rendering
* put ElForge viewport controls in `render/`
* put gameplay logic in `render/`
* make render depend on applications
* create a parallel render pipeline when the existing one can be extended

---

## When Adding New Code

Before changing rendering code:

1. Determine which stage owns the behavior.
2. Extend the existing stage if possible.
3. Keep asset loading in `data/`.
4. Keep UI state in the application.
5. Keep backend-specific behavior inside backend folders.
6. Keep generic render behavior outside backend folders.
7. Preserve the `RenderScene -> RenderPipeline -> RenderBackend` flow.

---

## Verification

Useful verification steps:

```bash
cmake --build build
```

Manual checks:

```text
Open ElForge.
Load a known model id.
Rotate the model.
Verify solid faces render.
Verify textured faces render.
Verify viewport size changes still work.
Verify repeated frames do not crash.
```

If backend behavior changes, verify:

```text
solid triangles
textured triangles
alpha handling
viewport placement
depth ordering
```

---

## Golden Rule

The rendering system draws already-loaded data.

It does not load assets, decode cache formats, or own application UI.
