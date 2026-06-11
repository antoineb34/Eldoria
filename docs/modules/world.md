# World Module

## Purpose

`src/world/` owns shared world representation.

It answers:

```text
Where are things and how do they move?
```

The world module defines the spatial model of Eldoria.

It describes where entities exist, how they occupy space, how movement works, and how world data is represented.

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
src/world/entity/
src/world/object/
src/world/collision/
src/world/pathfinding/
src/world/map/
```

If a feature is about space, location, movement, visibility, regions, tiles, or world representation, it probably belongs in `world/`.

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
```

Questions that belong to world:

```text
Where is the player?
Which tile is occupied?
Can an entity move here?
Which region contains this coordinate?
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
```

---

## Future Structure

Likely shape:

```text
src/world/
├── coordinate/
├── region/
├── entity/
├── object/
├── collision/
├── pathfinding/
└── map/
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

Game may depend on world.

World should not depend on game.

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
data/map/MapDefinition
= static map data

world/map/WorldMap
= runtime spatial representation
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

---

## When Adding New Code

Ask:

```text
Is this about location, movement, space, visibility, or occupancy?
```

If yes, it probably belongs in world.

If it is about rules, it probably belongs in game.

---

## Golden Rule

World owns space.

World does not own gameplay.
