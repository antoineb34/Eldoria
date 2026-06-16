# Phase 4 - Data Expansion

## Purpose

Phase 4 expands the shared `data/` module so Eldoria can describe real RuneScape-style world content.

The goal is not gameplay.

The goal is not client presentation.

The goal is to decode and represent enough static cache data for `world/`, ElForge, ElClient, and ElServer to build on.

Phase 4 exists because meaningful world representation depends on data that does not exist yet.

---

## Outcome

Phase 4 is complete when `data/` can provide the foundational static content required to build a local world region.

Required outcome:

* config archive foundation
* object definition loading
* map archive discovery
* terrain decoding
* location/object placement decoding
* clear ownership boundaries between data and world
* documentation explaining the new data domains

Optional or later:

* item definitions
* NPC definitions
* sprites
* interface definitions
* animations
* music
* sound

---

## Core Rule

`data/` owns static facts.

`world/` owns runtime spatial representation.

ElClient displays world state.

ElForge inspects and edits content through shared systems.

ElServer owns authoritative online state.

```text
cache bytes
    ↓
data definitions/assets
    ↓
world representation
    ↓
render/client presentation
```

Do not put map decoding, object definitions, item definitions, NPC definitions, or sprite decoding inside ElClient, ElForge, or ElServer.

---

# 4.1 Config Archive Foundation

## Purpose

Create shared support for reading config archives from the cache.

Many definition systems depend on config archive access.

## Required Implementation

* config archive loading path
* named archive/file lookup where required
* reusable config bytes access
* error handling for missing archives/files
* simple diagnostic output or probe path

## Required Integration

* use `data/cache/` for archive access
* keep raw archive details inside `data/`
* do not expose app-specific behavior

## Not Included

Not required:

* full definition parsing
* gameplay behavior
* editor UI
* client UI

## Exit Criteria

* code can load config archive data through `data/`
* missing data fails clearly
* future definition loaders have an obvious place to attach

---

# 4.2 Object Definitions

## Purpose

Decode object definitions so maps can resolve placed object ids into reusable static object data.

Object definitions describe what an object type is.

They do not describe where an object is placed.

## Required Implementation

* object definition data structure
* object definition file reader/decoder
* object definition builder if needed
* object definition loader
* fields required for early world rendering

Early required fields should include, where available:

* object id
* name
* model ids
* size in tiles
* interaction/action strings
* blocking/solid flags if known
* render-related flags required to place/display the object

## Required Integration

* object definitions live in `data/object/`
* placed object instances belong in `world/`
* object interaction rules belong in `game/`
* object rendering conversion belongs in `render/` or app-specific scene construction

## Not Included

Not required:

* object interaction gameplay
* object editing UI
* server authority
* collision runtime behavior beyond decoded static flags

## Exit Criteria

* object definitions can be loaded by id
* object definitions expose model ids and size information
* invalid/missing definitions fail clearly
* ElForge or probes can inspect loaded definitions later

---

# 4.3 Map Archive Discovery

## Purpose

Identify how map/landscape/location archives are discovered and loaded from the cache.

This is a discovery-heavy step and should stay focused.

## Required Implementation

* map archive lookup mechanism
* region/archive id discovery
* terrain file access
* location/object placement file access
* clear diagnostics for missing files

## Required Integration

* cache access remains in `data/cache/`
* map-specific interpretation belongs in `data/map/`
* world does not load raw cache archives directly

## Not Included

Not required:

* full terrain rendering
* full world representation
* client visualization
* collision/pathfinding

## Exit Criteria

* a known region's map-related files can be located
* raw map/location bytes can be loaded through `data/map/`
* diagnostics explain missing or unresolved archives

---

# 4.4 Terrain Decoding

## Purpose

Decode static terrain/tile data for a map region.

Terrain data describes the static map surface.

## Required Implementation

* terrain file reader
* tile height data where available
* overlay/underlay ids where available
* render-relevant tile attributes where available
* clean terrain data structure
* terrain loader path

## Required Integration

* decoded static terrain data lives in `data/map/`
* runtime region/tile state belongs in `world/`
* render mesh construction belongs outside `data/`

## Not Included

Not required:

* final terrain renderer
* collision runtime system
* minimap
* server simulation

## Exit Criteria

* a known region's terrain data can be decoded
* decoded tiles are represented in clean data structures
* malformed/missing terrain data fails clearly

---

# 4.5 Location/Object Placement Decoding

## Purpose

Decode static placed object/location data for a map region.

Location data says which object ids are placed at which coordinates with orientation/type information.

## Required Implementation

* location file reader
* placed object/location decoder
* static placement data structure
* region-local coordinate handling
* orientation/type fields where available

## Required Integration

* decoded static placement data lives in `data/map/`
* runtime `WorldObject` instances belong in `world/`
* object definitions come from `data/object/`

## Not Included

Not required:

* object interaction gameplay
* final collision system
* rendering conversion
* editor UI

## Exit Criteria

* known region object placements can be decoded
* placement entries include object id, position, type, and orientation where available
* object placements can later be combined with object definitions

---

# 4.6 Data Documentation

## Purpose

Keep data architecture documentation accurate as new domains are promoted into production code.

## Required Documentation

Update:

```text
docs/modules/data.md
```

Potential future system docs:

```text
docs/systems/config-loading.md
docs/systems/object-definition-loading.md
docs/systems/map-loading.md
```

Only create system documents when implementation complexity justifies them.

## Exit Criteria

* implemented data domains are documented as current state
* future-only data domains are not described as implemented
* ownership boundaries remain clear

---

## Not Included In Phase 4

Phase 4 does not require:

* live world state
* world simulation
* client world rendering
* ElServer authority
* gameplay rules
* inventory system
* combat system
* pathfinding
* production editing tools
* audio/music support

Those belong to later phases.

---

## Phase 4 Completion Criteria

Phase 4 is complete when:

* config archive access exists
* object definitions can be loaded
* map archives can be discovered
* terrain data can be decoded
* location/object placement data can be decoded
* decoded data is reusable by ElForge, ElClient, ElServer, and `world/`
* documentation matches implementation

---

## Golden Rule

Phase 4 makes the cache describe reality.

It does not simulate reality.

It does not display reality.
