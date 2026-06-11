# Eldoria Roadmap

## Purpose

This document defines the long-term direction of Eldoria.

It explains:

* what Eldoria is
* what Eldoria should become
* the principles guiding development
* the major areas expected to exist in the future

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

## Current Direction

The project is currently focused on building foundational systems that will be reused by all future applications.

Current areas of focus include:

* asset loading
* asset representation
* rendering
* ElForge workflows
* architecture stabilization
* documentation

The exact path forward may evolve as new information is discovered.

---

## Known Future Areas

The following areas are expected to exist in some form as the project grows.

These are not fixed phases.

They are expected areas of development.

### Asset Systems

Examples:

* models
* textures
* animations
* maps
* definitions
* interfaces

---

### Rendering

Examples:

* software rendering
* materials
* animation rendering
* world rendering
* UI rendering

---

### ElForge

Examples:

* inspectors
* editors
* validation tools
* content workflows
* world tools

---

### ElClient

Examples:

* screens
* login
* networking
* world presentation
* gameplay presentation

---

### ElServer

Examples:

* sessions
* networking
* persistence
* world simulation
* administration

---

### World Systems

Examples:

* maps
* regions
* entities
* spatial queries
* synchronization

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
* content can be created through Eldoria tooling
* players can connect and play together
* future systems can be added without major architectural rewrites

---

## Golden Rule

The roadmap defines where Eldoria is going.

Phase blueprints define how to get there.

The exact path is allowed to evolve as the project grows.
