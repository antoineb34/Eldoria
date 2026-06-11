# Game Module

## Purpose

`src/game/` owns gameplay rules.

It answers:

```text
What rules apply?
```

The game module defines the logic that turns world state and player actions into gameplay outcomes.

---

## Current State

The game module currently exists as a shared architectural module.

Implementation is intentionally minimal.

Current dependency shape:

```text
game
    depends on
        data
        world
```

The module exists to provide a dedicated home for future gameplay systems.

---

## Direction

Future gameplay systems should live under `src/game/`.

Examples:

```text
src/game/combat/
src/game/item/
src/game/inventory/
src/game/equipment/
src/game/skill/
src/game/prayer/
src/game/magic/
src/game/quest/
```

If a feature is about gameplay rules, it probably belongs in game.

---

## What Belongs Here

Examples:

```text
Combat
Inventory
Equipment
Skills
Quests
Trading
Shops
Banking
Item interactions
Object interactions
NPC interactions
```

Questions that belong to game:

```text
Can this item be equipped?
How much damage is dealt?
Can this quest progress?
Can this item be traded?
```

---

## What Does Not Belong Here

Examples:

```text
Rendering
Packets
SDL
Cache decoding
Model loading
Texture loading
UI
```

Questions that do NOT belong to game:

```text
How is a model rendered?
How is a packet encoded?
How is a cache file read?
```

---

## Future Structure

Likely shape:

```text
src/game/
├── combat/
├── inventory/
├── equipment/
├── item/
├── npc/
├── player/
├── skill/
├── prayer/
├── magic/
├── quest/
└── shop/
```

Structure may evolve as implementation grows.

---

## Dependency Rules

May depend on:

* data
* world

Should not depend on:

* render
* apps
* platform

Applications may depend on game.

Game should not depend on applications.

---

## Boundary Examples

```text
data/item/ItemDefinition
= item data

game/equipment/EquipmentRules
= equip rules

apps/elserver/player
= authoritative execution
```

```text
world/object/WorldObject
= object in world

game/object/ObjectInteraction
= interaction behavior
```

---

## Common Mistakes

Do not:

* put rendering logic in game
* put packet logic in game
* put cache decoding in game
* put SDL code in game
* make game depend on applications

---

## When Adding New Code

Ask:

```text
Is this a gameplay rule?
```

If yes, it probably belongs in game.

---

## Golden Rule

Game owns rules.

Applications own execution.
