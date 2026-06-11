# Platform Module

## Purpose

`src/platform/` owns machine and library integration.

It answers:

```text
How does Eldoria talk to the machine?
```

The platform module provides reusable wrappers around operating-system and third-party library functionality.

It isolates external dependencies from the rest of the codebase.

---

## Current State

`src/platform/` currently contains:

```text
src/platform/
├── sdl/
└── imgui/
```

Current responsibilities include:

* SDL initialization
* SDL shutdown
* window creation
* renderer creation
* SDL resource ownership
* ImGui integration

The primary implementation today is:

```text
platform/sdl/SdlContext
```

which owns:

* SDL window lifetime
* SDL renderer lifetime

---

## Direction

Future platform systems should also live under `src/platform/`.

Examples:

```text
src/platform/filesystem/
src/platform/input/
src/platform/window/
src/platform/time/
src/platform/audio/
```

If a feature is primarily about interacting with SDL, the operating system, or a third-party platform library, it probably belongs in `platform/`.

Examples:

```text
window management
= platform/window/

filesystem abstraction
= platform/filesystem/

keyboard and mouse integration
= platform/input/

time utilities
= platform/time/
```

Applications should consume platform services.

Applications should not directly own low-level platform integration.

---

## Standard Pattern

Preferred flow:

```text
Operating System
    ↓
Third-Party Library
    ↓
Platform Wrapper
    ↓
Application
```

Example:

```text
Windows/Linux
    ↓
SDL
    ↓
SdlContext
    ↓
ElForge
```

The rest of Eldoria should interact with platform wrappers rather than directly managing external libraries.

---

## Owns

`platform/` owns:

* SDL initialization
* SDL shutdown
* SDL window management
* SDL renderer management
* platform resource lifetime
* ImGui integration
* future filesystem wrappers
* future input wrappers
* future timing wrappers
* future platform services

---

## Does Not Own

`platform/` does not own:

* rendering pipelines
* asset loading
* cache decoding
* gameplay rules
* networking
* world state
* application workflows
* tool behavior
* client behavior
* server behavior

---

## Used By

`platform/` may be used by:

* ElForge
* ElClient
* ElServer

All applications should consume platform services through this module.

---

## Dependency Rules

`platform/` may depend on:

* SDL
* ImGui
* operating-system APIs
* approved third-party platform libraries

`platform/` should not depend on:

* apps
* game
* world
* net
* render
* data

Platform should remain reusable.

---

## Submodule Responsibilities

### `platform/sdl/`

Owns SDL integration.

Examples:

* SDL initialization
* SDL shutdown
* SDL windows
* SDL renderers

---

### `platform/imgui/`

Owns ImGui integration.

Examples:

* ImGui setup
* ImGui platform bindings
* ImGui renderer bindings

Applications should consume ImGui through platform integration rather than duplicating setup logic.

---

## Boundary Examples

```text
platform/sdl/SdlContext
= owns SDL resources

render/RenderPipeline
= performs rendering

apps/elforge
= uses both
```

```text
platform/imgui
= integrates ImGui

apps/elforge
= builds editor panels using ImGui
```

---

## Common Mistakes

Do not:

* put gameplay logic in platform
* put rendering pipelines in platform
* put cache decoding in platform
* put ElForge panel code in platform
* put client screen flow in platform
* put server behavior in platform
* bypass platform wrappers for common library integration

---

## When Adding New Platform Code

Before adding code to `src/platform/`:

1. Verify the feature is platform-related.
2. Check whether SDL or an existing wrapper already owns it.
3. Create reusable wrappers.
4. Avoid application-specific behavior.
5. Keep external library integration isolated here.

Good examples:

```text
platform/filesystem/
platform/input/
platform/time/
```

Bad examples:

```text
platform/model_viewer/
platform/combat/
platform/login_screen/
```

---

## Golden Rule

`platform/` owns machine integration.

It provides services to applications.

It does not own game behavior.
