# World Module

## Purpose

`src/world/` owns shared world representation.

It answers:

```text
Where are things, and how is spatial reality represented?
```

The world module defines the spatial model of Eldoria.

It describes where entities exist, how they occupy space, how regions and tiles are represented, and how movement/collision/pathfinding can reason about that space.

World is not the cache decoder.

World is not the gameplay rules layer.

World is the shared spatial state layer built from data and used by game, net, ElClient, ElServer, and ElForge.

---

## Current State

The world module currently exists as a shared architectural module.

Implementation is intentionally minimal.

Current dependency shape:

```text
world
    depends on
        data
```

The module exists so future world systems have a clear ownership boundary.

---

## Direction

Future world systems should live under `src/world/`.

Examples:

```text
src/world/coordinate/
src/world/region/
src/world/tile/
src/world/entity/
src/world/object/
src/world/collision/
src/world/pathfinding/
src/world/map/
src/world/snapshot/
```

If a feature is about space, location, movement, visibility, regions, tiles, occupancy, placed objects, entities, or world snapshots, it probably belongs in `world/`.

---

## Relationship To Data

`data/` owns static facts loaded from cache/content sources.

`world/` turns those facts into spatial representation.

Examples:

```text
data/object/ObjectDefinition
= object type definition

world/object/WorldObject
= object instance placed at a coordinate
```

```text
data/map/MapRegionData
= decoded static terrain/location data

world/region/Region
= spatial region built from decoded data
```

World should not read raw cache sectors or decode cache formats directly.

World should consume clean data structures from `data/`.

---

## What Belongs Here

Examples:

```text
WorldPosition
Tile
Chunk
Region
WorldObject
NpcSpawn
PlayerPosition
CollisionMap
Pathfinder
WorldSnapshot
```

Questions that belong to world:

```text
Where is the player?
Which tile is occupied?
Can an entity move here spatially?
Which region contains this coordinate?
Which objects are visible in this region?
What world snapshot should be sent or displayed?
```

---

## What Does Not Belong Here

Examples:

```text
Combat
Inventory
Skills
Quests
Packets
Rendering
Cache decoding
UI
```

Questions that do NOT belong to world:

```text
Can a sword be equipped?
How much damage was dealt?
How is a packet encoded?
How is a model rendered?
How is a cache file decoded?
Which UI should appear?
```

---

## Future Structure

Likely shape:

```text
src/world/
├── coordinate/
├── region/
├── tile/
├── entity/
├── object/
├── collision/
├── pathfinding/
├── map/
└── snapshot/
```

Structure may evolve as implementation grows.

---

## Dependency Rules

May depend on:

* data

Should not depend on:

* apps
* render
* net
* game

Game may depend on world.

World should not depend on game.

Net may depend on world types for protocol state/update structures.

World should not depend on net.

---

## Used By

`world/` may be used by:

* ElForge, for world/map inspection and editing workflows
* ElClient, for displaying local or synchronized world state
* ElServer, for authoritative world state
* game, for applying gameplay rules to spatial state
* net, for describing synchronized world updates

---

## Boundary Examples

```text
data/object/ObjectDefinition
= object type

world/object/WorldObject
= object placed in the world

game/object/ObjectInteraction
= gameplay behavior
```

```text
data/map/MapRegionData
= static map data

world/map/WorldMap or world/region/Region
= runtime spatial representation

render/scene/RenderScene
= renderable scene input
```

---

## Common Mistakes

Do not:

* put combat in world
* put inventory logic in world
* put packet code in world
* put rendering code in world
* put cache decoding in world
* make world depend on applications
* make world depend on game rules
* treat ElClient local debug state as the authoritative world model

---

## When Adding New Code

Ask:

```text
Is this about location, movement, space, visibility, occupancy, regions, tiles, or placed world state?
```

If yes, it probably belongs in world.

If it is about static data decoding, it belongs in data.

If it is about rules, it probably belongs in game.

If it is about pixels, it belongs in render.

If it is about packets, it belongs in net.

---

## Golden Rule

World owns spatial reality.

World does not own static data decoding.

World does not own gameplay authority.
