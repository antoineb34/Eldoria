# ElForge Render Parity Checklist

## Purpose

This checklist is for PR reviewers and implementers working on model rendering.
It defines the expected behavior that must be preserved or implemented before
a rendering change can be considered correct.

Use this during code review of any PR that touches:
- model loading / decoding
- the render pipeline (`render`, `render_next`)
- the ElForge viewport / model viewer
- texture loading / texture sampling

---

## 1. Filled Triangle Rendering

| Check | Expected | Status |
|-------|----------|--------|
| Solid triangles render with the correct flat color from `face.color` | Yes | Implemented |
| Color uses RuneScape 16-bit RGB conversion (`rsColorToRgb`) | Yes | Implemented |
| Depth buffer test-and-write prevents far triangles from overwriting near ones | Yes | Implemented |
| Back-face culling rejects faces with negative screen-space area | Yes | Implemented (`VisibilityStage`) |

## 2. Textured Triangle Rendering

| Check | Expected | Status |
|-------|----------|--------|
| Textured triangles are detected by `renderType == 2 \|\| renderType == 3` | Yes | Implemented |
| Texture UV mapping requires a valid `textureUVMappingIndex` | Yes | Implemented |
| Texture lookup requires the model to have a `TextureAsset` for `face.color` | Yes | Implemented |
| UV coordinates are computed via inverse bilinear mapping on the texture basis | Yes | Implemented |
| Texture sampling uses `TextureSampler::sample(u, v)` | Yes | Implemented |
| `TextureSampler` reads from `canvasWidth x canvasHeight` pixel grid | Yes | Implemented |
| Transparent texture pixels (alpha == 0) are skipped | Yes | Implemented |
| Black pixels `(0,0,0)` are skipped as transparent (classic RS behavior) | Yes | Implemented |
| Depth test is applied per-pixel for textured triangles | Yes | Implemented |
| Alpha blending is applied when `pixel.a < 255` | Partial | Applied in `drawPixel` but only for solid triangles; textured triangles use the alpha from the sampled pixel directly |

### Known Textured Rendering Issues

- **Texture wrapping**: `TextureSampler` clamps `u` to `[0, 1]` and wraps `v` via `v - floor(v)`. This may not match the wrapping mode used by the original RuneScape client for all texture types.
- **Canvas vs image dimensions**: The sampler uses `canvasWidth/canvasHeight` but the decoded pixel data may represent a smaller image placed on a larger canvas. The `TextureCanvasDecoder` places pixels using `xOffset/yOffset` but the sampler does not account for this offset.
- **Priority ordering**: The `RenderPacket` carries a `priority` field from the model face but the `render_next` pipeline does NOT sort or filter by priority. The `DepthSorter` sorts by `depthAvg` only.
- **Texture UV mapping accuracy**: The UV mapping uses world-space vertex positions projected onto a basis derived from the `TextureUVMapping` (origin, uVertex, vVertex). This is a reasonable approximation but has not been parity-tested against known-good renders.
- **Alpha for textured faces**: The `alpha` field from `Face` is stored in the `RenderPacket` but the textured triangle rasterizer reads alpha from the sampled texture pixel instead of the face alpha value.
- **`renderType` 0 and 1**: Treated as solid/untextured. Untextured faces use `face.color` as an RS 16-bit color.

## 3. Depth / Face Ordering

| Check | Expected | Status |
|-------|----------|--------|
| Depth sorting orders faces back-to-front by average depth | Partial | `DepthSorter` exists but is **not called** by `RenderPipeline::render()` |
| `DepthSorter::sort` sorts `RenderQueue` packets | Yes | Implemented but unused |
| Priority field is preserved on packets for future sorting | Yes | Implemented |
| RuneScape priority-aware ordering (priority bucket then depth) | **Missing** | Not implemented |
| Depth test per-pixel in framebuffer | Yes | Implemented |

## 4. Debug / Wireframe / Vertex Modes

