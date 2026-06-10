# Textured Model Rendering Parity Audit

## Purpose

This audit documents the current state of textured model rendering in the
ElForge/render_next pipeline. It identifies what works, what is broken, and
what is missing, to guide the implementation work in issue #95.

**Date**: 2026-06-09  
**Last updated**: 2026-06-09 (PR #101 — DepthSorter wired)  
**Scope**: `render_next` pipeline, `SoftwareRenderBackend`, `TriangleRasterizer`,
`TextureSampler`, `FaceAssembler`, `VisibilityStage`, `DepthSorter`

---

## Pipeline Overview

The current `render_next` rendering path is:

```
ModelAsset + TextureAsset
        ↓
   MeshProjector (world → screen)
        ↓
   FaceAssembler (build render packets)
        ↓
   VisibilityStage (back-face culling)
        ↓
   DepthSorter (wired into pipeline since PR #101)
        ↓
   SoftwareRenderBackend::drawObject
        ↓
   TriangleRasterizer (solid or textured)
        ↓
   Framebuffer → SDL_Texture → SDL_RenderTexture
```

---

## What Works

### Solid Triangle Rendering
- Flat color from RuneScape 16-bit `face.color` via `rsColorToRgb`
- Per-pixel depth test-and-write
- Back-face culling via screen-space area sign
- Alpha blending in `drawPixel` (solid triangles only)

### Depth Sorting (PR #101)
- `DepthSorter::sort` is called in `RenderPipeline::render()` after `VisibilityStage::apply` and before `backend.drawObject`
- Render packets are sorted back-to-front by average depth before drawing
- Per-pixel depth test still provides a second layer of correctness
- Remains a follow-up: priority-aware sorting (see Moderate issue #4)

### Texture Loading
- `TextureLoader` reads from cache index 0, archive 6
- `TextureFileReader` parses index data (palette, metadata, indexed pixels)
- `TexturePixelDecoder` handles type 0 (row-major) and type 1 (column-major)
- `TextureCanvasDecoder` places pixels on canvas with xOffset/yOffset
- `ModelLoader::loadModelTextures` loads textures referenced by model faces

### Texture Sampling
- `TextureSampler::sample(u, v)` maps UV to canvas pixel coordinates
- Clamps u to [0, 1], wraps v via `v - floor(v)`
- Returns nullptr for out-of-range or empty textures

### Textured Triangle Rendering
- Detected by `renderType == 2 || renderType == 3`
- Requires valid `textureUVMappingIndex` and presence of `TextureAsset` in model
- UV coordinates computed via inverse bilinear mapping on world-space basis vectors
- Depth test applied per-pixel
- Transparent pixels (alpha == 0) and black pixels (0,0,0) skipped

---

## Known Issues

### Critical

1. ~~DepthSorter not called~~ **Fixed in PR #101**
   - `DepthSorter::sort` is now called in `RenderPipeline::render()` after `VisibilityStage::apply` and before `backend.drawObject`
   - Face ordering now sorts back-to-front by average depth before drawing

2. **Canvas offset not accounted in TextureSampler**
   - `TextureCanvasDecoder` places texture pixels at `(xOffset, yOffset)` on a larger canvas
   - `TextureSampler` uses `canvasWidth/canvasHeight` for UV mapping but ignores the offset
   - Textures that use `xOffset > 0` or `yOffset > 0` will be sampled from the wrong position

3. **Face alpha ignored for textured triangles**
   - `RenderPacket.alpha` is populated from `Face.alpha`
   - `TriangleRasterizer::drawTexturedTriangle` reads alpha from the texture pixel, not the face
   - Models with semi-transparent faces (alpha > 0) will render fully opaque if the texture pixel is opaque

### Moderate

4. **Priority ordering not implemented**
   - `RenderPacket.priority` is stored but never used for sorting
   - RuneScape uses priority buckets to control draw order for overlapping geometry
   - Without priority sorting, overlapping objects may render in wrong order

5. **Texture wrapping may be incorrect**
   - `TextureSampler` clamps u and wraps v, but the original client may use different wrapping per texture type
   - No wrapping mode is stored in the texture metadata

6. **Black pixel transparency is aggressive**
   - Both `(0,0,0)` with any alpha and `alpha == 0` pixels are skipped
   - Some RuneScape textures use black as a valid color; this may cause holes in rendered models

### Minor

7. **highlightTexturedFaces_ flag not wired**
   - `SoftwareRenderBackend` has the flag but `drawObject` never reads it
   - The T key in ElForge sets the flag but it has no visual effect in the `render_next` path

8. **No wireframe/debug/vertex rendering modes**
   - `render_next` always renders filled triangles
   - The legacy `render` module has wireframe support but `render_next` does not

9. **RenderOptions not consumed**
   - `render_next` does not accept or use `RenderOptions` (fill, wireframe, vertex, alpha flags)
   - All rendering decisions are hardcoded

---

## Model Test Cases

The following model IDs are recommended for manual validation. These assume a
standard RS317 cache. If a model is not present in your cache, skip it.

### Solid Rendering Tests

| Model ID | Expected Behavior | What to Check |
|----------|-------------------|---------------|
| 99 | Renders as a simple solid object | Flat colors, no crash, correct vertex/face count in inspector |
| 100 | Renders with multiple solid colors | Different colored faces visible, no rainbow artifacts |

### Textured Rendering Tests

| Model ID | Expected Behavior | What to Check |
|----------|-------------------|---------------|
| 101 | Textured faces visible | Texture data appears on faces, not solid black |
| 200 | Multiple textures | Different textures visible on different face groups |
| 250 | Transparency/alpha | Some faces partially transparent or invisible (alpha handling) |
| 500 | Complex textured | Many textures, no crash, reasonable visual output |

### Error Handling Tests

| Model ID | Expected Behavior | What to Check |
|----------|-------------------|---------------|
| 1 | Graceful failure | "not found" or "truncated" error, no crash |
| 1000 | Graceful failure | "not found" or "truncated" error, no crash |

---

## Texture Test Cases

Use the ElForge texture browser panel to validate:

| Texture ID | Expected Behavior | What to Check |
|------------|-------------------|---------------|
| 0 | Loads or shows error | Preview visible or error message, no crash |
| 1 | Palette texture | Correct colors from palette |
| 10 | Canvas offset | Pixel data placed correctly on canvas |
| 50 | Typical texture | Correct dimensions, pixel data visible |

---

## Recommendations for Future PRs

Priority order for fixing textured rendering (updated after PR #101):

1. ~~Wire the DepthSorter~~ — **Done in PR #101**
2. **Fix TextureSampler canvas offset** — Account for `xOffset/yOffset` when computing pixel index from UV coordinates
3. **Fix face alpha for textured triangles** — Use `packet.alpha` to modulate the sampled texture pixel alpha
4. **Implement priority-aware sorting** — Sort by priority bucket first, then by depth within each bucket
5. **Wire highlightTexturedFaces_** — In `drawObject`, when the flag is set, outline or tint textured faces differently
6. **Add wireframe mode** — Draw triangle edges instead of filled polygons

---

## Files Audited

- `src/render_next/RenderPipeline.h/.cpp`
- `src/render_next/pipeline/FaceAssembler.h/.cpp`
- `src/render_next/pipeline/RenderPacket.h`
- `src/render_next/pipeline/RenderQueue.h`
- `src/render_next/pipeline/DepthSorter.h/.cpp`
- `src/render_next/pipeline/VisibilityStage.h/.cpp`
- `src/render_next/backend/software/SoftwareRenderBackend.h/.cpp`
- `src/render_next/backend/software/TriangleRasterizer.h/.cpp`
- `src/render_next/backend/software/Framebuffer.h`
- `src/render_next/backend/software/ColorBuffer.h`
- `src/render_next/backend/software/DepthBuffer.h`
- `src/render_next/material/TextureSampler.h/.cpp`
- `src/render_next/material/MaterialResolver.h/.cpp`
- `src/render_next/scene/RenderCamera.h`
- `src/render_next/scene/RenderObject.h`
- `src/render_next/scene/Transform.h/.cpp`
- `src/render_next/geometry/MeshProjector.h/.cpp`
- `src/render_next/geometry/ProjectedMesh.h`
- `src/data/texture/TextureAsset.h`
- `src/data/texture/TextureLoader.h/.cpp`
- `src/data/texture/TextureFileReader.h/.cpp`
- `src/data/texture/TextureBuilder.h/.cpp`
- `src/data/texture/decoder/TexturePixelDecoder.h/.cpp`
- `src/data/texture/decoder/TextureCanvasDecoder.h/.cpp`
- `src/data/model/ModelAsset.h`
- `src/data/model/ModelLoader.h/.cpp`
- `src/data/model/ModelBuilder.h/.cpp`
- `src/data/model/ModelFileReader.h/.cpp`
- `src/data/model/decoder/VertexDecoder.h/.cpp`
- `src/data/model/decoder/FaceDecoder.h/.cpp`
- `src/data/model/decoder/TextureUVMappingDecoder.h/.cpp`
- `src/apps/elforge/panels/cache/CacheViewportPanel.cpp`
- `src/apps/elforge/ElForgeApplication.cpp`
