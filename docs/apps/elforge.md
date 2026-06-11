# ElForge

## Purpose

ElForge is the internal development tool for Eldoria.

It exists to inspect, validate, debug, and eventually create Eldoria content.

ElForge is the primary environment used to understand cache data, verify asset pipelines, debug rendering systems, and build future content creation workflows.

ElForge is a developer tool.

It is not the player-facing game client.

---

## Current State

ElForge currently consists primarily of the Cache Explorer workflow.

Current architecture:

```text
Cache Explorer
    ├── Cache Tree Panel
    ├── Cache Inspector Panel
    └── Cache Viewport Panel
```

The Cache Explorer composes:

* Cache
* ModelLoader
* TextureLoader

Current responsibilities include:

* cache browsing
* cache asset selection
* model loading
* texture loading
* asset inspection
* viewport rendering
* rendering diagnostics

Current workflows are primarily inspection and validation workflows.

ElForge is currently more inspector than editor.

---

## Current UI Structure

Current UI is organized around panels.

```text
Cache Tree Panel
    ↓
Selection

Cache Inspector Panel
    ↓
Asset Information

Cache Viewport Panel
    ↓
Asset Visualization
```

Panels should remain focused.

Shared systems should remain outside ElForge.

---

## Application Flow

### Startup Flow

```text
main()
    ↓
ElForge Application
    ↓
CacheExplorer
    ↓
Initialize Cache
    ↓
Initialize Loaders
    ↓
Build Cache Tree
    ↓
Ready
```

Purpose:

Prepare the tool for asset exploration.

---

### Main Loop

```text
SDL Events
    ↓
CacheExplorer::handleEvent()

    ↓

CacheExplorer::update()

    ↓

CacheExplorer::renderUi()

    ↓

CacheExplorer::renderViewport()
```

Purpose:

* process input
* update state
* render UI
* render viewport

---

### State Flow

The central application state is:

```text
CacheExplorerState
```

Current state owns:

```text
selection
activeModel
activeTexture

camera
renderOptions
modelTransform

viewport bounds

expanded UI nodes
```

Current architecture:

```text
Cache Tree
        ↓
   Selection
        ↓
CacheExplorerState
        ↓
 Inspector
        ↓
 Viewport
```

Panels consume state.

Panels do not own state.

---

### Cache Exploration Flow

```text
Cache
    ↓
CacheTreeBuilder
    ↓
Cache Tree Nodes
    ↓
CacheTreePanel
    ↓
User Selection
    ↓
CacheSelection
```

Purpose:

Allow developers to browse cache contents.

---

### Asset Selection Flow

User selections are handled by:

```text
CacheExplorer
    ↓
handleSelectionChanged()
```

#### Model Selection

```text
Selection
    ↓
ModelLoader
    ↓
ModelAsset
    ↓
state.activeModel
```

#### Texture Selection

```text
Selection
    ↓
TextureLoader
    ↓
TextureAsset
    ↓
state.activeTexture
```

Purpose:

Convert cache selections into usable assets.

---

### Inspection Flow

```text
selection
activeModel
activeTexture
    ↓
CacheInspectorPanel
```

Purpose:

Display information about the currently selected asset.

---

### Viewport Flow

```text
activeModel
    ↓
CacheViewportPanel
    ↓
RenderObject
    ↓
RenderScene
    ↓
RenderPipeline
    ↓
SoftwareRenderBackend
    ↓
Viewport Output
```

Purpose:

Visualize loaded assets.

The viewport prepares render input.

The rendering system performs rendering.

---

## Long-Term Direction

ElForge should eventually become the primary content pipeline for Eldoria.

Future capabilities may include:

* model editing
* texture editing
* animation inspection
* animation editing
* map inspection
* map editing
* NPC editing
* item editing
* interface editing
* world editing
* content validation
* content export

ElForge should become the primary tool used to create and maintain Eldoria content.

---

## Ownership

ElForge owns:

* tool workflows
* editor workflows
* inspector workflows
* debugging workflows
* developer-facing UI
* viewport controls
* tool state
* tool navigation
* content inspection workflows

ElForge does not own:

* cache decoding
* model decoding
* texture decoding
* rendering implementation
* gameplay rules
* networking implementation
* world simulation

Those responsibilities belong to shared modules.

---

## Architectural Position

ElForge sits at the top of the architecture.

It composes shared systems into developer workflows.

```text
ElForge
    ↓
data
    ↓
render
    ↓
platform
```

ElForge should consume systems.

It should not reimplement them.

---

## Relationship To Systems

ElForge consumes systems.

Examples:

```text
Cache Exploration
    uses
        Cache System
```

```text
Model Inspection
    uses
        Model Loading System
```

```text
Texture Inspection
    uses
        Texture Loading System
```

```text
Viewport Rendering
    uses
        Rendering System
        ElForge Viewport System
```

ElForge should orchestrate systems.

It should not replace them.

---

## Future Workflows

Examples:

```text
Map Editor
NPC Editor
Item Editor
Interface Editor
Animation Editor
World Editor
```

Future workflows should build on shared systems rather than duplicating functionality.

---

## Common Mistakes

Do not:

* decode cache files inside ElForge
* decode models inside ElForge
* decode textures inside ElForge
* implement rendering pipelines inside ElForge
* duplicate shared systems
* make shared modules depend on ElForge
* move reusable logic into application code

When functionality becomes reusable, it should usually move into a shared module.

---

## Extension Guidelines

Before adding functionality to ElForge:

1. Determine whether the behavior is tool-specific.
2. Determine whether the behavior belongs in a shared module.
3. Keep reusable logic in shared modules.
4. Keep developer workflows in ElForge.
5. Reuse existing systems whenever possible.

Ask:

```text
Is this a tool workflow or a reusable system?
```

If it is a reusable system:

```text
data/
world/
game/
net/
render/
platform/
```

If it is a tool workflow:

```text
apps/elforge/
```

---

## Verification

```bash
cmake --build build --target elforge
```

Manual verification:

```text
Open ElForge.
Verify cache tree renders.
Verify selection updates.
Verify inspector updates.
Verify models load.
Verify viewport renders.
Verify viewport controls still work.
```

---

## Golden Rule

ElForge is a tool.

Shared modules perform the work.

ElForge exposes that work to developers.
