# Eldoria Roadmap

## Purpose

This document defines the master product roadmap for Eldoria.

It answers:

> What is the global build order from architecture baseline to complete private server ecosystem?

This document is not a task tracker.

Small implementation tasks belong in GitHub Issues or a GitHub Project board.

Detailed application roadmaps belong in:

- `elforge.md`
- `elclient.md`
- `elserver.md`

---

## Product Vision

Eldoria is a custom RuneScape-317-feeling private server ecosystem built in C++.

It is composed of:

- ElForge
- ElClient
- ElServer

### ElForge

The internal development, inspection, debugging, editing, and content creation tool.

It starts as a viewer.

Long-term, it becomes the tool used to create, validate, save, and export Eldoria content.

### ElClient

The player-facing C++ game client.

It should feel recognizable as RuneScape while supporting modern architecture, custom content, smooth controls, offline debugging, and server-authoritative online play.

### ElServer

The authoritative C++ game server.

It owns truth, validates player actions, synchronizes world state, persists player data, runs gameplay systems, and supports real players.

---

## Final Product Goal

The final goal is not only to decode RuneScape cache files.

The final goal is to build a complete custom private server ecosystem where:

- ElForge creates and validates content.
- ElClient presents the playable world.
- ElServer runs the authoritative game.
- Custom models, maps, items, NPCs, bosses, and gameplay can exist.
- The system is polished enough to support a real player base.

Eldoria may respect RuneScape protocol and data concepts where practical, but the long-term goal is a custom game ecosystem.

---

## Roadmap Rules

### Every Phase Must Prove Something

Each phase should produce something:

- visible
- testable
- playable
- inspectable
- deployable

If a phase only produces abstract architecture, it should be broken into a smaller milestone.

### Migration Is Not Rewriting

Eldoria is not a pure greenfield project.

The existing RuneForge-era codebase contains useful systems that should be migrated into the new architecture where appropriate.

Working systems should not be rewritten just because the ownership model changed.

Prefer:

```text
reuse useful code
    ↓
move it into the right module/app
    ↓
adjust names, includes, CMake, and boundaries
    ↓
verify behavior still works
```

Avoid:

```text
delete working systems
    ↓
rebuild from memory
```

### Build Vertical Slices

Eldoria should not be built by finishing one entire domain before touching the next.

Instead, build vertical slices.

A vertical slice means building enough of multiple systems together to prove a useful result.

Example:

```text
Model Viewer
    uses data/model
    uses render
    uses platform
    proves ElForge can inspect and render assets
```

### Probe When Unknown

Unknown systems may begin inside temporary probe apps or temporary panels.

Probe code is allowed to be messy.

Shared module code should be clean.

Once the system is understood, promote it into the proper shared module or app structure.

Example:

```text
apps/animation_probe/
    ↓
src/data/animation/
    ↓
ElForge Animation Viewer
    ↓
ElClient animation support
```

### Apps Prove Domains

Domains define what is being built.

Applications prove that domains work.

Examples:

- `data/model` is proven by ElForge Model Viewer.
- `data/map` and `world` are proven by ElForge Map Viewer and ElClient Local World.
- `net` is proven by the first ElClient/ElServer login loop.
- `game` is proven by server-authoritative gameplay systems.

### Issues Come From App Documents and Migration Audits

This roadmap defines major product order.

GitHub Issues should be generated mainly from:

- `elforge.md`
- `elclient.md`
- `elserver.md`
- migration audit findings

Those documents and audits contain feature areas and issue candidates.

---

## Document Stack

```text
architecture.md
    defines where code belongs

roadmap.md
    defines global product build order

elforge.md
    defines ElForge milestones and issue candidates

elclient.md
    defines ElClient milestones and issue candidates

elserver.md
    defines ElServer milestones and issue candidates

GitHub Issues / Project
    defines small implementation tasks
```

---

# Global Build Order

---

## Phase 0 — Architecture Baseline

### Status

