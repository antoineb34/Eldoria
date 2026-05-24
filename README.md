# RuneForge

RuneForge is a modern C++ RuneScape 317 cache viewer / tooling project.

The long-term goal is to build a clean and extensible foundation for:
- cache exploration
- asset decoding
- rendering
- tooling/editor workflows
- eventually a custom game client

Current development is focused on:
- cache decoding
- rendering pipeline foundations
- architecture and tooling

---

# Features

Current functionality includes:
- DAT/IDX FileStore reading
- archive extraction
- GZIP decompression
- model decoding experiments
- SDL3 application structure
- tool/viewer foundation

---

# Build

## Requirements

- CMake 3.20+
- C++20 compiler
- SDL3
- ZLIB
- BZip2

## Build Commands

```bash
cmake -B build
cmake --build build
```

---

# Project Structure

```text
src/
  core/      # cache, io, decoding, engine core
  ui/        # shared UI/rendering systems
  apps/
    tool/    # cache viewer/editor tool
    client/  # future game client
```

---

# Current State

See `PROJECT_STATE.md`
