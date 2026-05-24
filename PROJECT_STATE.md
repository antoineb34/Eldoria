# RuneForge Project State

## Current Focus

Building a clean RuneScape 317 cache viewer and rendering foundation in modern C++.

Development is currently focused on:
- rendering pipeline cleanup
- architecture stabilization
- model decoding
- preparing proper renderer abstractions

---

# Current Status

## Working

- DAT/IDX FileStore reading
- archive extraction
- GZIP decompression
- binary buffer utilities
- SDL3 tool application structure
- basic model decoding
- model viewer experiments

## In Progress

- renderer cleanup
- OpenGL rendering foundation
- texture mapping understanding
- separating decoded model data from render-ready mesh data

## Unknown / Research

- exact texture coordinate pipeline
- some textured face behavior
- future GPU abstraction design

---

# Current Architecture

## Pipeline

```text
DAT/IDX
-> FileStore
-> Archive Extraction
-> Decompression
-> Binary Parsing
-> Model Definition
-> CPU Mesh
-> GPU Mesh
-> Renderer
```

## Main Structure

```text
src/
  core/
    io/
    filestore/
    legacy/
  ui/
  apps/
    tool/
    client/
```

---

# Important Discoveries

- textured faces replace face color with texture ids
- current renderer/debug code is temporary and expected to evolve
- tool-first development approach is intentional
- raw cache structures should remain separate from renderer structures
- RuneScape coordinate system differs from OpenGL conventions

---

# Immediate Next Steps

1. stabilize renderer structure
2. clean model viewer pipeline
3. define CPU mesh structure
4. define GPU mesh abstraction
5. continue texture mapping investigation

---

# Session Handoff

Before making major architecture decisions:
- inspect the current pipeline first
- avoid premature abstractions
- renderer architecture is still evolving rapidly
- prioritize clear data flow over complex inheritance