Complete.

### Goal

Define the long-term structure of Eldoria.

### Primary Documents

- `architecture.md`
- `roadmap.md`

### Success Criteria

- Project applications are named.
- Shared modules are defined.
- Source architecture is documented.
- Roadmap direction is established.
- App roadmap files are planned.

### Proves

The project has a clear foundation and future code has an obvious place to live.

---

## Phase 1 — App Skeletons

### Status

Complete.

### Goal

Create the three runnable applications.

### Applications

- ElForge
- ElClient
- ElServer

### Success Criteria

- ElForge builds and launches.
- ElClient builds and launches.
- ElServer builds and launches.
- Each app has its own target.
- Shared modules can be linked cleanly.
- No app depends directly on another app.

### Proves

Eldoria exists as a multi-application workspace.

---

## Phase 2 — RuneForge Migration Audit

### Status

Complete for initial audit.

### Goal

Understand the existing RuneForge-era codebase before moving systems into the Eldoria architecture.

### Main Focus

- Existing app targets
- Existing shared systems
- Reusable code
- Temporary code
- Obsolete code
- Migration order

### Success Criteria

- Existing source tree is summarized.
- Reusable RuneForge subsystems are mapped to Eldoria homes.
- Obsolete or temporary systems are identified.
- The first ElForge/cache migration backlog is created.
- The roadmap is updated to reflect the migration reality.

### Key Decisions

- Eldoria is a migration project, not a greenfield rewrite.
- `src/apps/explorer` is the primary ElForge migration source.
- `src/apps/tool` is an older reference and should be reused selectively only.
- `src/core/cache` should migrate to `src/data/cache`.
- `src/core/assets/model` should migrate to `src/data/model`.
- `src/core/assets/texture` has been migrated to `src/data/texture`.
- `render_next` is likely the future renderer path.
- old `render` is legacy/reference until useful pieces are preserved.
- `src/ui` should not remain shared only for a theme/backend glue; UI ownership should be decided during ElForge migration.

### Proves

The project knows what it is migrating before moving code around.

---

## Phase 3 — ElForge Cache Foundation

### Goal

Make ElForge useful by migrating the existing cache foundation into the new architecture.

### Main App

- ElForge

### Main Domains

- `data`
- `platform`
- ElForge app UI/panels

### Main Migration Sources

- `src/core/io`
- `src/core/compression`
- `src/core/cache`
- `src/apps/explorer`
- selected pieces of `src/apps/tool` only if better or missing from explorer

### Success Criteria

- Binary reading and compression helpers have a clear `data` home.
- Cache reading lives under `src/data/cache`.
- ElForge can browse cache indexes/files using migrated cache code.
- ElForge has a basic cache details/metadata panel.
- ElForge owns app-specific panels instead of hiding them in shared modules.
- The workflow begins replacing manual probe/debug workflows like `archive_probe`.

### Initial Backlog

- #62 — Create `data/cache` module from existing `core/cache` code.
- #63 — Move binary reading and compression support into `data`.
- #64 — Migrate explorer cache browser shell into ElForge.
- #65 — Add ElForge cache file details panel.
- #66 — Decide ElForge UI ownership.
- #67 — Plan renderer path consolidation around `render_next`.

### Related Document

- `elforge.md`

### Proves

The first useful ElForge workflow exists and RuneForge code can be migrated without starting over.

---

## Phase 4 — ElForge Model and Render Foundation

### Goal

Create the first real data-to-render validation path.

### Main App

- ElForge

### Main Domains

- `data`
- `render`
- `platform`

### Main Migration Sources

- `src/core/assets/model`
- `src/render_next`
- selected old `src/render` pieces only if still useful/correct
- selected `src/apps/explorer` viewport/model inspection code

### Success Criteria

- A known model can be loaded through `data/model`.
- A known model can be inspected in ElForge.
- A known model can be rendered through the chosen render path.
- Basic model viewport controls exist.
- `render_next` has a clear migration/consolidation path into final `src/render` ownership.
- The old renderer path is no longer treated as equal to the future path unless audit proves otherwise.

