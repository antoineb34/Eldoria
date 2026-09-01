# render

The render module turns prepared graphics resources into pixels.

## Responsibilities

- Camera and projection calculations
- Scene objects and transforms
- Render submission
- Backend-independent rendering interfaces
- Hardware OpenGL rendering
- Software rendering
- Framebuffer, depth testing, texture sampling, and rasterization

## Boundaries

The render module consumes normalized types from `graphics`.

It does not:

- Read cache files
- Call repositories
- Decode models or textures
- Interpret RuneScape-specific model data
- Resolve source model or texture IDs

Source-specific visual conversion belongs to `graphics`. Shared vectors and matrices belong to `math`.

## Flow

```text
data repositories
    -> graphics resources
    -> render scene
    -> render pipeline
    -> rendering backend
    -> pixels

## Backends

`OpenGLRenderBackend` is the primary hardware path. It consumes stable
`ModelHandle` / `TextureHandle` values, uploads normalized graphics resources
once, and keeps the resulting GPU buffers and textures resident for later
frames. The current baseline is OpenGL 3.3 core.

`SoftwareRenderBackend` remains a CPU reference/fallback implementation. It
uses the same `IRenderBackend` contract and normalized graphics resources.

Backend code must stay source-agnostic: RuneScape map/model interpretation
continues to belong to `graphics` and `data`, not to the GPU backend.
