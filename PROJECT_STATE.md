# RuneForge Project State

## Current Focus

Building a clean RuneScape 317 cache exploration, asset decoding, and rendering foundation in modern C++.

The current focus is:

* cache architecture
* asset decoding
* software rendering
* tooling workflows
* preparing the foundation for a future game client

---

# Current Status

## Working

### Cache Pipeline

* DAT/IDX FileStore reading
* Archive extraction
* GZIP decompression
* Binary parsing utilities

### Asset Decoding

* Model loading
* Vertex decoding
* Face decoding
* Texture UV mapping decoding
* Texture loading
* Texture canvas decoding
* Texture pixel decoding

### Rendering

* Software rasterizer
* Model rendering
* Texture mapping
* Backface culling
* Priority-based face rendering
* RuneScape-style transparency handling
* Interactive model viewer

### Tooling

* SDL3 application framework
* Cache browser
* Cache tree explorer
* Asset inspection panels
* Model preview system

---

## In Progress

### Research

* Map decoding
* Animation decoding
* Configuration decoding
* Scene rendering

### Architecture

* Runtime asset structures
* Renderer abstraction
* GPU rendering foundation
* Client architecture

---

## Unknown / Research Areas

* Some textured face edge cases
* Remaining model rendering quirks
* Exact map scene pipeline
* Animation data pipeline
* Long-term renderer architecture

---

# Current Architecture

## Pipeline

```text
Cache Files
    ↓

DAT/IDX FileStore
    ↓

Archive Extraction
    ↓

Decompression
    ↓

Binary Parsing
    ↓

Decoded Asset
    ↓

Runtime Asset
    ↓

Renderer / Tooling
```

---

## Source Layout

```text
src/

apps/
├── tool/
└── client/

core/
├── assets/
│   ├── model/
│   └── texture/
│
├── cache/
├── compression/
├── io/
└── references/

render/
├── model/
└── software/

platform/
└── sdl/

ui/
```

---

# Important Discoveries

### Models

* Face decoding pipeline is functioning correctly.
* Overlapping faces are intentional in many RuneScape assets.
* Face priorities are important for correct rendering.
* Backface culling is required for proper visual results.

### Textures

* Texture UV mapping pipeline is functioning.
* Texture sampling should clamp UVs rather than wrap.
* Black texture pixels are often used as transparency.
* Texture data remains separate from model geometry.

### Architecture

* Cache structures should remain separate from runtime structures.
* Asset decoding should remain separate from rendering.
* Tool-first development continues to validate architecture decisions.
* Understanding the original formats is more valuable than premature abstractions.

---

# Immediate Next Steps

1. Clean remaining rendering edge cases
2. Begin map decoding investigation
3. Begin animation decoding investigation
4. Define runtime asset structures
5. Define future GPU renderer architecture
6. Begin planning client architecture

---

# Long-Term Goals

* Complete cache exploration tooling
* Full asset decoding coverage
* Scene rendering
* Modern rendering backend
* Asset editing workflows
* Custom RuneScape-inspired client

---

# Session Notes

The model and texture pipeline has reached a stable state suitable for continued research.

Current renderer behavior is sufficiently accurate for asset exploration and debugging purposes.

Future work should focus on expanding asset coverage rather than rewriting existing model and texture systems.