### Related Document

- `elforge.md`

### Proves

Eldoria can move from raw cache browsing to visual asset inspection.

---

## Phase 5 — ElClient Shell

### Goal

Create the first player-facing client shell.

### Main App

- ElClient

### Main Domains

- `platform`
- `render`

### Success Criteria

- ElClient launches.
- Loading screen exists.
- Login screen exists.
- Empty game screen exists.
- Client can transition between screens.
- Basic settings path is planned.

### Related Document

- `elclient.md`

### Proves

ElClient exists as a real application before full world or gameplay systems exist.

---

## Phase 6 — Tiny ElServer and Login Loop

### Goal

Create the smallest useful client/server loop.

### Main Apps

- ElClient
- ElServer

### Main Domains

- `net`
- `platform`

### Success Criteria

- ElServer launches.
- ElServer accepts a connection.
- ElClient connects to ElServer.
- ElClient sends login data.
- ElServer returns login success or failure.
- ElClient enters the game screen after success.
- Packet flow can be logged.

### Related Documents

- `elclient.md`
- `elserver.md`

### Proves

The client/server boundary works before the world becomes complicated.

---

## Phase 7 — Texture and Material Foundation

### Goal

Expand visual asset support.

### Main App

- ElForge

### Main Domains

- `data`
- `render`

### Main Migration Sources

- `src/data/texture` (migrated from `src/core/assets/texture`)
- `render_next` material/texture sampling code

### Success Criteria

- Textures can be loaded through `data/texture`.
- Textures can be inspected.
- Materials exist.
- Model viewer can use texture/material data where available.
- Missing textures have a clear fallback/debug path.

### Related Document

- `elforge.md`

### Proves

Eldoria can handle more than untextured geometry.

---

## Phase 8 — Map and World Inspection

### Goal

Move from isolated assets to world-space data.

### Main App

- ElForge

### Main Domains

- `data`
- `world`
- `render`

### Success Criteria

- Map region data can be loaded.
- Terrain can be decoded.
- Object spawns can be decoded.
- Collision or tile flags can be inspected.
- ElForge can display a basic map/region view.

### Related Document

- `elforge.md`

### Proves

Eldoria can understand and inspect the world, not just individual assets.

---

## Phase 9 — Local World in ElClient

### Goal

Make ElClient load and render an offline local world.

### Main App

- ElClient

### Main Domains

- `data`
- `world`
- `render`
- `platform`

### Success Criteria

- ElClient can load a region.
- ElClient can render terrain.
- ElClient can render placed objects.
- Camera can move around the region.
- Collision can be visualized or partially respected.
- Client world state is separate from rendering state.

### Related Document

- `elclient.md`

### Proves

ElClient can present a real world before server-authoritative gameplay exists.

---

## Phase 10 — Local Player and Animation

### Goal

Put a character into the world and make it feel alive.

### Main Apps

- ElClient
- ElForge

### Main Domains

- `data`
- `world`
- `render`
- `platform`

### Success Criteria

- Local player exists.
- Player appears in the world.
- Camera can follow the player.
- Player can move locally.
- Basic collision can be respected.
- Animation data can be explored.
- Basic idle/walk animation can be applied.

### Related Documents

- `elclient.md`
- `elforge.md`

### Proves

The client can represent a controllable, animated player in a world.

---

## Phase 11 — Server-Authoritative Movement

### Goal

Move from local movement to real online movement.

### Main Apps

- ElClient
- ElServer

### Main Domains

- `world`
- `net`

### Success Criteria

- ElClient sends movement intent.
- ElServer receives movement intent.
- ElServer validates movement.
- ElServer updates authoritative position.
- ElClient receives confirmed position.
- ElClient updates its local world mirror.
- Desync can be debugged.

### Related Documents

- `elclient.md`
- `elserver.md`

