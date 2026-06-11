# Model Loading System

## Purpose

The model loading system reads RuneScape model files from the cache and turns them into reusable `ModelAsset` data.

It answers:

```text
How does a cached model id become usable model asset data?
```

The model loading system belongs to:

```text
src/data/model/
```

---

## Current State

The model loading system currently supports:

* loading model files from the cache
* GZip decompression when required
* model footer reading
* model layout calculation
* vertex decoding
* face decoding
* texture UV mapping decoding
* model asset construction
* model asset caching by id
* texture lookup for textured model faces

Current files:

```text
src/data/model/
├── decoder/
│   ├── FaceDecoder.cpp
│   ├── FaceDecoder.h
│   ├── TextureUVMappingDecoder.cpp
│   ├── TextureUVMappingDecoder.h
│   ├── VertexDecoder.cpp
│   └── VertexDecoder.h
├── ModelAsset.h
├── ModelBuilder.cpp
├── ModelBuilder.h
├── ModelFile.h
├── ModelFileReader.cpp
├── ModelFileReader.h
├── ModelLoader.cpp
└── ModelLoader.h
```

---

## Data Flow

```text
model id
    ↓
ModelLoader
    ↓
Cache::readFile(CacheIndex::Model, id)
    ↓
raw cache payload
    ↓
GZip decompression if needed
    ↓
ModelFileReader
    ↓
ModelFile
    ↓
ModelBuilder
    ↓
VertexDecoder
FaceDecoder
TextureUVMappingDecoder
    ↓
ModelAsset
    ↓
TextureLoader for textured faces
    ↓
cached ModelAsset
```

---

## Important Types

### `ModelLoader`

Main public entry point for loading models.

Public API:

```text
load(id)
```

Responsibilities:

* read model file payload from `Cache`
* decompress model payload when needed
* ask `ModelFileReader` to parse file layout
* ask `ModelBuilder` to build `ModelAsset`
* load referenced textures for textured faces
* cache loaded models by id

`ModelLoader` depends on:

```text
Cache
TextureLoader
ModelFileReader
ModelBuilder
```

---

### `ModelFileReader`

Reads raw model file structure.

Responsibilities:

* validate payload size
* read the model footer
* calculate internal section offsets
* return `ModelFile`

It does not decode vertices or faces directly.

---

### `ModelFile`

Represents the raw parsed model file container.

Contains:

* original payload
* footer
* calculated layout

The decoders use this structure to decode individual sections.

---

### `ModelBuilder`

Builds a `ModelAsset` from a `ModelFile`.

Responsibilities:

* create decoders
* decode vertices
* decode faces
* decode texture UV mappings
* assemble final `ModelAsset`

---

### `ModelAsset`

Reusable model representation.

Contains:

```text
vertices
faces
textureUVMappings
textures
```

`ModelAsset` is data.

It is not a renderer.

---

### `VertexDecoder`

Decodes model vertices.

Owns vertex delta decoding and vertex position reconstruction.

---

### `FaceDecoder`

Decodes model faces.

Owns face indices, colors, priorities, alpha, render type, triangle type, texture pointer, and texture UV mapping index.

---

### `TextureUVMappingDecoder`

Decodes model texture UV mapping triangle data.

---

## Model File Reading

`ModelFileReader::read()` performs the first structural parse.

Flow:

```text
validate payload
    ↓
read footer from final 18 bytes
    ↓
calculate section layout
    ↓
return ModelFile
```

The footer includes:

```text
vertexCount
triangleCount
textureTriangleCount
textureFlag
priorityFlag
alphaFlag
triangleSkinFlag
vertexSkinFlag
xDataLength
yDataLength
zDataLength
triangleDataLength
```

The layout calculates offsets for:

```text
vertex flags
triangle types
triangle priorities
triangle skins
texture pointers
vertex skins
triangle alphas
triangle data
triangle colors
texture data
x data
y data
z data
```

---

## Model Asset Building

`ModelBuilder::build()` creates specialized decoders and returns a `ModelAsset`.

Flow:

```text
ModelFile
    ↓
VertexDecoder::decode()
FaceDecoder::decode()
TextureUVMappingDecoder::decode()
    ↓
ModelAsset
```

The builder coordinates decoding.

It should not read cache files.

It should not render models.

---

## Texture Loading Integration

`ModelLoader` loads textures after building the model asset.

Current behavior:

* iterate model faces
* detect textured render types
* validate texture UV mapping index
* treat face color as texture id
* ask `TextureLoader` to load the texture
* store loaded texture in `ModelAsset::textures`

Texture loading remains owned by:

```text
src/data/texture/
```

The model loader only coordinates texture references needed by model assets.

---

## Ownership

The model loading system owns:

* model cache lookup by id
* model payload decompression
* model file footer reading
* model layout calculation
* vertex decoding
* face decoding
* texture UV mapping decoding
* model asset construction
* model asset caching

The model loading system does not own:

* rendering
* camera behavior
* viewport controls
* ElForge UI
* gameplay meaning
* world placement
* cache sector reading
* texture file decoding internals

---

## Extension Points

Add model-loading behavior here when the behavior is about:

* model file structure
* model decode correctness
* model asset fields
* model-specific cache loading
* model texture references
* model asset caching

Examples:

Good:

```text
support another model format variant
decode additional face metadata
improve texture UV mapping
add model bounds calculation to ModelAsset
```

Bad:

```text
draw model triangles
add viewport camera controls
load item definitions
perform object placement
```

---

## Common Mistakes

Do not:

* decode models in ElForge
* decode models in render
* read DAT/IDX sectors directly from model decoders
* duplicate `ModelLoader`
* create a second model asset type without a clear reason
* add rendering behavior to `ModelAsset`
* bypass `ModelLoader` for normal model loading
* put texture file decoding in `data/model/`

---

## When Adding New Code

Before changing the model loading system:

1. Determine whether the change is about model data, rendering, or UI.
2. Keep model data behavior in `data/model/`.
3. Keep rendering behavior in `render/`.
4. Keep viewport controls in the application.
5. Extend `ModelLoader`, `ModelFileReader`, `ModelBuilder`, or the relevant decoder instead of creating a parallel path.
6. Preserve the reader / decoder / builder / asset / loader separation.

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
Verify the model appears.
Verify textured models still load when texture behavior changes.
Verify repeated model loads use the loader path correctly.
```

Known model ids used during development should be documented in issue descriptions when relevant.

---

## Golden Rule

The model loading system turns cache model bytes into `ModelAsset`.

It does not render models or decide what models mean in gameplay.
