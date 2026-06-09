# Eldoria

Eldoria is a custom RuneScape-317-feeling private server ecosystem built in C++.

The project is not only a cache viewer, a client remake, or a server experiment. The long-term goal is a complete ecosystem for building and running a custom RuneScape-like world.

Eldoria is composed of three main applications:

- **ElForge** — the development, inspection, debugging, editing, and content creation tool.
- **ElClient** — the player-facing C++ game client.
- **ElServer** — the authoritative C++ game server.

Shared modules provide reusable systems for data loading, world representation, gameplay rules, networking, rendering, and platform integration.

---

## Project Goals

Eldoria should eventually support:

- RuneScape-317-style visuals and feel
- a custom C++ game client
- an authoritative C++ server
- internal tooling for inspecting and editing content
- custom models, maps, items, NPCs, bosses, interfaces, and gameplay
- offline client/debug workflows
- server-authoritative online play
- content created or validated through ElForge

The goal is not to stay frozen as a pure 317 clone forever.

The goal is to use the RuneScape-317 style and data concepts as a foundation for a custom private server ecosystem.

---

## Applications

### ElForge

ElForge is the internal tool used to understand and create Eldoria content.

It starts as a viewer and inspector for cache data, models, textures, maps, animations, interfaces, and definitions.

Long-term, ElForge should become the tool used to edit, validate, save, export, and maintain custom Eldoria content.

### ElClient

ElClient is the player-facing game client.

It should feel recognizable as a RuneScape-style client while using a cleaner modern C++ architecture.

ElClient presents the world, handles player input, renders interfaces, connects to ElServer, and displays gameplay controlled by the server.

### ElServer

ElServer is the authoritative game server.

It owns truth.

Clients request actions. The server validates and applies them.

ElServer is responsible for login, sessions, world state, movement validation, entity synchronization, persistence, gameplay rules, admin tools, and production server operation.

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

### Core Mental Model

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

## Documentation

Detailed project planning lives in `docs/`.

- [`docs/architecture.md`](docs/architecture.md) — where code belongs and which module owns which responsibility
- [`docs/roadmap.md`](docs/roadmap.md) — master product roadmap
- [`docs/elforge.md`](docs/elforge.md) — ElForge roadmap
- [`docs/elclient.md`](docs/elclient.md) — ElClient roadmap
- [`docs/elserver.md`](docs/elserver.md) — ElServer roadmap

---

## Current Development Approach

Development is milestone-driven.

The current focus is to turn the architecture into a working multi-application foundation:

1. Planning baseline
2. App skeletons
3. ElForge asset foundation
4. Client login loop
5. Map and world foundation

Small implementation tasks are tracked as GitHub Issues.

---

## Build

Build instructions will evolve as the multi-app workspace is implemented.

Current expected baseline:

```bash
cmake -B build
cmake --build build
```

Requirements will be documented as the application skeletons and shared modules become stable.
