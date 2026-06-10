# Eldoria Architecture

## Purpose

This document defines the architecture of Eldoria.

It answers:

> Where does code belong, and which part of Eldoria owns which responsibility?

This document is not a roadmap and not a task tracker.

Detailed build order belongs in:

- `roadmap.md`

Detailed application plans belong in:

- `elforge.md`
- `elclient.md`
- `elserver.md`

Small implementation tasks belong in GitHub Issues.

---

## Product Vision

Eldoria is a custom RuneScape-317-feeling private server ecosystem built in C++.

It is composed of three applications:

- ElForge
- ElClient
- ElServer

The goal is not only to decode old RuneScape data.

The goal is to build a complete ecosystem where:

- ElForge creates, edits, validates, saves, and exports content.
- ElClient presents the playable world to players.
- ElServer owns the authoritative game state and runs the online world.
- Shared modules provide reusable systems used by the apps.
- Custom maps, models, items, NPCs, bosses, interfaces, and gameplay can exist.

Eldoria should respect RuneScape protocol and data concepts where practical, but the long-term goal is a custom game ecosystem.

---

## Source Overview

```text
src/
├── apps/
├── data/
├── world/
├── game/
├── net/
├── render/
└── platform/
```

---

## Core Mental Model

```text
apps
= runnable products

data
= what things are

world
= where things are and how they move

game
= what rules apply

net
= how state and actions travel

render
= how things become pixels

platform
= how Eldoria talks to the machine
```

---

# Application Roles

## ElForge

ElForge is the development, inspection, debugging, editing, and content creation tool.

It owns:

- tool UI
- project workspaces
- cache browsing workflows
- asset inspection workflows
- editor workflows
- save/export workflows
- validation reports
- import/export tools

It does not own:

- cache formats
- model decoding
- map decoding
- rendering backend
- server authority
- client runtime behavior

ElForge combines shared systems into a tool experience.

---

## ElClient

ElClient is the player-facing game client.

It owns:

- startup flow
- loading flow
- login flow
- screen transitions
- local client session state
- local world mirror
- client camera behavior
- input mapping
- runtime game interfaces
- client packet handling
- offline debug mode
- client-side error presentation

It does not own:

- authoritative gameplay truth
- player persistence
- content editing
- cache decoding
- packet definitions
- rendering backend implementation

ElClient combines shared systems into a playable client experience.

---

## ElServer

ElServer is the authoritative game server.

It owns:

- server startup and shutdown
- configuration
- logging
- network listener
- sessions
- login handling
- authoritative world state
- tick processing
- player runtime state
- entity synchronization
- gameplay validation
- persistence
- admin/moderation systems
- deployment and server operations

It does not own:

- client UI
- rendering
- content editing
- tool workflows
- raw asset decoding logic when that belongs in shared data modules

ElServer combines shared systems into an authoritative online world.

---

## Module Usage

```text
Module      ElForge    ElClient    ElServer
------------------------------------------------
data           yes        yes         yes
world          yes        yes         yes
game           limited    limited     yes
net            debug      yes         yes
render         yes        yes         no
platform       yes        yes         yes
```

Notes:

- ElClient may use limited gameplay logic for prediction, presentation, and interface support.
- ElServer remains authoritative for gameplay rules.
- ElForge may use `game` or `net` later for debugging, validation, or simulation tools.
- ElServer may use limited `platform` services such as filesystem and time.

---

## Global Dependency Rule

Applications may depend on shared modules.

Shared modules must not depend on applications.

Allowed:

```text
apps/elclient
    depends on render
        depends on data
```

Not allowed:

```text
data
    depends on apps/elclient
```

Shared modules should remain reusable.

Applications compose shared modules into products.

---

# Shared Modules

---

## `src/data/`

## Purpose

`data/` contains static game data and data-loading systems.

It answers:

> What are things, and how are they loaded?

## Used By

- ElForge
- ElClient
- ElServer

## Owns

- binary helpers used for data formats
- compression helpers used for cache/data loading
- cache reading
- asset decoding
- asset loading
- static definitions
- model data
- texture data
- map data
- animation data
- interface definitions
- sprite data
- item/NPC/object definitions

## Does Not Own

- live world state
- gameplay authority
- rendering behavior
- networking behavior
- app-specific UI
- editing workflows

## Planned Structure

