# Eldoria

A C++ RuneScape-317 private server ecosystem. Purpose-built as a complete foundation for building and running custom RuneScape-like worlds.

**Status: Foundation Phase** — Data loading and parsing infrastructure complete. Multi-application architecture established. Ready for gameplay systems.

---

## What You're Building

Eldoria is three tightly integrated C++ applications that share a unified codebase:

- **ElForge** — Inspection and content creation tool for game assets and definitions
- **ElClient** — Player-facing game client (C++, SDL3)  
- **ElServer** — Authoritative gameplay server

All three use the same data, world, networking, and rendering modules, eliminating duplication and keeping them synchronized.

---

## What's Done

### Data Layer (Complete)
- **Cache system** — Full RS-317 cache reader supporting all indices (config, models, animations, MIDI, maps)
- **Asset parsing** — Models, animations, textures, fonts, interfaces, sprites
- **Definition tables** — Items, NPCs, floors, locations, sequences, spot animations, varps, varbits, parameters, messages
- **Binary utilities** — Byte readers/writers with smart-int support, compression (BZIP2, ZLIB), JPEG decoding
- **Index entry validation** — Sector chain following, file integrity checks

### Application Skeletons (Complete)
- Three standalone executables that build, launch, and shut down cleanly
- CMake multi-application structure with shared module linking
- SDL3 windowing (ElClient)

### Shared Modules (In Progress)
```
data/      → Binary, cache, archive, definition, texture, model, image parsing
world/     → Entity placement, navigation, coordinate systems
game/      → Gameplay rules and mechanics
net/       → Client-server communication
render/    → Software rasterization, texture sampling
graphics/  → Texture and graphics abstractions
math/      → Vector/matrix utilities
platform/  → OS integration
```

---

## Architecture

```
ElForge ─┐
ElClient├─→ [data, world, game, net, render, graphics, math, platform]
ElServer ┘
```

Each application links the shared modules it needs. The data module is fully functional and can load, parse, and validate any RS-317 cache index. The other modules are placeholders awaiting gameplay implementation.

---

## Build

```bash
# Configure
cmake -B build

# Build all
cmake --build build

# Or one app
cmake --build build --target elforge
cmake --build build --target elclient
cmake --build build --target elserver

# Run
./build/bin/elforge
./build/bin/elclient
./build/bin/elserver
```

Expected output:
```
ElForge starting...
ElForge shutdown.

ElClient starting...
ElClient screen: startup
ElClient shutdown.

ElServer starting...
ElServer run loop tick.
ElServer shutdown.
```

---

## Next Priorities

1. **World foundation** — Tile grid, entity spawning, coordinate validation
2. **Login flow** — ElClient → ElServer authentication and session handshake
3. **Movement validation** — Server-authoritative pathfinding and collision
4. **Rendering** — Convert cache models to drawable geometry, map rendering
5. **Message protocol** — Structured packet definitions between client and server

---

## Key Dependencies

- **C++20** — Modern standard library features
- **CMake 3.20+** — Multi-app build orchestration
- **SDL3** — Window and input (ElClient)
- **ZLIB, BZIP2, JPEG** — Compression and image codecs

---

## Module Responsibilities

| Module | Purpose |
|--------|---------|
| **data** | Load and parse RS-317 caches, assets, definitions |
| **world** | Entity state, navigation, spatial queries |
| **game** | Gameplay logic, rule enforcement |
| **net** | Message framing, serialization, client-server protocol |
| **render** | Rasterization, texture sampling, camera projection |
| **graphics** | Texture management, shader abstraction |
| **math** | Vectors, matrices, transforms |
| **platform** | File I/O, threading, system integration |

---

## Development

- Feature branches from `dev`, merged via pull request
- Issues track small implementation tasks
- Milestone-driven development focused on completing each application phase before moving on