| Check | Expected | Status |
|-------|----------|--------|
| Wireframe rendering | **Missing** | Not implemented in `render_next` |
| Vertex point rendering | **Missing** | Not implemented in `render_next` |
| Textured-face debug highlight | Partial | `highlightTexturedFaces_` flag exists in `SoftwareRenderBackend` but is not wired to any visual output in `drawObject` |
| `RenderOptions` / `RenderOptions` flags | **Missing** | `render_next` does not consume `RenderOptions`; the legacy `render` module still does |
| Face fill toggle (`fillTriangles`) | **Missing** | Always fills; no wireframe path |
| Alpha toggle | **Missing** | Alpha is always applied from pixel data |

## 5. Camera and Transform Controls

| Check | Expected | Status |
|-------|----------|--------|
| Camera position, rotation, FOV, near/far planes are configurable | Yes | `RenderCamera` aliases `rf::render::Camera` |
| Model transform (position, rotation, scale) applied per-object | Yes | `RenderObject::transform` applied in `MeshProjector` |
| Keyboard-driven rotation (left/right/up/down/Q/E) | Yes | `CacheViewportPanel::updateViewportControls` |
| Keyboard-driven movement (W/A/S/D) | Yes | Implemented |
| Keyboard-driven zoom (+/-) | Yes | Implemented |
| Reset transform (R key) | Yes | Implemented |
| Textured-face debug toggle (T key) | Yes | Flag set but not visually wired in `render_next` |

## 6. Missing / Error States

| Check | Expected | Status |
|-------|----------|--------|
| Missing model file shows error message | Yes | `ModelLoadResult::MissingCacheFile` |
| Corrupt/unsupported compression shows error | Yes | `UnsupportedCompression`, `DecompressionFailed` |
| Empty model (no faces) shows error | Yes | `EmptyModel` |
| Missing texture in cache returns `std::nullopt` | Yes | `TextureLoader::load` returns empty optional |
| Model with missing texture references renders without those faces | Yes | `isTexturedPacket` returns false if texture not found |
| Invalid face indices are rejected | Yes | `isValidFace` check in `FaceAssembler` |
| Degenerate triangles (zero area) are rejected | Yes | Area check in rasterizer |

## 7. Model Loading Pipeline

| Check | Expected | Status |
|-------|----------|--------|
| Model file read from cache index 1 | Yes | `ModelLoader::getModelFile` |
| Decompression (Gzip, Bzip2) | Yes | `ModelLoader::decompressPayload` |
| Model file parsing (footer validation) | Yes | `ModelFileReader::read` |
| Vertex decoding (3 parts: positions, normals, UVs) | Yes | `VertexDecoder` |
| Face decoding (color, renderType, texture, priority, alpha) | Yes | `FaceDecoder` |
| Texture UV mapping decoding | Yes | `TextureUVMappingDecoder` |
| Texture loading for textured faces | Yes | `ModelLoader::loadModelTextures` |
| Model caching (avoid re-load) | Yes | `modelCache_` in `ModelLoader` |

## 8. Texture Loading Pipeline

| Check | Expected | Status |
|-------|----------|--------|
| Texture file read from cache index 0, archive 6 | Yes | `TextureLoader::getTextureFile` |
| Texture index file parsing (palette, metadata, pixels) | Yes | `TextureFileReader::read` |
| Indexed pixel decoding (type 0: row-major, type 1: column-major) | Yes | `TexturePixelDecoder` |
| Canvas placement (xOffset, yOffset, canvasWidth, canvasHeight) | Yes | `TextureCanvasDecoder` |
| Palette color resolution (RGB -> RGBA) | Yes | `TextureCanvasDecoder::resolveColor` |
| Texture caching | Yes | `textureCache_` in `TextureLoader` |

---

## PR Review Checklist

Before approving a rendering PR, verify:

- [ ] All checks in sections 1-6 that were "Implemented" still pass
- [ ] New rendering features include a row in the appropriate table above
- [ ] Known issues section is updated if a fix changes behavior
- [ ] `cmake --build build --target elforge -j` succeeds
- [ ] Model browsing in ElForge still works (select a model in the tree, it renders)
- [ ] Texture browsing in ElForge still works (enter texture id, Load, preview appears)
- [ ] No regression in `explorer`, `elclient`, `elserver`, `archive_probe` builds
