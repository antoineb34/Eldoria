# Data Module

## Purpose

`src/data/` owns static game data and data-loading systems.

It answers:

```text
What are things, and how are they loaded?
```

The data module is responsible for reading, decoding, and building reusable asset/data representations from RuneScape-style cache data and future Eldoria content sources.

---

## Current State

`src/data/` currently contains implemented systems for:

* binary reading
* compression
* cache access
* archive reading
* model loading
* texture loading
* map loading
* RuneScape-317 reference code

Current structure:

```text
src/data/
├── binary/
├── cache/
├── map/
├── model/
├── texture/
└── references_rs317/
```

---

## Direction

Future static data systems should also live under `src/data/`.

Examples:

```text
src/data/animation/
src/data/interface/
src/data/item/
src/data/npc/
src/data/object/
src/data/sprite/
src/data/definition/
```

If a new feature is about reading, decoding, representing, or loading static content, it probably belongs in `data/`.

Examples:

```text
animation loading
= data/animation/

item definition parsing
= data/item/

NPC definition parsing
= data/npc/

object definition parsing
= data/object/

interface definition parsing
= data/interface/

sprite decoding
= data/sprite/
```

Do not create these systems inside ElForge, ElClient, or ElServer unless the code is truly application-specific.

Applications consume data systems.

They should not own them.

---

## Standard Pattern

New data domains should follow the existing architecture style used by model, texture, and map systems.

Preferred shape:

```text
Raw bytes
    ↓
FileReader
    ↓
Decoder
    ↓
Builder
    ↓
Asset / Definition
    ↓
Loader
```

Example model-style pattern:

```text
ModelFileReader
    reads raw model file structure

VertexDecoder / FaceDecoder
    decode encoded model sections

ModelBuilder
    builds clean runtime asset data

ModelAsset
    reusable model representation

ModelLoader
    loads model data from cache/content source
```

Example texture-style pattern:

```text
TextureFileReader
    reads raw texture file structure

TextureCanvasDecoder / TexturePixelDecoder
    decode texture data

TextureBuilder
    builds clean texture asset data

TextureAsset
    reusable texture representation

TextureLoader
    loads texture data from cache/content source
```

When adding a new data domain, follow the existing reader / decoder / builder / asset-or-definition / loader separation unless there is a clear reason not to.

---

## Owns

`data/` owns:

* binary reading helpers
* compression helpers
* cache file access
* archive reading
* static asset file reading
* static asset decoding
* static asset building
* static asset loading
* model data
* texture data
* map data
* animation data, when added
* interface data, when added
* sprite data, when added
* item definitions, when added
* NPC definitions, when added
* object definitions, when added
* RuneScape-317 reference parsing code

---

## Does Not Own

`data/` does not own:

* rendering behavior
* render pipelines
* texture sampling for rendering
* live world state
* gameplay rules
* networking behavior
* app-specific UI
* ElForge tool workflows
* ElClient runtime behavior
* ElServer authority
* player/session state
* editing workflows
* save/export UI behavior

---

## Used By

`data/` may be used by:

* ElForge
* ElClient
* ElServer
* world
* game
* render, when renderable asset data is required

---

## Dependency Rules

`data/` should stay low-level and reusable.

`data/` should not depend on:

* `apps/`
* `world/`
* `game/`
* `net/`
* `render/`

`data/` may depend on:

* the C++ standard library
* approved compression libraries
* local binary helpers inside `src/data/binary/`
* local cache helpers inside `src/data/cache/`

Other modules may depend on `data/`.

---

## Submodule Responsibilities

### `data/binary/`

Owns binary utilities used by data formats.

Examples:

* byte buffers
* primitive reads
* compression helpers

Should not know about applications, rendering, gameplay, or networking.

---

### `data/cache/`

Owns RuneScape-style cache access.

Examples:

* cache index files
* data file sectors
* archive files
* archive readers
* cache file types

Cache reads bytes.

Other data systems interpret those bytes.

---

### `data/model/`

Owns model data loading and decoding.

Examples:

* model file reading
* vertex decoding
* face decoding
* texture UV mapping decoding
* model asset building
* model loading

Should not render models.

Should not know about ElForge viewport controls.

---

### `data/texture/`

Owns texture data loading and decoding.

Examples:

* texture file reading
* texture canvas decoding
* texture pixel decoding
* texture asset building
* texture loading

Static texture asset data belongs here.

Rendering-specific texture sampling belongs in `render/`.

---

### `data/map/`

Owns map file loading and decoding.

Examples:

* terrain decoding
* object spawn decoding
* map file reading
* map region data
* map tile data

Decoded map data describes static map content.

Live world state does not belong here.

---

### `data/references_rs317/`

Owns reference code used to understand RuneScape-317 data formats.

This code may be less clean than production systems.

Treat it as:

* reference material
* research support
* migration source

Do not build new application behavior directly on top of reference code unless the issue explicitly allows it.

When useful behavior is understood, promote it into the proper production data submodule.

---

## Boundary Examples

```text
data/item/ItemDefinition
= what an item type is

world/object/WorldObject
= an object placed at a coordinate

game/object/ObjectInteraction
= what happens when a player interacts with the object

render/scene/RenderObject
= visual object submitted for drawing

apps/elforge/panels
= UI for inspecting or editing the object
```

```text
data/model/ModelAsset
= static model data

render/scene/RenderObject
= model instance submitted to renderer

apps/elforge/viewport
= tool viewport that displays the rendered model
```

---

## Common Mistakes

Do not:

* decode models inside ElForge
* decode textures inside ElForge
* decode maps inside ElForge
* place animation parsing inside ElClient
* place item/NPC/object definition parsing inside ElServer
* put rendering behavior in `data/`
* put gameplay rules in `data/`
* put live world state in `data/`
* make `data/` depend on applications
* create parallel loaders when an existing loader can be extended
* use `references_rs317/` as permanent application architecture

---

## When Adding New Data Code

Before adding code to `src/data/`:

1. Identify the data domain.
2. Check whether a matching submodule already exists.
3. Follow the existing reader / decoder / builder / asset-or-definition / loader pattern.
4. Keep the result reusable by ElForge, ElClient, and ElServer.
5. Keep application-specific behavior outside `data/`.

If a new folder is needed, it should represent a real static data domain.

Good examples:

```text
src/data/animation/
src/data/interface/
src/data/item/
src/data/npc/
src/data/object/
src/data/sprite/
```

Bad examples:

```text
src/data/elforge_model_viewer/
src/data/client_inventory_ui/
src/data/server_player_state/
```

---

## Golden Rule

`data/` owns static content and the process of loading it.

It does not own what applications do with that content.
