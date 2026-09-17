# Eldoria Asset-to-Rendering Architecture

This document defines the intended path from source/cache assets to generic
runtime rendering resources.

The texture pipeline is the first complete implementation of this pattern.
Models and other shared rendering resources should generally follow the same
design when their ownership requirements justify it.

---

## 1. Layers

```text
assets/
    source-specific data
        |
        v
graphics/
    source -> generic rendering translation
        |
        v
runtime/render/
    generic runtime rendering
```

Dependency direction:

```text
assets ------\
              > graphics
render ------/
```

`graphics` is the bridge.

`runtime/render` must not know about RuneScape cache formats or source IDs.

---

## 2. Asset Pipeline

The normal asset pipeline is:

```text
Cache
  |
  v
Decoder
  |
  v
Data
  |
  v
Loader
```

Example:

```text
Cache
  |
  v
ImageDecoder
  |
  v
ImageData
  |
  v
TextureLoader
```

### Data

`Data` is the decoded, source-specific representation.

Examples:

```text
ImageData
ModelData
FloorData
TerrainData
```

### Loader

A Loader:

- retrieves source data
- decodes it when required
- caches decoded Data
- returns source-specific Data

A Loader does not create rendering resources.

A Loader does not return render handles.

A Loader does not know about OpenGL.

---

## 3. Graphics Layer

`src/graphics/` is the translation boundary.

It is intentionally allowed to know both:

```text
assets
runtime/render
```

Example:

```text
ImageData
   |
   v
TextureBuilder
   |
   v
TextureResource
```

If replacing RuneScape with a completely different source format would require
changing something, that thing generally belongs in `assets` or `graphics`,
not `runtime/render`.

---

## 4. Builders

A Builder converts representations.

Example:

```text
TextureBuilder

ImageData
    |
    v
TextureBuilder::build()
    |
    v
TextureResource
```

Builders:

- know the source representation
- know the destination representation
- perform conversion
- do not own runtime resource lifetime
- do not normally return handles

A Builder answers:

> "How do I turn this source data into generic render data?"

---

## 5. Runtime Resources

Generic rendering resources live in:

```text
src/runtime/render/
```

Examples:

```text
TextureResource
TextureHandle
TextureManager
TextureSampler

RenderModel
RenderMesh
RenderMaterial
RenderObject
```

These types should make sense even if Eldoria stopped using RuneScape assets.

---

## 6. Resource Ownership

Runtime Managers own runtime Resources.

Example:

```text
TextureManager
    owns
TextureResource
```

Other systems should not store raw pointers into asset Loader caches.

Instead they use Handles.

---

## 7. Handles

A Handle is a lightweight identity for a Manager-owned Resource.

Example:

```cpp
struct TextureHandle
{
    std::uint32_t index = 0;
    std::uint32_t generation = 0;

    bool operator==(const TextureHandle&) const = default;
};
```

A Handle:

- owns nothing
- does not keep the Resource alive
- does not contain pixel/model data
- identifies a Resource owned by a Manager

Conceptually:

```text
TextureHandle
     |
     v
TextureManager
     |
     v
TextureResource
```

The generation protects against stale handles after a slot is destroyed and
reused.

```text
slot 3, generation 1
        |
        v
Handle {3,1}

destroy resource

slot 3, generation 2
        |
        v

Handle {3,1} is now invalid
```

This pattern may later be reused for resources such as:

```text
ModelHandle
AnimationHandle
```

but only when shared ownership/lifetime actually requires it.

---

## 8. Managers

A Manager owns generic runtime resources.

Example API:

```cpp
TextureHandle create(TextureResource resource);

const TextureResource& get(
    TextureHandle handle) const;

bool isValid(
    TextureHandle handle) const;

void destroy(
    TextureHandle handle);
```

Responsibilities:

```text
Resource creation
Resource ownership
Resource lookup
Handle validation
Resource destruction
Lifetime management
```

A generic Manager must not know source IDs.

Bad:

```cpp
textureManager.getRuneScapeTexture(17);
```

Good:

```cpp
textureSystem.get(17);
```

---

## 9. Graphics Systems

A graphics System coordinates a complete source-to-runtime workflow.

Current example:

```text
graphics::TextureSystem
```

It connects:

```text
TextureLoader
TextureBuilder
TextureManager
```

and remembers:

```text
source texture ID -> TextureHandle
```

Workflow:

```text
TextureSystem::get(17)
        |
        +-- cached valid handle?
        |       |
        |       +-- yes -> return it
        |
        +-- no
             |
             v
       TextureLoader.get(17)
             |
             v
          ImageData
             |
             v
       TextureBuilder.build()
             |
             v
       TextureResource
             |
             v
       TextureManager.create()
             |
             v
        TextureHandle
             |
             v
       cache 17 -> handle
             |
             v
           return
```

Ownership:

```text
TextureSystem
|-- references TextureLoader
|-- references TextureManager
|-- owns TextureBuilder
`-- owns source-ID -> TextureHandle mapping
```

The System coordinates the workflow.

It does not need to own the lifetime of every dependency it uses.

---

## 10. Complete Texture Pipeline

```text
SOURCE
RuneScape texture ID
        |
        v
TextureLoader
        |
        v
ImageData

GRAPHICS
        |
        v
TextureBuilder
        |
        v
TextureResource
        |
        v
TextureSystem coordinates creation

RUNTIME
        |
        v
TextureManager
        |
        v
TextureHandle
```

For terrain:

```text
FloorData.textureId
        |
        v
TerrainBuilder
        |
        v
TextureSystem.get(id)
        |
        v
TextureHandle
        |
        v
RenderMaterial.texture
```

For rendering:

```text
RenderMaterial
        |
        v
TextureHandle
        |
        v
SimpleRenderer
        |
        v
TextureManager.get(handle)
        |
        v
TextureResource
        |
        v
OpenGL texture
```

The renderer never needs `ImageData`.

The material never points into `TextureLoader` storage.

The terrain builder never owns texture pixels.

---

## 11. RenderMaterial

A RenderMaterial describes how geometry should be rendered.

Conceptually:

```cpp
struct RenderMaterial
{
    Vec4 baseColor;

    std::optional<TextureHandle> texture;

    TextureSampler sampler;

    AlphaMode alphaMode;
    bool doubleSided;
};
```

Separation:

```text
TextureHandle
    which texture

TextureSampler
    how the texture is sampled
```

The same texture Resource may therefore be used with different sampler state.

---

## 12. TextureResource

`TextureResource` contains generic runtime texture data.

Example:

```cpp
struct TextureResource
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    std::vector<std::uint8_t> rgba;
};
```

It should not contain:

```text
ImageData
RuneScape texture IDs
Cache references
TextureLoader references
OpenGL GLuint objects
```

---

## 13. RenderModel

A RenderModel describes reusable renderable geometry.

```text
RenderModel
|-- RenderMesh
|   |-- vertices
|   |-- indices
|   `-- sections
|
`-- RenderMaterial
    |-- baseColor
    |-- TextureHandle
    |-- TextureSampler
    `-- render state
```

A RenderModel is generic runtime rendering data.

It should not know how its original source model was stored.

---

## 14. RenderObject

Conceptually:

```text
RenderModel
    what can be drawn

RenderObject
    an instance of something being drawn
```

Example:

```text
one tree RenderModel

RenderObject A
    tree
    transform at location A

RenderObject B
    tree
    transform at location B
```

The current implementation may still store a RenderModel directly.

If RenderModels later become shared runtime resources, the intended direction
is:

```text
ModelManager
    owns RenderModel

ModelHandle
    identifies RenderModel

RenderObject
|-- ModelHandle
`-- Transform
```

Do not introduce this purely for symmetry. Introduce it when model sharing and
lifetime management require it.

---

## 15. Likely Model Pipeline

The likely long-term model equivalent is:

```text
ASSETS
ModelLoader
    |
    v
