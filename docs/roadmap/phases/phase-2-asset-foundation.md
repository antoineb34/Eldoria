# Phase 2 - Asset Foundation

## Purpose

Phase 2 builds the foundational asset systems required by ElForge, ElClient, and ElServer.

The goal is to understand, load, decode, represent, and validate core RuneScape-style asset data.

This phase is not about gameplay.

This phase is not about content editing.

This phase is about creating reliable shared data systems that future applications can use.

---

## Outcome

Phase 2 is complete when Eldoria can reliably load and represent core asset data through reusable systems in `src/data/`.

Required outcomes:

* cache files can be read
* archives can be decoded
* models can be loaded
* textures can be loaded
* important asset formats are documented
* ElForge can inspect loaded assets
* render systems can visualize loaded assets where appropriate

Phase 2 turns raw cache data into usable Eldoria asset data.

---

## Core Rule

Unknown formats should be researched before implementation.

Expected workflow:

```text
Probe
    ↓
Document Format
    ↓
Design Data Types
    ↓
Implement Loader System
    ↓
Integrate With Apps
    ↓
Document System
```

Do not skip directly from guessing to implementation.

---

## Required Architecture Pattern

New asset systems should follow the existing pattern:

```text
Raw Bytes
    ↓
File Reader
    ↓
Decoder
    ↓
Builder
    ↓
Asset / Definition
    ↓
Loader
```

Examples already established:

```text
ModelLoader
TextureLoader
```

Future systems should follow the same style unless there is a clear reason not to.

---

# 2.1 Cache System

## Purpose

Read raw RuneScape-style cache files from disk.

## Required Discovery

* understand DAT/IDX layout
* understand index entries
* understand sector chains
* understand cache index mapping

## Required Documentation

* document cache file layout
* document sector format
* document archive relationship

## Required Implementation

* `src/data/cache/Cache`
* cache index types
* cache file types
* sector reading
* index entry reading
* file payload reconstruction

## Required Integration

* expose cache reads to higher-level loaders
* allow ElForge to browse cache contents

## Required System Documentation

```text
docs/systems/cache.md
```

## Exit Criteria

* cache directory can be validated
* cache files can be read by index/id
* cache payloads are returned as reusable byte vectors
* cache behavior is documented

---

# 2.2 Archive System

## Purpose

Decode archive payloads stored inside cache files.

## Required Discovery

* understand archive header
* understand archive metadata table
* understand archive compression behavior
* understand archive file lookup by hash/index

## Required Documentation

* document archive payload structure
* document compression behavior
* document archive lookup behavior

## Required Implementation

* `Archive`
* `ArchiveFile`
* `ArchiveReader`
* archive lookup helpers

## Required Integration

* support texture archive loading
* support future config/definition archive loading

## Required System Documentation

Archive behavior may be documented inside:

```text
docs/systems/cache.md
```

or split later if archive behavior grows large enough.

## Exit Criteria

* archive payloads can be decoded
* archive files can be extracted
* archive files can be found by hash or index
* archive behavior is documented

---

# 2.3 Model Loading System

## Purpose

Load and decode model data into reusable model assets.

## Required Discovery

* understand model footer
* understand model layout offsets
* understand vertex delta encoding
* understand face decoding
* understand texture UV mapping behavior

## Required Documentation

* document model file layout
* document known model assumptions
* document decoder responsibilities

## Required Implementation

* `src/data/model/ModelFile`
* `ModelFileReader`
* `VertexDecoder`
* `FaceDecoder`
* `TextureUVMappingDecoder`
* `ModelBuilder`
* `ModelAsset`
* `ModelLoader`

## Required Integration

* load model payloads from cache
* decompress model payloads when needed
* expose `ModelAsset` to ElForge
* expose `ModelAsset` to render systems

## Required ElForge Workflow

* select model from cache tree
* load model asset
* inspect model information
* display model in viewport

## Required System Documentation

```text
docs/systems/model-loading.md
```

## Exit Criteria

* known model ids load correctly
* vertices decode correctly
* faces decode correctly
* textured face metadata is represented
* ElForge can inspect and render models
* model loading behavior is documented

---

# 2.4 Texture Loading System

## Purpose

Load and decode texture data into reusable texture assets.

## Required Discovery

* understand texture archive location
* understand texture index data
* understand palette format
* understand indexed pixel layout
* understand canvas placement

## Required Documentation

* document texture archive behavior
* document texture file/index relationship
* document texture metadata
* document palette behavior

## Required Implementation

