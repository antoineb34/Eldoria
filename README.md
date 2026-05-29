# RuneForge

RuneForge is a modern C++ RuneScape 317 cache exploration, asset decoding, and rendering project.

The goal is to build a clean, maintainable, and extensible foundation for understanding RuneScape's cache formats while providing tooling, rendering systems, and eventually a custom game client.

Development follows a tool-first approach: build tooling to understand the data, then use that knowledge to build the client.

---

# Current Goals

* Cache exploration and research
* Asset decoding
* Software rendering
* Tooling and editor workflows
* Rendering architecture
* Future game client development

---

# Features

Current functionality includes:

## Cache System

* DAT/IDX FileStore reading
* Archive extraction
* GZIP decompression
* Binary parsing utilities

## Asset Decoding

* Model loading
* Vertex decoding
* Face decoding
* Texture UV mapping decoding
* Texture loading
* Texture canvas decoding
* Texture pixel decoding

## Rendering

* Software rasterizer
* Model rendering
* Texture mapping
* Backface culling
* Priority-based face rendering
* RuneScape-style transparency handling
* Interactive model viewer

## Tooling

* SDL3 application framework
* Cache browser
* Cache tree explorer
* Asset inspection panels
* Model preview system

---

# Build

## Requirements

* CMake 3.20+
* C++20 compiler
* SDL3
* ZLIB
* BZip2

## Build

```bash
cmake -B build
cmake --build build
```

---

# Project Structure

```text
src/

├── apps/
│   ├── tool/
│   └── client/
│
├── core/
│   ├── assets/
│   │   ├── model/
│   │   └── texture/
│   │
│   ├── cache/
│   ├── compression/
│   ├── io/
│   └── references/
│
├── render/
│   ├── model/
│   └── software/
│
├── platform/
│   └── sdl/
│
└── ui/
```

---

# Development Philosophy

RuneForge favors:

* clear data flow
* explicit decoding steps
* minimal abstractions
* tool-first development
* renderer-independent asset formats
* separation between cache structures and runtime structures

The project intentionally prioritizes understanding the original RuneScape formats before introducing engine-level abstractions.

---

# Current Status

The cache pipeline, model decoding pipeline, texture decoding pipeline, and software rendering foundation are operational.

Current research areas include:

* map decoding
* animation decoding
* scene rendering
* GPU rendering architecture
* client architecture
* editor tooling

---

# License

Work in progress.
