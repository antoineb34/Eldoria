# Texture Loading System

## Purpose

The texture loading system reads RuneScape texture data from the cache and turns it into reusable `TextureAsset` data.

It answers:

```text
How does a texture id become decoded texture asset data?
```

The texture loading system belongs to:

```text
src/data/texture/
```

---

## Current State

The texture loading system currently supports:

* loading the texture archive from the config cache index
* decoding archive files through `ArchiveReader`
* finding texture data by archive file index
* finding shared texture index metadata
* reading texture metadata
* reading texture palettes
* reading indexed pixels
* converting indexed pixels into canvas pixels
* caching loaded textures by id

Current files:

```text
src/data/texture/
├── decoder/
│   ├── TextureCanvasDecoder.cpp
│   ├── TextureCanvasDecoder.h
│   ├── TexturePixelDecoder.cpp
│   └── TexturePixelDecoder.h
├── TextureAsset.h
├── TextureBuilder.cpp
├── TextureBuilder.h
├── TextureFile.h
├── TextureFileReader.cpp
├── TextureFileReader.h
├── TextureLoader.cpp
└── TextureLoader.h
```

---

## Data Flow

```text
texture id
    ↓
TextureLoader
    ↓
Cache::readFile(CacheIndex::Config, 6)
    ↓
ArchiveReader
    ↓
Archive
    ↓
texture file by index
index file by final archive index
    ↓
TextureFileReader
    ↓
TextureFile
    ↓
TextureBuilder
    ↓
TexturePixelDecoder
TextureCanvasDecoder
    ↓
TextureAsset
    ↓
cached TextureAsset
```

---

## Important Types

### `TextureLoader`

Main public entry point for loading textures.

Public API:

```text
load(id)
```

Responsibilities:

* load texture archive `6` from `CacheIndex::Config`
* decode the archive through `ArchiveReader`
* find the requested texture file by index
* find the shared index metadata file
* ask `TextureFileReader` to read texture data
* ask `TextureBuilder` to build `TextureAsset`
* cache loaded textures by id

---

### `TextureFileReader`

Reads raw texture file data into `TextureFile`.

Responsibilities:

* read index offset from texture data
* read palette count
* read palette colors
* read texture metadata
* read indexed pixels

It combines:

```text
index data
texture data
```

into one `TextureFile`.

---

### `TextureFile`

Represents raw parsed texture data.

Contains:

* texture id
* palette
* metadata
* indexed pixels

---

### `TextureBuilder`

Builds a `TextureAsset` from a `TextureFile`.

Responsibilities:

* decode indexed pixels
* decode final canvas pixels
* assemble reusable texture asset data

---

### `TextureAsset`

Reusable decoded texture representation.

Contains:

* metadata
* palette
* decoded pixels

`TextureAsset` is data.

It is not a renderer.

---

### `TexturePixelDecoder`

Decodes texture pixel indices.

---

### `TextureCanvasDecoder`

Builds the final texture canvas from texture metadata and decoded pixel indices.

---

## Texture Archive

Textures are currently loaded from:

```text
CacheIndex::Config
archive id 6
```

Current loader constant:

```text
TextureArchiveId = 6
```

The loader expects:

```text
archive files
    texture files by index
    final file as shared index metadata
```

The requested texture id maps to an archive file index.

The final archive file is treated as shared index data.

---

## Texture File Reading

`TextureFileReader::read()` performs raw texture file parsing.

Flow:

```text
validate texture data
    ↓
read index offset from first 2 bytes of texture data
    ↓
read palette count from index data
    ↓
read palette
    ↓
read metadata
    ↓
read indexed pixels
    ↓
return TextureFile
```

Metadata includes:

```text
canvasWidth
canvasHeight
xOffset
yOffset
width
height
type
```

Palette colors are read from index data.

Indexed pixels are read from texture data after the first 2 bytes.

---

## Texture Asset Building

`TextureBuilder::build()` turns `TextureFile` into `TextureAsset`.

Flow:

```text
TextureFile
    ↓
TexturePixelDecoder
    ↓
indexed pixels
    ↓
TextureCanvasDecoder
    ↓
decoded canvas pixels
    ↓
TextureAsset
```

The builder coordinates decoding.

It should not read cache files.

It should not perform rendering.

---

## Ownership

The texture loading system owns:

* texture archive lookup
* texture file lookup
* texture index metadata reading
* palette reading
* metadata reading
* indexed pixel reading
* texture pixel decoding
* texture canvas decoding
* texture asset construction
* texture asset caching

The texture loading system does not own:

* render-time texture sampling
* model loading
* cache sector reading
* ElForge UI
* viewport behavior
* gameplay meaning
* world placement

---

## Extension Points

Add texture-loading behavior here when the behavior is about:

* texture file structure
* texture metadata
* palette decoding
* pixel decoding
* canvas construction
* texture asset fields
* texture asset caching

Examples:

Good:

```text
support another texture layout type
fix palette decoding
add alpha handling to TextureAsset
improve canvas placement behavior
```

Bad:

```text
sample textures during rasterization
draw textured triangles
load model faces
add texture browser UI
```

---

## Common Mistakes

Do not:

* decode textures in ElForge
* decode textures in render
* sample render materials in `data/texture/`
* duplicate `TextureLoader`
* bypass `TextureLoader` for normal texture loading
* put model-specific logic in `data/texture/`
* put UI or viewport behavior in `data/texture/`

---

## When Adding New Code

Before changing the texture loading system:

1. Determine whether the change is about texture data or render-time sampling.
2. Keep texture data behavior in `data/texture/`.
3. Keep render-time sampling in `render/material/`.
4. Extend `TextureLoader`, `TextureFileReader`, `TextureBuilder`, or the relevant decoder instead of creating a parallel path.
5. Preserve the reader / decoder / builder / asset / loader separation.

---

## Verification

Useful verification steps:

```bash
cmake --build build
```

Manual checks:

```text
Open ElForge.
Load a textured model.
Verify textures still appear correctly.
Verify repeated texture loads use the loader path correctly.
```

If archive behavior changes, also verify:

```text
Texture archive 6 loads from config index.
Texture index metadata is found.
Multiple texture ids can be loaded.
```

---

## Golden Rule

The texture loading system turns cache texture bytes into `TextureAsset`.

It does not render textures or decide how textures are sampled during drawing.
