# Issue Template

## Purpose

This document defines the preferred structure for GitHub issues created by the Architect.

Issues should be focused enough that an implementation agent can execute them without inventing architecture.

The Architect owns the thinking.

The implementation agent owns the execution.

---

# Issue Title

Use a clear action-oriented title.

Good:

```text
Add AnimationFileReader skeleton
```

Bad:

```text
Animations
```

---

# Goal

Explain the single objective of the issue.

The goal should be narrow.

Example:

```text
Create the initial `src/data/animation/` file reader skeleton so future animation decoding work has a clear home.
```

---

# Context

Explain why this work exists.

Include relevant roadmap, phase, module, system, or app documents.

Example:

```text
This belongs to Phase 2 - Asset Foundation.

Relevant docs:

- docs/modules/data.md
- docs/roadmap/phases/phase-2-asset-foundation.md
```

---

# Current State

Describe what exists now.

This should be based on repository inspection.

Example:

```text
The repo currently has model and texture loading systems, but no `src/data/animation/` module yet.
```

---

# Desired State

Describe what should exist after the issue is complete.

Example:

```text
`src/data/animation/` exists with initial file types and a reader skeleton that follows the existing data loading architecture.
```

---

# Required Changes

List the expected changes.

Example:

```text
- Create `src/data/animation/`
- Add `AnimationFile.h`
- Add `AnimationFileReader.h`
- Add `AnimationFileReader.cpp`
- Wire files into the data CMake target if required
```

---

# Architecture Constraints

State ownership and boundary rules.

Example:

```text
- Animation loading belongs in `src/data/animation/`
- Do not add ElForge UI in this issue
- Do not add rendering support in this issue
- Follow the reader / decoder / builder / asset / loader pattern
```

---

# Out Of Scope

Explicitly say what not to do.

Example:

```text
Out of scope:

- animation preview
- animation rendering
- ElForge animation inspector
- gameplay animation state machine
```

---

# Verification

Define how to verify the issue.

Example:

```bash
cmake --build build
```

Add manual checks when needed.

Example:

```text
Verify existing model and texture loading still works.
```

---

# Documentation Updates

Say whether docs are required.

Example:

```text
No documentation update required for this issue unless the implementation changes the planned architecture.
```

or:

```text
Update `docs/modules/data.md` if the animation module ownership changes.
```

---

# Acceptance Criteria

Use concrete completion criteria.

Example:

```text
- `src/data/animation/` exists
- animation reader skeleton compiles
- no ElForge/render/game logic is added
- existing build succeeds
```

---

## Golden Rule

A good issue should make the correct implementation obvious.

If the agent still has to invent architecture, the issue is not specific enough.