```text
src/data/
├── binary/
├── cache/
├── model/
├── texture/
├── map/
├── animation/
├── item/
├── npc/
├── object/
├── interface/
└── sprite/
```

## Boundary Examples

```text
data/item/ItemDefinition
= what an item type is

data/object/ObjectDefinition
= what an object type is

data/model/ModelAsset
= usable static model data
```

## Dependency Rule

`data/` should not depend on:

- `apps/`
- `world/`
- `game/`
- `net/`
- `render/`

Other modules may depend on `data/`.

---

## `src/world/`

## Purpose

`world/` defines shared world concepts and mechanics.

It answers:

> Where are things, what occupies space, and how can things move?

## Used By

- ElForge
- ElClient
- ElServer

## Owns

- coordinates
- regions/chunks/zones
- terrain concepts
- tile concepts
- placed world objects
- shared entity concepts
- collision maps
- movement representation
- pathfinding
- world queries

## Does Not Own

- static cache decoding
- rendering
- gameplay rewards/rules
- client-only world presentation
- server-only tick processing
- tool-only map editing UI

## Planned Structure

```text
src/world/
├── coord/
├── spatial/
├── terrain/
├── object/
├── entity/
├── collision/
├── movement/
├── pathfinding/
└── query/
```

## Boundary Examples

```text
data/object/ObjectDefinition
= what an oak tree type is

world/object/WorldObject
= an oak tree exists at a tile

game/object/ObjectInteraction
= chopping the tree gives logs and XP
```

## Runtime Ownership

`world/` defines shared world types and mechanics.

Applications own their runtime world state.

```text
ElServer
= authoritative world state

ElClient
= local world mirror

ElForge
= inspection/editing view of the world
```

## Dependency Rule

`world/` may depend on:

- `data/`

`world/` should not depend on:

- `apps/`
- `render/`
- `net/`
- client-specific state
- server-specific state
- tool-specific state

---

## `src/game/`

## Purpose

`game/` defines shared gameplay rules and gameplay concepts.

It answers:

> What actions are allowed, what outcomes occur, and how do game systems interact?

## Used By

- ElServer
- ElClient, limited
- ElForge, limited/debugging later

## Owns

- player gameplay concepts
- inventory rules
- equipment rules
- combat rules
- skill rules
- prayer rules
- magic rules
- item action concepts
- object interaction concepts
- NPC interaction concepts
- shops
- trading
- banking
- quests
- commands where shared

## Does Not Own

- rendering
- networking transport
- app-specific UI
- cache decoding
- live server session management
- client presentation details

## Planned Structure

```text
src/game/
├── player/
├── inventory/
├── equipment/
├── combat/
├── skills/
├── prayer/
├── magic/
├── npc/
├── item/
├── object/
├── dialogue/
├── shop/
├── trade/
├── bank/
├── quest/
└── command/
```

## Server Authority

ElServer is authoritative for gameplay.

ElClient may use limited gameplay logic for:

- prediction
- presentation
- interface support
- local validation hints

ElForge may use gameplay logic later for:

- content validation
- simulation/debugging tools
- editor previews

## Boundary Examples

```text
data/item/ItemDefinition
= what a sword is

game/equipment/EquipmentRules
= whether the player can equip the sword

apps/elserver/player/ServerPlayer
= the player actually equipping it online

apps/elclient/interface/InventoryView
= the client displaying it
```

## Dependency Rule

`game/` may depend on:

- `data/`
- `world/`

`game/` should not depend on:

- `apps/`
- `render/`
- client-specific systems
- server-specific systems
- tool-specific systems

---

## `src/net/`

## Purpose

`net/` defines the shared networking language used by Eldoria.

It answers:

> What messages exist, how are they encoded, and how do bytes become packets?

## Used By

- ElClient
- ElServer
- ElForge, later for packet debugging tools

## Owns

- protocol states
- packet identities
- packet sizes
- packet structures
- packet reading
- packet writing
- packet encoding
- packet decoding
- protocol crypto/obfuscation
- shared transport helpers if needed
- packet debugging primitives

## Does Not Own

- ElClient connection flow
- ElServer listener/session behavior
- login policy
- gameplay validation
- world synchronization decisions
- app-specific packet handling behavior

## Planned Structure

```text
src/net/
├── protocol/
├── packet/
├── codec/
├── crypto/
├── transport/
└── debug/
```

## Protocol Philosophy

Eldoria should respect RuneScape protocol concepts where practical.

Examples:

