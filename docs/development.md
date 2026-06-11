# Development Workflow

## Purpose

This document defines the development workflow used by the Eldoria repository.

It explains:

* branch strategy
* planning workflow
* implementation workflow
* commit workflow
* review workflow
* documentation workflow
* project management workflow

Development should remain incremental, predictable, and easy to review.

---

## Planning Workflow

Development should follow:

```text
Roadmap
    ↓
Phase Blueprint
    ↓
Repository Inspection
    ↓
Gap Analysis
    ↓
Focused Issues
    ↓
Implementation
    ↓
Review
    ↓
Documentation Assessment
    ↓
Merge
```

Roadmap documents define direction.

Phase blueprints define what must be built.

GitHub issues define implementation work.

---

## Planning Responsibilities

### Architect

Responsible for:

* repository understanding
* roadmap planning
* phase blueprint creation
* repository inspection
* gap analysis
* issue creation
* prompt creation
* pull request review
* documentation assessment

The Architect owns planning.

---

### Implementation Agents

Responsible for:

* implementing assigned issues
* staying within issue scope
* following architecture documentation
* following module ownership rules
* following system documentation
* opening pull requests

Implementation agents own execution.

Implementation agents should not redesign architecture.

---

## Implementation Agent Workflow

When assigned an issue:

1. Read the issue completely.
2. Read all referenced documentation.
3. Inspect the current implementation.
4. Understand the existing architecture.
5. Implement only the requested scope.
6. Verify the implementation.
7. Stage all changes.
8. Create commit(s).
9. Push the branch.
10. Open a pull request.
11. Stop and wait for review.

Implementation agents should not:

* redesign architecture
* expand issue scope
* start unrelated work
* implement future issues
* continue implementing after opening a pull request

The pull request is the handoff point between the implementation agent and the Architect.

---

## Branch Strategy

### main

* Protected branch
* Stable branch
* Presentable branch
* Receives changes through pull requests only
* Should only receive completed phase or release-ready work from `dev`

### dev

* Active integration branch
* Receives completed feature work
* Source branch for future stable updates to `main`

### feature/*

* One feature branch per issue or tightly related group of issues
* Created from `dev`
* Merged back into `dev` through a pull request

Example branch names:

```text
feature/issue-43-cmake-workspace
feature/issue-44-shared-modules
feature/issue-49-development-workflow
```

For bug fixes or documentation-only changes, use clear names:

```text
bugfix/fix-model-decoder
docs/update-roadmap
```

---

## Development Flow

1. Select a GitHub issue.
2. Move the issue to `Doing` on the project board.
3. Create a feature branch from `dev`.
4. Implement the change.
5. Verify the change.
6. Stage and commit the change.
7. Push the feature branch.
8. Open a pull request into `dev`.
9. Review the pull request.
10. Merge the pull request into `dev` after review.
11. Close the issue.
12. Move the issue to `Done` on the project board.

Example:

```text
Issue #43
    ↓
feature/issue-43-cmake-workspace
    ↓
Pull Request
    ↓
Review
    ↓
dev
    ↓
Close issue
```

Issues are considered complete when their pull request is reviewed and merged into `dev`.

Issues do not need to wait for `dev` to merge into `main` before being closed.

---

## Commit Workflow

### Required Git Workflow

Expected workflow:

```bash
git status
git add .
git status
git commit -m "..."
git push
```

Implementation agents should assume that:

* new files may have been created
* files may have been moved
* documentation may have changed
* CMake files may have changed
* build files may have changed

Missing files is a known failure mode.

The default staging strategy is:

```bash
git add .
```

Do not manually stage individual files unless explicitly required.

---

### Staging Rule

Before creating a commit:

1. Review repository status.
2. Stage the entire working tree.
3. Review staged files.
4. Create the commit.

Expected workflow:

```bash
git status
git add .
git status
git commit -m "..."
```

After staging:

* verify the staged file list
* remove unintended files if necessary
* verify all intended files are included

