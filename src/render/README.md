# render

The render module turns prepared graphics resources into pixels.

## Responsibilities

- Camera and projection calculations
- Scene objects and transforms
- Render submission
- Backend-independent rendering interfaces
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
