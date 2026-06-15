# Eldoria Roadmap

## Purpose

This document defines the long-term direction of Eldoria.

It explains:

* what Eldoria is
* what Eldoria should become
* the principles guiding development
* the major areas expected to exist in the future
* the current architectural sequence of development

This document intentionally remains high level.

Detailed planning belongs in:

```text
docs/roadmap/phases/
```

Phase documents should only be created when an area becomes relevant for active development.

---

## Vision

Eldoria is a custom RuneScape-inspired private server ecosystem written in C++.

The project is not simply:

* a cache viewer
* a client remake
* a server experiment
* a rendering sandbox

The goal is a complete ecosystem for building, running, maintaining, and expanding a custom online game world.

Eldoria consists of:

* ElForge
* ElClient
* ElServer

Shared modules provide reusable systems used by all applications.

---

## End Goal

A completed Eldoria should allow developers to:

* inspect content
* create content
* edit content
* validate content
* export content
* run servers
* connect clients
* build custom gameplay
* expand the game without major architectural rewrites

The project should eventually support:

* custom models
* custom textures
* custom animations
* custom maps
* custom NPCs
* custom items
* custom interfaces
* custom quests
* custom gameplay systems
* multiplayer gameplay

while remaining maintainable and understandable.

---

## Development Philosophy

Development should be:

* milestone-driven
* incremental
* documentation-first when exploring unknown systems
* architecture-driven
* implementation-focused

Prefer:

* working software
* focused issues
* reusable systems
* clear ownership
* documented behavior

Avoid:

* speculative architecture
* duplicate systems
* unnecessary abstraction
* undocumented behavior
* large rewrites

The repository should remain buildable and understandable throughout development.

---

## Core Development Sequence

Eldoria should be developed in dependency order.

The player-facing client is the window into shared systems.
It becomes meaningful only after the shared systems can describe something real.

The core sequence is:

```text
data
    ↓
world
    ↓
game
    ↓
net
    ↓
apps
    ↓
rendered/player-facing experience
```

This does not mean every module must be finished before the next begins.
It means planning should respect ownership and dependency direction.

Examples:

* ElClient should not invent world data that belongs in `world/`.
* ElClient should not decode cache formats that belong in `data/`.
* ElServer should not define packet formats that belong in `net/`.
* ElForge should expose shared systems rather than reimplementing them.

---

## Current Direction

The project is currently focused on building foundational systems that will be reused by all future applications.

Current areas of focus include:

* cache access
* model loading
* texture loading
* rendering foundations
* ElForge inspection workflows
* ElClient application shell
* architecture stabilization
* documentation

The next major direction is expanding `data/` so Eldoria can describe real RuneScape-style world content.

This means moving beyond models and textures toward:

* config archive foundations
* object definitions
* item definitions
* NPC definitions
* map archive discovery
* terrain data
* location/object placement data
* sprites and interface assets later

This data expansion is required before `world/` and ElClient world presentation can become meaningful.

---

## Active Phase Direction

Current phase direction:

```text
Phase 1 - Foundation
Phase 2 - Asset Foundation
Phase 3 - Client Foundation
Phase 4 - Data Expansion
Phase 5 - World Foundation
Phase 6 - Client World Visualization
```

### Phase 3 - Client Foundation

Phase 3 creates the ElClient application shell.

It should establish:

* lifecycle
* screens
* input routing
* render loop foundation
* client state placeholders

It should not try to build the real world.

### Phase 4 - Data Expansion

Phase 4 expands `data/` so cache content can describe real world content.

It should establish:

* config archive reading
* definition loading
* map archive discovery
* terrain decoding
* object placement decoding

### Phase 5 - World Foundation

Phase 5 uses decoded data to build shared spatial representation.

It should establish:

* coordinates
* regions
* tiles
* placed objects
* world snapshots

### Phase 6 - Client World Visualization

Phase 6 connects ElClient to shared `data/`, `world/`, and `render/` systems.

It should make the GameScreen display a real local world snapshot.

---

## Known Future Areas

The following areas are expected to exist in some form as the project grows.

These are not fixed phases.
They are expected areas of development.

### Data Systems

Examples:

* models
* textures
* animations
* maps
* config archives
* object definitions
* item definitions
* NPC definitions
* interface definitions
* sprites

---

### World Systems

Examples:

* coordinates
* maps
* regions
* tiles
* placed objects
* entities
* spatial queries
* visibility
* synchronization-ready snapshots

---

### Rendering

Examples:

* software rendering
* materials
* animation rendering
* world rendering
* UI rendering
* debug rendering

---

### ElForge

Examples:

* inspectors
* editors
* validation tools
* content workflows
* world tools

ElForge should expose shared systems to developers.
It should not own the reusable systems themselves.

---

### ElClient

Examples:

* screens
* login
* loading flow
* local/offline debug flow
* networking
* world presentation
* gameplay presentation

ElClient should be treated as the player-facing window into shared data, world, render, net, and game systems.

---

### ElServer

Examples:

* sessions
* networking
* persistence
* authoritative world simulation
* gameplay authority
* administration

---

### Gameplay Systems

Examples:

* movement
* inventories
* equipment
* combat
* skills
* quests

---

### Content Creation

Examples:

* NPC workflows
* item workflows
* map workflows
* interface workflows
* animation workflows

---

### Multiplayer

Examples:

* login
* synchronization
* visibility
* player interaction

---

### Production Operation

Examples:

* deployment
* monitoring
* administration
* maintenance tooling

---

## Planning Workflow

Future development should follow:

```text
Roadmap
    ↓
Phase Blueprint
    ↓
Repository Inspection
    ↓
Gap Analysis
    ↓
GitHub Issues
    ↓
Implementation
    ↓
Review
    ↓
Documentation Updates
```

The roadmap provides direction.

Phase blueprints provide planning.

Issues provide execution.

---

## Success Criteria

Eldoria is successful when:

* ElForge is a complete content tool
* ElClient is a complete game client
* ElServer is a complete authoritative server
* shared data/world/game/net/render systems support all three applications
* content can be created through Eldoria tooling
* players can connect and play together
* future systems can be added without major architectural rewrites

---

## Golden Rule

The roadmap defines where Eldoria is going.

Phase blueprints define how to get there.

The exact path is allowed to evolve as the project grows.