* `src/data/texture/TextureFile`
* `TextureFileReader`
* `TexturePixelDecoder`
* `TextureCanvasDecoder`
* `TextureBuilder`
* `TextureAsset`
* `TextureLoader`

## Required Integration

* load textures from config archive
* allow model loading to resolve referenced textures
* expose texture assets to ElForge
* expose texture assets to render systems

## Required ElForge Workflow

* select texture from cache tree or relevant asset list
* inspect texture information
* verify decoded texture output

## Required System Documentation

```text
docs/systems/texture-loading.md
```

## Exit Criteria

* texture archive loads correctly
* texture metadata is decoded
* palettes decode correctly
* texture pixels decode correctly
* textured models can resolve texture assets
* texture loading behavior is documented

---

# 2.5 Asset-Driven Rendering Foundation

## Purpose

Render loaded assets to validate that decoded data is usable.

Rendering belongs to `src/render/`.

Asset decoding belongs to `src/data/`.

This sub-phase exists because decoded assets need visual verification.

## Required Discovery

* understand how `ModelAsset` should become render input
* understand coordinate conversion needs
* understand camera/projection needs
* understand solid and textured triangle requirements

## Required Documentation

* document rendering pipeline
* document relationship between model assets and render objects
* document renderer/application boundaries

## Required Implementation

* `RenderObject`
* `RenderScene`
* `RenderPipeline`
* render backend interface
* software render backend
* projection
* face assembly
* visibility filtering
* depth sorting
* solid triangle rasterization
* textured triangle rasterization

## Required Integration

* ElForge viewport creates render scenes
* render pipeline consumes `ModelAsset`
* render backend displays viewport output

## Required ElForge Workflow

* select model
* create viewport render scene
* render selected model
* support basic debug controls

## Required System Documentation

```text
docs/systems/rendering.md
docs/systems/elforge-viewport.md
```

## Exit Criteria

* loaded models can be rendered
* textured faces can be rendered where supported
* viewport rendering works in ElForge
* rendering behavior is documented

---

# 2.6 Map Loading System

## Purpose

Load map and terrain data into reusable static map representations.

## Current Status

This system may exist only as a placeholder until real implementation begins.

Do not create a system document until the system has real implementation.

## Required Discovery

* probe map cache files
* identify map archive/file layout
* identify terrain format
* identify object spawn format
* compare findings against RS317 references

## Required Documentation

* document map file layout
* document terrain encoding
* document object placement encoding
* document unknowns and assumptions

## Required Implementation

Expected future location:

```text
src/data/map/
```

Expected pattern:

```text
MapFileReader
MapDecoder
MapBuilder
MapAsset / MapDefinition
MapLoader
```

## Required Integration

* expose map data to world systems
* expose map inspection to ElForge when useful

## Required ElForge Workflow

Future workflow:

* browse map files
* inspect terrain data
* inspect object placements
* eventually visualize map regions

## Required System Documentation

Create only when implementation exists:

```text
docs/systems/map-loading.md
```

## Exit Criteria

* map files can be loaded
* terrain data can be represented
* object placement data can be represented
* format is documented
* system behavior is documented

---

# 2.7 Definition Loading Systems

## Purpose

Load static gameplay/content definitions.

Examples:

* items
* NPCs
* objects
* interfaces
* sequences
* spot animations

Definitions describe what things are.

They do not execute gameplay behavior.

## Required Discovery

* identify relevant config archives/files
* inspect definition formats
* identify relationships between definitions and assets
* compare with RS317 references

## Required Documentation

* document each definition format as it is discovered
* document file/archive locations
* document field meanings
* document unknown fields

## Required Implementation

Expected future locations:

```text
src/data/item/
src/data/npc/
src/data/object/
src/data/interface/
src/data/animation/
```

Each definition domain should follow the same pattern:

```text
DefinitionFileReader
DefinitionDecoder
DefinitionBuilder
Definition
DefinitionLoader
```

## Required Integration

* expose definitions to ElForge
* expose definitions to game/world systems when needed
* avoid putting gameplay logic in data definitions

## Required ElForge Workflow

Future workflow:

* browse definitions
* inspect fields
* validate asset references
* eventually edit definitions

## Required System Documentation

Create system docs per implemented system.

Examples:

```text
docs/systems/item-definition-loading.md
docs/systems/npc-definition-loading.md
docs/systems/object-definition-loading.md
```

## Exit Criteria

A definition system is complete when:

* source format is documented
* definition files can be decoded
* reusable definition types exist
* ElForge can inspect definitions
* future systems can consume definitions

---

# 2.8 Animation Loading System

## Purpose

Load animation-related data into reusable animation assets or definitions.

This is expected to be a research-heavy system.

Do not guess the format.

Probe first.

## Required Discovery

* create probe app or diagnostic tooling
* inspect animation cache index/files
* identify skeleton/frame/sequence relationships
* identify file layouts
* identify compression behavior
* identify how animations relate to models
* compare against RS317 references

## Required Documentation

Before implementation, document:

* discovered animation file locations
* discovered file structure
* known fields
* unknown fields
* assumptions
* example ids
* relationship to models/rendering

## Required Implementation

Expected future location:

```text
src/data/animation/
```

Expected pattern:

```text
AnimationFileReader
AnimationDecoder
AnimationBuilder
AnimationAsset / AnimationDefinition
AnimationLoader
```

Implementation should follow model/texture loader architecture.

## Required Integration

Likely integration points:

* model assets
* render animation support
* ElForge animation inspection
* future gameplay animation selection

Renderer work may be required, but only after the data format is understood.

## Required ElForge Workflow

Future workflow:

* browse animation data
* inspect animation metadata
* inspect frame/skeleton data
* preview animation only after data and render support exist

## Required System Documentation

When implemented:

```text
docs/systems/animation-loading.md
```

Potential related updates:

```text
docs/modules/data.md
docs/modules/render.md
docs/systems/rendering.md
docs/apps/elforge.md
```

## Not Included

This sub-phase does not require:

* full animation editor
* gameplay animation state machine
* final animation blending
* production animation tooling

## Exit Criteria

Animation loading is complete when:

* animation cache structure has been probed
* format findings are documented
* animation data loads through `src/data/animation/`
* loader follows existing asset-loading architecture
* ElForge can inspect loaded animation data
* render requirements are documented or implemented

---

# 2.9 Sprite and Interface Asset Loading

## Purpose

Load sprite/interface-related static data required by future UI and tooling.

## Required Discovery

* identify sprite storage format
* identify interface config format
* inspect palette/image behavior if applicable
* compare against RS317 references

## Required Documentation

* document sprite format
* document interface format
* document asset references
* document unknowns

## Required Implementation

Expected future locations:

```text
src/data/sprite/
src/data/interface/
```

Expected pattern:

```text
SpriteLoader
InterfaceLoader
```

with supporting readers/decoders/builders as needed.

## Required Integration

* expose sprites/interfaces to ElForge
* later expose interfaces to ElClient

## Required ElForge Workflow

Future workflow:

* browse sprites
* inspect sprites
* browse interfaces
* inspect interface hierarchy

## Required System Documentation

Create when implemented:

```text
docs/systems/sprite-loading.md
docs/systems/interface-loading.md
```

## Exit Criteria

* sprites can be loaded
* interfaces can be decoded enough for inspection
* ElForge can inspect loaded data
* behavior is documented

---

## Not Included In Phase 2

Phase 2 does not require:

* gameplay systems
* server authority
* multiplayer
* final content editors
* full map editor
* full animation editor
* production tooling

Phase 2 may expose assets in ElForge for inspection.

Editing belongs to later content tooling phases.

---

## Issue Breakdown Strategy

Each asset system should be broken into focused issues.

Preferred order:

```text
1. Probe format
2. Document format findings
3. Create data types
4. Implement reader
5. Implement decoder
6. Implement builder
7. Implement loader
8. Integrate with ElForge
9. Add verification
10. Document system
```

Issues should not combine discovery, implementation, ElForge integration, and documentation unless the scope is extremely small.

Good issue:

```text
Probe animation cache files and document observed structure.
```

Bad issue:

```text
Implement animations.
```

Good issue:

```text
Add AnimationFileReader using documented animation format.
```

Bad issue:

```text
Add animation loading, rendering, and editor.
```

---

## Phase 2 Completion Criteria

Phase 2 is complete when the core asset foundation is stable enough that future work can build on it.

Required:

* cache system works
* archive system works
* model loading works
* texture loading works
* rendering can validate loaded models
* ElForge can inspect core assets
* implemented systems are documented

Research-heavy systems such as maps, definitions, sprites, and animations may be completed incrementally.

If a system is not implemented yet, its unknowns should be captured before agents are asked to build it.

---

## Golden Rule

Phase 2 turns unknown cache bytes into documented, reusable asset systems.

Probe first.

Document findings.

Then implement.