### Proves

The first real online gameplay loop works.

---

## Phase 12 — World Synchronization

### Goal

Synchronize world state beyond the local player.

### Main Apps

- ElClient
- ElServer

### Main Domains

- `world`
- `net`
- `render`

### Success Criteria

- Nearby players can be synchronized.
- NPCs can be synchronized.
- World object changes can be synchronized.
- Ground items can be synchronized.
- Region changes can be sent.
- Chat messages can be exchanged.
- ElClient maintains a local mirror of server state.

### Related Documents

- `elclient.md`
- `elserver.md`

### Proves

Eldoria behaves like a real multiplayer world foundation.

---

## Phase 13 — Interfaces, Sprites, and Client Game UI

### Goal

Support RuneScape-style 2D/game interfaces.

### Main Apps

- ElClient
- ElForge

### Main Domains

- `data`
- `render`

### Success Criteria

- Sprites can be loaded.
- Interface definitions can be decoded.
- ElForge can inspect interface data.
- ElClient can display runtime interfaces.
- Chatbox path exists.
- Inventory path exists.
- Equipment path exists.
- Minimap path exists.
- Context menu path exists.

### Related Documents

- `elclient.md`
- `elforge.md`

### Proves

ElClient can become a real RuneScape-style game client, not only a world renderer.

---

## Phase 14 — Core Server Gameplay

### Goal

Implement the first real gameplay systems.

### Main Apps

- ElServer
- ElClient

### Main Domains

- `game`
- `world`
- `net`
- `data`

### Suggested Order

1. Inventory
2. Equipment
3. Object interactions
4. NPC interactions
5. Skills
6. Combat
7. Banking
8. Trading
9. Shops

### Success Criteria

- Server owns gameplay truth.
- Client displays gameplay results.
- Inventory and equipment work.
- Players can interact with objects.
- Players can interact with NPCs.
- Basic skills work.
- Basic combat works.
- Major item movement systems are safe.

### Related Documents

- `elserver.md`
- `elclient.md`

### Proves

Eldoria is becoming an actual game, not only a connected world viewer.

---

## Phase 15 — Content Systems

### Goal

Support authored gameplay content.

### Main Apps

- ElServer
- ElClient
- ElForge

### Main Domains

- `game`
- `world`
- `data`

### Success Criteria

- NPC spawns can be configured.
- Shops can be configured.
- Dialogues can exist.
- Quests can exist.
- Areas can be populated.
- Activities can be added.
- Content state can be saved where needed.

### Related Documents

- `elserver.md`
- `elforge.md`
- `elclient.md`

### Proves

Eldoria can support real authored content instead of hardcoded tests.

---

## Phase 16 — ElForge Editing and Export Pipeline

### Goal

Turn ElForge from inspector into creator.

### Main App

- ElForge

### Main Domains

- `data`
- `world`
- `render`

### Success Criteria

- ElForge project workspace exists.
- Save/export foundation exists.
- Map/object editing exists.
- Definition editing exists where practical.
- Custom asset import exists.
- Content validation exists.
- Exported content can be loaded by ElClient and ElServer.

### Related Document

- `elforge.md`

### Proves

Eldoria has a real custom content pipeline.

---

## Phase 17 — Custom Eldoria Content

### Goal

Use the ecosystem to create custom content.

### Main Apps

- ElForge
- ElClient
- ElServer

### Success Criteria

- Custom models can be imported and displayed.
- Custom maps can be loaded.
- Custom NPCs can exist.
- Custom items/equipment can exist.
- Custom bosses can exist.
- Custom activities or gameplay loops can exist.
- Client and server agree on custom content versions.

### Related Documents

- `elforge.md`
- `elclient.md`
- `elserver.md`

### Proves

Eldoria is no longer just a 317 reproduction. It can become its own game.

---

## Phase 18 — Administration, Security, and Operations

### Goal

Prepare the server ecosystem for real players.

### Main App

- ElServer

### Supporting Apps

