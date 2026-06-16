# Data Module

## Purpose

`src/data/` owns static game data and data-loading systems.

It answers:

```text
What are things, and how are they loaded?
```

The data module is responsible for reading, decoding, and building reusable asset/data representations from RuneScape-style cache data and future Eldoria content sources.

Data is the lowest major content layer.
World, game, render, net, and apps consume data.
Data should not depend on those higher layers.

---

## Current State

`src/data/` currently contains implemented systems for:

* binary reading
* compression
* cache access
* archive reading
* model loading
* texture loading

Current production structure:

```text
src/data/
├── binary/
├── cache/
├── model/
└── texture/
```

Map loading, config loading, definition loading, sprite loading, animation loading, and interface loading are future data expansion areas.

Do not document those systems as implemented until production code exists in `src/data/` and is included in the `Eldoria::Data` target.

---

## Direction

The next major direction for `data/` is expanding beyond models and textures so Eldoria can describe real RuneScape-style world content.

Priority data expansion areas:

```text
src/data/config/
src/data/object/
src/data/item/
src/data/npc/
src/data/map/
src/data/sprite/
src/data/interface/
src/data/animation/
```

Recommended order:

1. Config archive foundation.
2. Object definitions.
3. Map archive discovery.
4. Terrain decoding.
5. Location/object placement decoding.
6. Item definitions.
7. NPC definitions.
8. Sprite/interface assets.
9. Animation data.

Music and sound data can wait until they are needed by client presentation.

If a new feature is about reading, decoding, representing, or loading static content, it probably belongs in `data/`.

Examples:

```text
object definition parsing
= data/object/

map terrain decoding
= data/map/

item definition parsing
= data/item/

NPC definition parsing
= data/npc/

sprite decoding
= data/sprite/

interface definition parsing
= data/interface/

animation loading
= data/animation/
```

Do not create these systems inside ElForge, ElClient, or ElServer unless the code is truly application-specific.

Applications consume data systems.

They should not own them.

---

## Relationship To World, Client, And Tools

Data describes static facts.

Examples:

```text
data/object/ObjectDefinition
= what an object type is

data/map/MapRegionData
= static terrain and placement data decoded from cache

data/model/ModelAsset
= static model geometry and material data
```

World uses data to build spatial reality.

Examples:

```text
world/object/WorldObject
= an object instance placed at a coordinate

world/region/Region
= runtime spatial representation built from decoded map data
```

ElClient displays world/data state.

ElForge inspects, validates, and eventually edits content through shared systems.

ElServer owns authoritative runtime state and gameplay authority.

---

## Standard Pattern

New data domains should follow the existing architecture style used by model and texture systems.

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

Keep raw format knowledge isolated from clean reusable data structures.

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
* config archive data, when added
* map data, when added
* object definitions, when added
* item definitions, when added
* NPC definitions, when added
* sprite data, when added
* interface data, when added
* animation data, when added

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

Should not know about ElForge viewport controls or ElClient screens.

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

### `data/config/`

Future home for decoded config archives and shared config archive access.

This should provide the foundation used by object, item, NPC, interface, and other definition loaders.

---

### `data/object/`

Future home for object definition loading.

Object definitions describe object types.

Placed object instances belong in `world/`.

---

### `data/item/`

Future home for item definition loading.

Item definitions describe item types.

Inventory state and equipment rules belong in `game/` and application/server state.

---

### `data/npc/`

Future home for NPC definition loading.

NPC definitions describe NPC types.

NPC runtime state belongs in `world/` and authoritative behavior belongs in ElServer/game systems.

---

### `data/map/`

Future home for map file loading and decoding.

Examples:

* terrain decoding
* location/object placement decoding
* map archive discovery
* map file reading
* static region data
* static tile data

Decoded map data describes static map content.

Live world state does not belong here.

---

### `data/sprite/`

Future home for sprite loading and decoding.

Sprites are static visual assets.

Runtime interface behavior belongs in ElClient.

---

### `data/interface/`

Future home for interface definition loading.

Interface definitions describe static interface layout/content.

Runtime widget behavior and input routing belong in ElClient.

---

### `data/animation/`

Future home for animation data loading.

Animation data describes available transformations/sequences.

Runtime animation state belongs in world/client/game presentation layers depending on context.

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

apps/elclient/screens/GameScreen
= player-facing screen that displays the world through render output
```

---

## Common Mistakes

Do not:

* decode models inside ElForge
* decode textures inside ElForge
* decode maps inside ElForge
* decode maps inside ElClient
* place animation parsing inside ElClient
* place item/NPC/object definition parsing inside ElServer
* put rendering behavior in `data/`
* put gameplay rules in `data/`
* put live world state in `data/`
* make `data/` depend on applications
* create parallel loaders when an existing loader can be extended
* document a data domain as implemented before production code exists

---

## When Adding New Data Code

Before adding code to `src/data/`:

1. Identify the data domain.
2. Check whether a matching submodule already exists.
3. Follow the existing reader / decoder / builder / asset-or-definition / loader pattern.
4. Keep the result reusable by ElForge, ElClient, and ElServer.
5. Keep application-specific behavior outside `data/`.
6. Update this document if a new production data domain is added.

If a new folder is needed, it should represent a real static data domain.

Good examples:

```text
src/data/config/
src/data/object/
src/data/item/
src/data/npc/
src/data/map/
src/data/sprite/
src/data/interface/
src/data/animation/
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
