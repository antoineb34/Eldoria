# Eldoria Architecture

## Purpose

This document is the entry point for understanding Eldoria's architecture.

It provides the high-level structure of the project and points to the documents that define ownership, responsibilities, workflows, and implementation rules.

This document should remain concise.

Detailed implementation and ownership rules belong in focused documents.

---

## Project Structure

Eldoria is a custom RuneScape-inspired private server ecosystem written in C++.

The project consists of three applications:

* ElForge
* ElClient
* ElServer

Shared modules provide reusable systems used by those applications.

---

## Source Layout

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
= what things are and how static content is loaded

world
= where things are and how spatial state is represented

game
= what rules apply

net
= how state and actions travel

render
= how things become pixels

platform
= how Eldoria talks to the machine
```

This mental model should guide ownership decisions throughout the repository.

---

## Dependency Direction

Development should respect the natural dependency direction of the project.

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
```

Rendering is a presentation system used by tools and the client:

```text
data/world/app presentation input
    ↓
render
    ↓
pixels
```

This does not mean every lower module must be complete before higher modules can exist.

It means higher modules should not duplicate or own responsibilities from lower modules.

Examples:

* `data/` loads static content.
* `world/` represents spatial reality using data.
* `game/` defines rules that apply to world state.
* `net/` defines protocol language used to move state/actions.
* `apps/` compose shared systems into products.
* `render/` converts renderable scene input into pixels.

---

## Product Roles

### ElForge

ElForge is the developer-facing tool.

It inspects, validates, debugs, and eventually edits or creates content through shared systems.

It does not own reusable data, world, game, net, render, or platform systems.

### ElClient

ElClient is the player-facing window into Eldoria.

It presents world/game state and sends player intent.

It does not own cache decoding, world authority, game rules, or server truth.

### ElServer

ElServer is the authoritative online runtime.

It owns online truth by composing shared data, world, game, and net systems into server behavior.

It does not own reusable module definitions just because it is authoritative.

---

## Documentation Structure

The documentation is organized into layers.

Each layer answers a different question.

---

### Architecture

Defines the high-level structure of the repository.

Answers:

* What are the major parts of Eldoria?
* How do they relate to each other?
* What ownership boundaries exist?

---

### Modules

Define ownership.

Answer:

* Where does code belong?
* Which module owns a responsibility?
* What does not belong in this module?

Examples:

```text
data
world
game
net
render
platform
```

Modules define ownership.

They do not describe implementation details.

---

### Systems

Define behavior.

Answer:

* How does a feature work?
* How does data flow through the implementation?
* Which components participate?

Examples:

```text
cache loading
model loading
texture loading
map loading
rendering
```

Systems often resemble pipelines.

Systems define implementation.

They do not define ownership.

---

### Applications

Define product responsibilities.

Answer:

* What is ElForge responsible for?
* What is ElClient responsible for?
* What is ElServer responsible for?

Applications compose modules and systems into runnable products.

---

### AI

Defines the development workflow.

Answer:

* How are issues created?
* How are prompts written?
* How are pull requests reviewed?
* How should implementation agents behave?

The AI documents define how development work is performed.

---

### Roadmap

Defines future work.

Answer:

* Where is Eldoria going?
* Which milestones exist?
* Which systems still need to be built?

The roadmap defines direction.

It does not define implementation.

---

## Application Layer

Applications compose shared modules into runnable products.

Applications may depend on shared modules.

Shared modules must not depend on applications.

For application responsibilities:

* `docs/apps/elforge.md`
* `docs/apps/elclient.md`
* `docs/apps/elserver.md`

---

## Module Layer

Each shared module has a dedicated ownership document.

Read the relevant module document before modifying code in that area.

* `docs/modules/data.md`
* `docs/modules/world.md`
* `docs/modules/game.md`
* `docs/modules/net.md`
* `docs/modules/render.md`
* `docs/modules/platform.md`

Module documents define:

* purpose
* current state
* future direction
* ownership
* boundaries
* dependencies
* common mistakes

---

## System Layer

System documents explain how important systems currently work.

Read these before modifying an existing system.

Examples:

* `docs/systems/cache.md`
* `docs/systems/model-loading.md`
* `docs/systems/texture-loading.md`
* `docs/systems/rendering.md`
* `docs/systems/elforge-viewport.md`

System documents define:

* purpose
* current implementation
* data flow
* important types
* important files
* extension points

Module documents describe ownership.

System documents describe implementation.

---

## AI Workflow

AI-assisted development follows the workflow defined in:

* `docs/ai/architect.md`
* `docs/ai/issue-philosophy.md`
* `docs/ai/prompt-philosophy.md`
* `docs/ai/review-philosophy.md`
* `docs/ai/agent-rules.md`
* `docs/ai/implementation-workflow.md`

These documents define how work moves from milestone to merged code.

---

## Development Workflow

Development workflow, branching strategy, and project process are defined in:

* `docs/development.md`

Roadmap and milestone planning are defined in:

* `docs/roadmap/roadmap.md`
* `docs/roadmap/milestones.md`

---

## Dependency Rule

Applications may depend on shared modules.

Shared modules should not depend on applications.

Example:

```text
apps/elforge
    depends on render
        depends on data
```

Not:

```text
data
    depends on apps/elforge
```

Shared modules should remain reusable.

Applications compose those modules into products.

---

## Architecture Changes

Architecture changes should be intentional.

Do not introduce:

* new top-level modules
* new ownership boundaries
* duplicate systems
* parallel implementations

unless the change has been explicitly planned and approved.

When architecture appears insufficient:

1. Identify the problem.
2. Explain the limitation.
3. Propose the change.
4. Review the proposal before implementation.

---

## Golden Rule

Architecture defines ownership.

System documents define implementation.

Application documents define product responsibilities.

Roadmap documents define sequencing.
