# Phase 1 - Foundation

## Purpose

Phase 1 establishes the foundation required for Eldoria to grow without turning into a messy single-purpose experiment.

The goal is not gameplay.

The goal is to create a stable multi-application C++ workspace with clear architecture, clear ownership, and enough documentation for future work to be planned safely.

---

## Outcome

Phase 1 is complete when Eldoria has:

* a working repository structure
* a buildable C++ workspace
* three application skeletons
* shared module boundaries
* architecture documentation
* development workflow documentation
* AI-friendly planning rules

This phase creates the container for the project.

Later phases fill it with real systems.

---

## Application Foundation

Eldoria must define three top-level applications:

```text
ElForge
ElClient
ElServer
```

### ElForge

Purpose:

The internal development, inspection, debugging, editing, and content creation tool.

Phase 1 requirement:

* application skeleton exists
* application builds
* application launches
* application exits cleanly

### ElClient

Purpose:

The player-facing game client.

Phase 1 requirement:

* application skeleton exists
* application builds
* application launches
* application exits cleanly

### ElServer

Purpose:

The authoritative game server.

Phase 1 requirement:

* application skeleton exists
* application builds
* application launches
* application exits cleanly

---

## Source Layout Foundation

Phase 1 establishes the core source layout:

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

Each top-level folder must have a clear responsibility.

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

## Required Module Foundation

Phase 1 does not need every module to be deeply implemented.

It must establish where future code belongs.

Required module ownership documents:

```text
docs/modules/data.md
docs/modules/world.md
docs/modules/game.md
docs/modules/net.md
docs/modules/render.md
docs/modules/platform.md
```

These documents should explain:

* purpose
* current state
* future direction
* what belongs there
* what does not belong there
* dependency rules
* common mistakes

---

## Required Architecture Documentation

Phase 1 must include:

```text
docs/architecture.md
```

This document is the entry point for understanding the repository architecture.

It should explain:

* project structure
* documentation structure
* application layer
* module layer
* system layer
* AI workflow layer
* roadmap layer
* dependency rules
* architecture change rules

---

## Required AI Workflow Documentation

Phase 1 must include AI-agent-friendly workflow documentation.

Required files:

```text
docs/ai/architect.md
docs/ai/issue-philosophy.md
docs/ai/prompt-philosophy.md
docs/ai/review-philosophy.md
docs/ai/agent-rules.md
docs/ai/implementation-workflow.md
```

These documents define:

* how the Architect works
* how issues are created
* how agent prompts are written
* how PRs are reviewed
* how implementation agents should behave
* how documentation drift is prevented

The goal is to make future AI-assisted development safer and more predictable.

---

## Required Development Workflow

Phase 1 must include:

```text
docs/development.md
```

This document should explain:

* branch workflow
* pull request workflow
* build workflow
* review workflow
* documentation update expectations

---

## Required Roadmap Foundation

Phase 1 must include:

```text
docs/roadmap/roadmap.md
docs/roadmap/phases/
```

The roadmap should define long-term direction without pretending the full future is known.

Phase blueprint files should be created only when they are useful for active or near-term development.

---

## Build Foundation

Phase 1 must establish a buildable C++ workspace.

Required:

* root CMake project
* application targets
* shared module targets
* consistent output location
* clean configure/build workflow

Expected build commands:

```bash
cmake -B build
cmake --build build
```

Expected application targets:

```bash
cmake --build build --target elforge
cmake --build build --target elclient
cmake --build build --target elserver
```

---

## Not Included

Phase 1 does not require:

* complete cache loading
* complete model loading
* complete texture loading
* real rendering pipeline
* gameplay
* networking protocol
* world simulation
* content editing
* multiplayer

Those belong to later phases.

A renderer module may exist in Phase 1 as a placeholder or skeleton.

The real asset-driven rendering pipeline belongs to Phase 2.

---

## Exit Criteria

Phase 1 is complete when:

* repository structure is established
* CMake workspace builds
* ElForge builds and launches
* ElClient builds and launches
* ElServer builds and launches
* architecture documentation exists
* module documentation exists
* AI workflow documentation exists
* development workflow documentation exists
* roadmap foundation exists

---

## Issue Breakdown Strategy

Phase 1 issues should be small and structural.

Good issue categories:

```text
workspace setup
application skeletons
module skeletons
architecture documentation
AI workflow documentation
development workflow documentation
roadmap foundation
```

Avoid combining unrelated concerns.

Example good issue:

```text
Create ElClient, ElServer, and ElForge skeleton targets.
```

Example bad issue:

```text
Create application skeletons, implement model loading, and build renderer.
```

---

## Golden Rule

Phase 1 creates the structure.

It does not build the product.