Do not assume files were staged correctly.

Always verify using:

```bash
git status
```

before committing.

---

### Commit Scope

Commits should represent a coherent piece of work.

Prefer:

```text
Issue #42
    ↓
Implementation
    ↓
Single Commit or Small Commit Series
```

Avoid mixing unrelated changes into the same commit.

---

### Common Failure Modes

Examples:

```text
New source file created but not committed
```

```text
CMake updated but source file not committed
```

```text
Documentation updated but implementation not committed
```

```text
Implementation completed but generated files committed accidentally
```

Review staged files before every commit.

The goal is:

```text
Everything intended
Nothing unintended
```

---

## Review Gate

Implementation work should stop at an open pull request unless the maintainer explicitly asks for more.

A contributor should:

1. Read the relevant docs before coding.
2. Create a branch from `dev`.
3. Keep the change scoped to the issue.
4. Build or verify the change when practical.
5. Push the branch.
6. Open a pull request targeting `dev`.
7. Include a summary, changed files, and verification notes.
8. Wait for review.

A contributor should not:

* commit directly to `main`
* commit directly to `dev`
* merge their own pull request
* close the issue before review
* perform unrelated refactors outside the issue scope

---

### Stop Condition

After opening a pull request:

STOP.

Do not:

* continue implementing additional features
* expand issue scope
* perform opportunistic refactors
* start the next issue

Wait for review.

The pull request is the handoff point between the implementation agent and the Architect.

---

## Documentation Workflow

After reviewing a pull request:

```text
Implementation PR
    ↓
Review
    ↓
Documentation Assessment
    ↓
Documentation Issue (if needed)
    ↓
Documentation PR
```

Documentation should be updated whenever repository knowledge changes.

See:

```text
docs/ai/documentation-rules.md
```

for documentation ownership and update rules.

---

## CMake Workspace Rules

The root `CMakeLists.txt` owns the workspace shape, not every source file.

Use the helper functions in `cmake/EldoriaTargets.cmake` to register modules and applications.

### Adding a Shared Module

A shared module should live under `src/` and should own its own `CMakeLists.txt`.

Example:

```cmake
eldoria_add_module(data src/data)
```

The module's source files should be listed inside `src/data/CMakeLists.txt`, not in the root `CMakeLists.txt`.

---

### Adding an Application

An application should live under `src/apps/` and should own its own `CMakeLists.txt`.

Example:

```cmake
eldoria_add_app(elforge src/apps/elforge)
```

The application's source files should be listed inside `src/apps/elforge/CMakeLists.txt`, not in the root `CMakeLists.txt`.

---

### Ownership Rules

* Root CMake registers modules and applications.
* Module CMake files list module source files and dependencies.
* Application CMake files list application source files and dependencies.
* New modules should match `docs/architecture.md`.
* Avoid one-off build logic in the root CMake file unless it applies to the whole workspace.

---

## Release Flow

When a phase is complete and stable:

```text
dev
    ↓
Pull Request
    ↓
main
```

A `dev` to `main` pull request should represent a stable phase completion or release-ready snapshot.

---

## Project Management

```text
Roadmap
    ↓
Phase Blueprints
    ↓
Issues
    ↓
Project Board
```

Project Board Columns:

* Todo
* Doing
* Blocked
* Done

Roadmap documents define direction.

Phase blueprints define what should be built.

Issues define concrete implementation work.

The project board defines what is actively being worked on.

---

## Rules

Do not commit directly to `main`.

Do not use `main` for active development.

All implementation work should begin from a feature branch created from `dev`.

Use `git add .` as the default staging workflow before commits.

Prefer small pull requests that map clearly to one issue.

Prefer focused issues over large multi-system changes.

Prefer incremental progress over large rewrites.

---

## Golden Rule

The Architect plans.

Implementation agents execute.

Every change should move Eldoria closer to the active phase objective while preserving architecture quality.
