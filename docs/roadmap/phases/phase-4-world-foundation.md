# Phase 4 - World Foundation

## Purpose

Phase 4 establishes the foundational world architecture required by Eldoria.

The goal is to create the first structured world representation that ElClient can display and that future ElServer systems can own authoritatively.

This phase is not about full gameplay.

This phase is not about combat.

This phase is not about skills, inventory, quests, or content systems.

This phase is about world ownership, map representation, camera behavior, entity foundations, player representation, and basic movement architecture.

---

## Current Status

Phase 4 begins after ElClient has a stable application foundation.

Phase 3 established:

* client lifecycle
* screen management
* input foundation
* render integration
* UI foundation
* login screen foundation
* network bootstrap

Phase 4 builds the first game-world foundation.

---

## Outcome

Phase 4 is complete when Eldoria can represent and display a minimal world scene with a player entity foundation.

Required outcomes:

* world ownership exists
* scene ownership exists
* map/tile representation exists
* camera foundation exists
* entity foundation exists
* local player representation exists
* movement foundation exists
* world render integration exists

Phase 4 turns ElClient from a client shell into a basic world viewer/player foundation.

---

## Core Rule

World systems should be built around ownership boundaries.

Expected ownership direction:

```text
ElServer
    owns authoritative world state later

ElClient
    owns local presentation and prediction later

src/world
    owns shared world data structures

src/game
    owns gameplay rules later

src/render
    owns rendering only
```

Do not put gameplay rules in render code.

Do not put server authority in ElClient.

Do not put client presentation logic in shared world data.

---

# 4.1 World Data Model

## Purpose

Create reusable world data structures.

## Required Implementation

* World
* WorldScene
* TilePosition
* WorldPosition
* Region or Zone concept if needed
* basic tile/grid representation

## Exit Criteria

* world state can be represented
* positions can be represented
* scene ownership is clear
* no gameplay logic is introduced

---

# 4.2 Map Representation Foundation

## Purpose

Represent static map data needed by the world scene.

This does not require full cache map decoding unless needed by the issue.

## Required Implementation

* MapTile
* TileHeight or placeholder height data
* TileCollision placeholder if needed
* static object placement placeholder if needed

## Exit Criteria

* map/tile data can be represented
* world scene can reference map data
* implementation does not require full map editor
* unknown cache formats are not guessed

---

# 4.3 Camera Foundation

## Purpose

Add camera ownership for viewing the world.

## Required Implementation

* WorldCamera
* camera position
* camera rotation
* zoom or distance
* basic update controls

## Exit Criteria

* camera can view a world scene
* camera ownership is clear
* camera is not hardcoded into render internals

---

# 4.4 Entity Foundation

## Purpose

Create the base structure for world entities.

## Required Implementation

* EntityId
* Entity
* EntityType
* entity position
* entity update hook
* entity render representation hook

## Exit Criteria

* entities can exist in a world scene
* entity identity is represented
* entity position is represented
* no combat, NPC AI, inventory, or skills are introduced

---

# 4.5 Local Player Foundation

## Purpose

Represent the local player as the first controllable entity.

## Required Implementation

* LocalPlayer
* player position
* player facing/direction
* player model placeholder or render reference
* integration with world scene

## Exit Criteria

* local player exists
* local player is part of the world scene
* local player can be rendered as a placeholder or model
* no account, stats, inventory, combat, or equipment systems are introduced

---

# 4.6 Movement Foundation

## Purpose

Create basic movement architecture.

This phase does not require final RuneScape movement rules.

## Required Implementation

* movement request representation
* target tile or direction movement
* basic position update
* movement validation placeholder

## Exit Criteria

* local player can request movement
* position can update through world systems
* movement does not bypass world ownership
* no server authority or prediction complexity is required yet

---

# 4.7 World Render Integration

## Purpose

Connect world data to rendering.

Rendering should consume world scene data without owning world logic.

## Required Implementation

* world-to-render conversion
* render scene generation from world scene
* tile/grid visualization
* entity placeholder rendering
* local player rendering

## Exit Criteria

* world scene can be rendered
* camera can view the scene
* local player appears in the scene
* render code does not own gameplay/world logic

---

## Not Included In Phase 4

Phase 4 does not require:

* combat
* skills
* inventory
* equipment
* quests
* NPC AI
* full multiplayer
* server authority
* final map decoding
* full collision system
* pathfinding
* animation system
* production world streaming

---

## Issue Breakdown Strategy

Each world system should be implemented as a focused issue.

Preferred order:

```text
1. World data model
2. Map/tile representation
3. Camera foundation
4. Entity foundation
5. Local player foundation
6. Movement foundation
7. World render integration
```

Good issue:

```text
Add world position and tile coordinate types.
```

Bad issue:

```text
Implement the game world.
```

Good issue:

```text
Add local player entity foundation.
```

Bad issue:

```text
Add player movement, combat, inventory, and networking.
```

---

## Phase 4 Completion Criteria

Phase 4 is complete when:

* world data model exists
* world scene ownership exists
* map/tile representation exists
* camera foundation exists
* entity foundation exists
* local player foundation exists
* basic movement architecture exists
* world render integration exists
* ElClient can display a minimal world scene

When these criteria are satisfied, Eldoria has a basic playable-world foundation.

---

## Transition To Phase 5

Phase 5 should focus on client-server gameplay connection.

Likely Phase 5 areas:

* server-authoritative world state
* login-to-world transition
* player session
* movement packets
* entity synchronization
* basic multiplayer visibility
* disconnect/reconnect behavior

Phase 5 is where ElClient and ElServer begin acting as a connected game system.

---

## Golden Rule

Phase 4 creates the world foundation.

Represent the world first.

Render it second.

Gameplay comes later.