- login flow
- opcode structure
- packet format style
- ISAAC usage where applicable
- movement/message concepts

The goal is not perfect compatibility with every external RSPS.

The goal is to build Eldoria cleanly while keeping the protocol recognizable and adaptable.

## App Boundaries

```text
src/net/
= shared packet/protocol language

apps/elclient/net/
= client connection behavior

apps/elserver/net/
= server listener and session behavior
```

## Dependency Rule

`net/` may depend on:

- `data/binary/`

`net/` should not depend on:

- `apps/`
- `render/`
- `game/`
- `world/`
- client-specific systems
- server-specific systems

---

## `src/render/`

## Purpose

`render/` is the shared rendering system.

It answers:

> How do renderable scenes become pixels?

## Current Implementation Status

`src/render_next/` is the intended future renderer architecture.

It currently owns the newer scene/pipeline/backend shape:

- `RenderScene`
- `RenderObject`
- `RenderPipeline`
- `RenderBackend`
- software framebuffer/depth-buffer rendering
- material and texture sampling stages

The existing `src/render/` module is legacy/reference code plus some still-used primitives.

It still contains useful low-level pieces such as:

- camera and projection types
- math types
- color conversion helpers
- model transform/options types used by current tools
- older direct SDL model rendering code

Final ownership should converge back to `src/render/` once `render_next` has replaced the old path.

Until then:

- do not delete `src/render/`
- do not rename `render_next`
- preserve useful primitives before removing old renderer code
- migrate behavior in small steps after parity is confirmed

## Used By

- ElForge
- ElClient

## Not Used By

- ElServer

## Owns

- render math
- render scenes
- render cameras
- render geometry
- materials
- texture sampling for rendering
- render pipeline
- CPU backend
- future GPU backend
- text rendering
- viewports
- render debug tools

## Does Not Own

- cache decoding
- asset file formats
- live world authority
- gameplay rules
- networking
- app-specific UI behavior
- server logic

## Planned Structure

```text
src/render/
├── math/
├── scene/
├── camera/
├── geometry/
├── material/
├── pipeline/
├── backend/
│   ├── cpu/
│   └── gpu/
├── text/
├── viewport/
└── debug/
```

## Rendering Flow

```text
RenderScene
    ↓
RenderPipeline
    ↓
RenderBackend
    ↓
Pixels
```

## Boundary Examples

```text
world/object/WorldObject
= object exists at a tile

render/scene/RenderObject
= object is submitted for drawing

data/model/ModelAsset
= static model data used to build render geometry
```

## Backend Rule

Applications should not care whether rendering is CPU or GPU.

CPU/GPU are backend implementation details.

## Dependency Rule

`render/` may depend on:

- `data/`
- `platform/`

`render/` should not depend on:

- `apps/`
- `world/`
- `game/`
- `net/`

---

## `src/platform/`

## Purpose

`platform/` provides Eldoria's interface to the machine and external runtime environment.

It answers:

> How does Eldoria talk to the operating system and platform libraries?

## Used By

- ElForge
- ElClient
- ElServer, limited

## Owns

- window abstraction
- input abstraction
- audio device access
- filesystem access
- time/timers
- clipboard
- native dialogs
- SDL integration

## Does Not Own

- gameplay rules
- rendering pipeline logic
- cache decoding
- networking protocol
- app-specific behavior
- content editing
- client UI

## Planned Structure

```text
src/platform/
├── window/
├── input/
├── audio/
├── filesystem/
├── time/
├── clipboard/
├── dialog/
└── sdl/
```

## Platform Philosophy

`platform/` is not just SDL.

It represents Eldoria's interface to the machine.

Examples:

```text
platform/filesystem
= std::filesystem or future platform-specific filesystem behavior

platform/time
= std::chrono or future timing helpers

platform/sdl
= SDL-specific implementation details
```

## Dependency Rule

`platform/` should not depend on:

- `apps/`
- `data/`
- `world/`
- `game/`
- `net/`
- `render/`

All other modules may depend on `platform/` when machine/runtime services are required.

---

# Application Boundaries

## `src/apps/`

## Purpose

`apps/` contains runnable products.

It answers:

> What can be built and launched?

## Planned Structure

```text
src/apps/
├── elforge/
├── elclient/
└── elserver/
```

Each app may have its own internal architecture.

Examples:

```text
apps/elforge/
= tool workspaces, panels, inspectors, editors

apps/elclient/
= screens, client world mirror, client networking, runtime interfaces

apps/elserver/
= listener, sessions, server world, persistence, admin tools
```

Apps may depend on shared modules.

Apps should not depend directly on each other.

For example:

```text
ElClient should not include ElServer headers.

ElServer should not include ElClient headers.

ElForge should not be required for ElClient or ElServer to run.
```

Shared behavior should move into shared modules.

---

# Content Pipeline

Eldoria's long-term content pipeline is:

```text
ElForge
    creates / edits / validates / exports content
        ↓
shared data/world/game formats
        ↓
ElClient
    loads and presents content
        ↓
ElServer
    loads and runs authoritative content
```

## Content Ownership

ElForge owns the editing workflow.

Shared modules own reusable representations and loaders.

ElClient owns player-facing presentation.

ElServer owns runtime authority and gameplay application.

## Examples

Custom model:

```text
ElForge imports/previews/exports model
    ↓
data/model represents model data
    ↓
ElClient renders model
    ↓
ElServer references model through definitions/content where needed
```

Custom map:

```text
ElForge edits map/object placement
    ↓
data/map and world represent map/world data
    ↓
ElClient displays the map
    ↓
ElServer uses world/collision/object data authoritatively
```

Custom NPC:

```text
ElForge edits NPC definition/spawn
    ↓
data/npc represents static NPC definition
    ↓
ElClient displays NPC
    ↓
ElServer owns NPC state, behavior, combat, and drops
```

---

# Top-Level Modules Not Included Initially

## No shared product UI module initially

There is no top-level shared `src/ui/` product UI module at this stage.

Reason:

- ElForge UI is tool UI: panels, dockspaces, inspectors, editors.
- ElClient UI is game UI: chatbox, inventory, minimap, widgets, interfaces.

These systems are too different to force into one shared module.

Current ownership:

- ElForge UI belongs in `apps/elforge/`.
- ElForge ImGui theme/look-and-feel belongs in `apps/elforge/`.
- ElClient runtime interfaces belong in `apps/elclient/interface/`.
- Interface definitions loaded from cache belong in `src/data/interface/`.
- Shared window/input behavior belongs in `src/platform/`.
- Shared ImGui backend/vendor build plumbing belongs in `src/platform/imgui/`.

A shared `src/ui/` module may be introduced later only if real duplicated product UI code appears.

---

# Probe Code Rule

Exploration is allowed.

Unknown systems may begin as temporary probe apps or temporary panels.

Examples:

```text
apps/model_probe/
apps/animation_probe/
apps/interface_probe/
apps/protocol_probe/
```

Probe code may be messy.

Probe code should not become permanent architecture.

Once a system is understood:

```text
probe
    ↓
notes / understanding
    ↓
clean shared module
    ↓
ElForge validation tool
    ↓
ElClient or ElServer integration
```

This keeps experimentation fast without letting experimental code pollute the architecture.

---

# Runtime Authority Model

## Offline / Debug Mode

ElClient may run offline for debugging.

In offline mode, ElClient may temporarily own local world/player state.

This is useful for:

- map rendering
- camera testing
- collision debugging
- animation testing
- interface testing
- content validation

## Online Mode

In online mode, ElServer owns the truth.

ElClient sends intentions.

ElServer validates and applies actions.

ElClient displays the result.

Examples:

```text
movement:
client sends movement intent
server validates movement
server sends confirmed position

inventory:
client requests item action
server validates inventory state
server sends updated inventory

combat:
client requests attack
server validates target/range/timing
server sends combat updates
```

---

# GitHub Issues Relationship

Architecture does not define individual tasks.

Architecture defines ownership and boundaries.

GitHub Issues should be generated from:

- `roadmap.md`
- `elforge.md`
- `elclient.md`
- `elserver.md`

Architecture should be used to decide where each issue's code belongs.

Example:

```text
Issue:
Render placed world objects in ElClient

Likely code areas:
apps/elclient/world/
apps/elclient/render/
src/world/object/
src/render/scene/
src/data/object/
```

---

# Architecture Status

This document represents the current Eldoria architecture baseline.

The architecture is expected to evolve as implementation reveals new requirements.

Major changes should preserve the core principles:

- shared modules do not depend on apps
- apps compose shared modules into products
- ElServer owns authority
- ElClient presents the game
- ElForge creates and validates content
- architecture should reduce complexity, not create ceremony
