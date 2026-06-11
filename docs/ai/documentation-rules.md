# Documentation Rules

## Purpose

This document defines when documentation should be updated during Eldoria development.

Documentation is part of the architecture.

If future developers or AI agents need to understand a change before safely modifying the repository, that change should usually be documented.

---

## Core Rule

Documentation should change when repository knowledge changes.

Do not let code and documentation drift apart.

---

## Documentation Ownership

The Architect owns documentation quality.

Implementation agents should not be expected to decide documentation strategy by default.

When implementation work creates new knowledge, the Architect should decide whether a focused documentation issue or documentation pull request is required.

---

## When To Update Documentation

Update documentation when a change introduces:

* a new system
* a new module
* a new application workflow
* a new ownership boundary
* a new architectural pattern
* a new development workflow
* a significant behavior change
* a discovered file format
* a new debugging or validation workflow

---

## What To Update

### Architecture Changes

Update:

```text
docs/architecture.md
```

when the change affects:

* top-level structure
* dependency rules
* documentation structure
* ownership model
* architecture rules

---

### Module Ownership Changes

Update:

```text
docs/modules/
```

when the change affects:

* what belongs in a module
* what does not belong in a module
* dependency rules
* future direction of a module

Example:

```text
New animation loader
    → docs/modules/data.md
```

---

### System Behavior Changes

Update:

```text
docs/systems/
```

when the change affects:

* how a pipeline works
* how data flows
* important types
* important files
* extension points
* verification steps

Example:

```text
New animation loading pipeline
    → docs/systems/animation-loading.md
```

---

### Application Workflow Changes

Update:

```text
docs/apps/
```

when the change affects:

* app responsibilities
* app workflows
* app state
* app UI structure
* app integration with shared systems

Example:

```text
New ElForge animation inspector
    → docs/apps/elforge.md
```

---

### AI Workflow Changes

Update:

```text
docs/ai/
```

when the change affects:

* issue creation
* prompt writing
* PR review
* agent rules
* Architect workflow
* documentation rules

---

### Roadmap Or Phase Changes

Update:

```text
docs/roadmap/
```

when the change affects:

* project direction
* active phase planning
* phase blueprint
* expected issue breakdown
* phase exit criteria

---

## Documentation Issue Rule

Implementation issues should stay focused.

Do not automatically bundle documentation updates into implementation issues unless documentation is the primary objective or the update is tiny and directly tied to the change.

Preferred workflow:

```text
Implementation PR
    ↓
Architect Review
    ↓
Documentation Assessment
    ↓
Documentation Issue if needed
    ↓
Documentation PR
```

---

## Documentation Drift

Documentation drift occurs when:

```text
Repository
    ≠
Documentation
```

When drift is discovered:

1. Identify the mismatch.
2. Determine whether code or documentation is correct.
3. Update the incorrect side.
4. Avoid leaving known drift unresolved.

---

## Review Checklist

During PR review, ask:

```text
Did this change introduce knowledge that future developers or AI agents should know?
```

If yes, determine:

* which document should change
* whether the change belongs in the current PR
* whether a dedicated documentation issue is required

---

## Golden Rule

If a future agent would need to know it before safely changing the code, document it.