ModelData

GRAPHICS
ModelBuilder
    |
    v
RenderModel

RUNTIME
ModelManager
    |
    v
ModelHandle
```

Then:

```text
RenderObject
|-- ModelHandle
`-- Transform
```

This is an intended direction, not something that must be implemented
immediately.

---

## 16. Client / Application Boundary

The client is the composition root.

It creates long-lived Managers, Systems, Loaders, renderers, etc. and connects
their dependencies.

The client should not manually execute every stage of a resource pipeline.

For example, the client may wire textures like this:

```cpp
eld::asset::AssetManager assets;

eld::render::TextureManager textureManager;

eld::graphics::TextureSystem textureSystem(
    assets.textures,
    textureManager
);
```

But it should use higher-level graphics operations:

```cpp
const auto terrainModel =
    terrainBuilder.build(
        terrain,
        assets.floors,
        textureSystem
    );
```

The TerrainBuilder internally asks TextureSystem for textures.

The client should NOT manually do this for each texture:

```text
TextureLoader
    |
    v
TextureBuilder
    |
    v
TextureManager
```

The rule is:

> The application wires the architecture together, but calls the highest
> useful graphics-level API.

The source-to-resource plumbing stays behind Systems and Builders.

Rendering then remains similarly simple:

```cpp
renderer.uploadModel(terrainModel);
```

The renderer resolves TextureHandles through TextureManager.

---

## 17. Ownership Summary

```text
Asset Loader
    owns/caches source Data

Builder
    converts representations
    owns no runtime resource lifetime

Graphics System
    coordinates source -> runtime workflow
    may own source-ID -> Handle mappings

Runtime Manager
    owns Resources

Handle
    owns nothing
    identifies a Manager-owned Resource

RenderMaterial
    owns rendering settings and Handles
    does not own texture pixels

RenderModel
    owns reusable generic mesh/material data

RenderObject
    represents an instance + Transform
```

---

## 18. Stable Dependency Rules

### Rule 1

Assets must not depend on rendering.

Bad:

```text
TextureLoader -> TextureHandle
```

### Rule 2

Runtime rendering must not depend on source asset representations.

Bad:

```text
RenderMaterial -> ImageData*
```

Good:

```text
RenderMaterial -> TextureHandle
```

### Rule 3

Graphics may depend on both assets and render.

```text
ImageData
    |
    v
TextureBuilder
    |
    v
TextureResource
```

### Rule 4

Managers own Resources.

Builders do not manage runtime lifetime.

### Rule 5

Source IDs do not belong in generic Managers.

```text
TextureSystem knows RuneScape ID 17
TextureManager does not
```

### Rule 6

Handles contain generic runtime identity, not source IDs.

### Rule 7

The client wires dependencies but should call high-level APIs rather than
manually reproducing pipeline internals.

---

## 19. Naming

Use these meanings consistently:

```text
Data
    decoded/source-specific representation

Loader
    retrieves and caches source Data

Builder
    converts representations

Resource
    generic runtime-owned data

Manager
    owns Resources and controls lifetime

Handle
    identity for a Manager-owned Resource

System
    coordinates a larger workflow
```

Avoid introducing concepts such as:

```text
Resolver
Registry
Catalog
Provider
```

unless they have a genuinely distinct responsibility not already represented
by these concepts.

---

## 20. Core Pattern

The architecture can be summarized as:

```text
SOURCE DATA
     |
     v
  BUILDER
     |
     v
GENERIC RESOURCE
     |
     v
  MANAGER
     |
     v
   HANDLE
```

When source IDs need to be mapped into that pipeline:

```text
source ID
    |
    v
SYSTEM
 |-- Loader
 |-- Builder
 `-- Manager
    |
    v
 Handle
```

And application code should generally see:

```text
Client
   |
   v
high-level graphics operation
   |
   v
RenderModel / Handle
   |
   v
Renderer
```

That is the pattern to preserve as Eldoria's rendering architecture grows.