- ElClient
- ElForge

### Success Criteria

- Admin commands exist.
- Moderation commands exist.
- Permissions exist.
- Bad packets are handled.
- Rate limits or abuse checks exist.
- Logs are useful.
- Monitoring exists.
- Backups exist.
- Deployment paths are documented.

### Related Document

- `elserver.md`

### Proves

The server is moving toward real private-server infrastructure.

---

## Phase 19 — Production Readiness

### Goal

Prepare Eldoria for serious testing and eventual release.

### Main Apps

- ElForge
- ElClient
- ElServer

### Success Criteria

- ElClient can be packaged or distributed for testing.
- ElServer can run outside the development environment.
- ElForge can manage real project content.
- Content versioning is understood.
- Error handling is acceptable.
- Documentation exists.
- Known limitations are documented.
- Test checklists exist.
- Release checklist exists.

### Related Documents

- `elforge.md`
- `elclient.md`
- `elserver.md`

### Proves

Eldoria is moving from prototype ecosystem to serious product.

---

## Phase 20 — Polish, Optimization, and Growth

### Goal

Improve quality, performance, usability, and long-term maintainability.

### Success Criteria

- Rendering performance improves.
- Server performance improves.
- Tool workflow improves.
- Client feel improves.
- Debug tools improve.
- Documentation stays accurate.
- Old probe code is removed or archived.
- GitHub Issues are cleaned up.
- Architecture remains understandable.

### Related Documents

- all project documents

### Proves

Eldoria can keep growing without collapsing under its own complexity.

---

# Long-Term Build Order

```text
Architecture baseline
    ↓
App skeletons
    ↓
RuneForge migration audit
    ↓
ElForge cache foundation
    ↓
ElForge model/render foundation
    ↓
ElClient shell
    ↓
Tiny ElServer login loop
    ↓
Textures/materials
    ↓
Map/world inspection
    ↓
Local world in ElClient
    ↓
Local player and animation
    ↓
Server-authoritative movement
    ↓
World synchronization
    ↓
Interfaces and client game UI
    ↓
Core server gameplay
    ↓
Content systems
    ↓
ElForge editing/export pipeline
    ↓
Custom Eldoria content
    ↓
Administration/security/operations
    ↓
Production readiness
    ↓
Polish, optimization, and growth
```

---

## GitHub Issues Strategy

This roadmap should not contain hundreds of small tasks.

Small tasks should become GitHub Issues generated from:

- `elforge.md`
- `elclient.md`
- `elserver.md`
- migration audit findings

Recommended hierarchy:

```text
Roadmap Phase
    ↓
App Roadmap Milestone or Migration Audit
        ↓
Feature Area
            ↓
GitHub Issue
```

Example:

```text
Phase 3 — ElForge Cache Foundation
    ↓
Migration Audit Finding: core/cache belongs in data/cache
        ↓
Feature Area: Cache Access
            ↓
Issue: Create data/cache module from existing core cache code
```

Recommended GitHub Project columns:

```text
Todo
Doing
Done
```

Keep the board lightweight.

The goal is memory and direction, not bureaucracy.

---

## Product Completion Criteria

Eldoria can be considered complete enough for serious public/player testing when:

- ElForge can inspect, validate, edit, and export meaningful content.
- ElClient can launch, log in, render the world, present gameplay, and handle custom content.
- ElServer can run authoritatively, synchronize the world, persist player data, and run core gameplay systems.
- Custom content can move from ElForge into ElClient and ElServer.
- The server can run outside the development environment.
- The client can be distributed for testing.
- The ecosystem has enough stability, polish, and documentation to support real players.
- Known limitations are documented instead of hidden.

---

## Roadmap Status

This document represents the current Eldoria master roadmap baseline after the initial RuneForge migration audit.

It is expected to evolve as implementation reveals new requirements.

Major changes should preserve the core principle:

> Every phase should produce something visible, testable, playable, inspectable, or deployable.
