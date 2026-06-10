# Known Model Regression Set

## Purpose

This document defines a small set of known model IDs that should be used to
validate model loading, textured rendering, and asset pipeline behavior across
future changes.

Since we cannot commit external RuneScape cache data, these IDs reference
models that are expected to exist in a standard RS317 cache. If a model ID
is not present in your local cache, skip that case and note it.

---

## Regression Models

### Basic / Solid Models

| Model ID | Name/Type | What it validates | Notes |
|----------|-----------|-------------------|-------|
| 0 | Dummy/placeholder | Basic model loading, empty or minimal geometry | Should load without error even if degenerate |
| 1 | Simple object | Low vertex/face count, solid colors | Known to have truncated payload in some caches; should fail gracefully |
| 99 | Low-poly object | Basic solid triangle rendering | Good smoke test for the render pipeline |
| 100 | Medium object | Moderate face count, multiple materials | Validates face iteration and color conversion |

### Textured Models

| Model ID | Name/Type | What it validates | Notes |
|----------|-----------|-------------------|-------|
| 101 | Textured object | Texture loading, UV mapping, textured triangle fill | Must have `renderType == 2` or `3` faces with valid `textureUVMappingIndex` |
| 200 | Multi-texture object | Multiple textures on one model | Validates that `ModelLoader::loadModelTextures` loads multiple textures |
| 250 | Alpha/transparency object | Alpha blending in textured faces | Look for faces with `alpha > 0` or texture pixels with `a == 0` |
| 500 | Complex textured object | High face count + textures | Stress test for the rasterizer and depth sorting |

### Large / Stress Models

| Model ID | Name/Type | What it validates | Notes |
|----------|-----------|-------------------|-------|
| 1000 | High-poly model | Large vertex/face arrays, memory handling | Known to have truncated payload in some caches; should fail gracefully |

---

## Per-Model Validation Checklist

For each model in the regression set, verify:

### Structural

- [ ] Model loads without crashing (even if it fails gracefully with an error message)
- [ ] Vertex count is reported correctly in the ElForge inspector
- [ ] Face count is reported correctly
- [ ] Texture UV mapping count is reported correctly
- [ ] Loaded texture count matches the number of unique texture IDs referenced by faces

### Visual (ElForge Viewport)

- [ ] Model renders in the viewport (not a blank screen)
- [ ] Solid faces show the expected flat colors (no rainbow artifacts)
- [ ] Textured faces show texture data (not solid black)
- [ ] Camera controls work: rotation (arrows/Q/E), movement (WASD), zoom (+/-), reset (R)
- [ ] No depth-fighting artifacts on faces at similar depths
- [ ] Model does not disappear when rotated (back-face culling is correct)

### Error Cases

- [ ] Model ID not in cache: shows "not found" error, no crash
- [ ] Model with corrupt/truncated data: shows decode error, no crash
- [ ] Model with missing textures: renders with textured faces falling back to solid (no crash)

---

## Texture Regression Set

| Texture ID | What it validates | Notes |
|------------|-------------------|-------|
| 0 | Basic texture load | Should load or gracefully fail |
| 1 | Palette texture | Validates palette color resolution |
| 10 | Canvas offset texture | Validates xOffset/yOffset placement |
| 50 | Typical texture | Common texture for model face references |
| 100 | Large texture | Validates larger canvas dimensions |

### Per-Texture Validation (ElForge Texture Browser)

- [ ] Valid texture id shows pixel preview in the texture panel
- [ ] Valid texture id shows metadata: dimensions, palette count, pixel count, transparency %
- [ ] Invalid texture id shows error message, no crash
- [ ] Texture preview scales to fit the panel (no overflow)
- [ ] Transparent pixels are not drawn (alpha == 0 skipped)

---

## How to Use This Document

1. **Before a PR**: Run through the relevant sections above with your local cache.
2. **During PR review**: Check that the "Status" column entries in the parity checklist are still accurate.
3. **After merge**: If new behavior is added or a known issue is fixed, update this document.

This document is expected to evolve as the renderer matures and more parity issues are discovered or resolved.
