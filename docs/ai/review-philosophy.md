# Review Philosophy

## Purpose

This document defines how pull requests should be reviewed within Eldoria.

The purpose of review is not only to verify correctness.

The purpose of review is to preserve architecture, ownership, maintainability, and project direction.

A pull request can be technically correct while still being architecturally wrong.

---

## Core Principle

Implementation agents optimize for completing a task.

The Architect optimizes for the long-term health of the repository.

Review exists to verify that both goals were achieved.

---

## Review Order

Review pull requests in the following order:

1. Scope
2. Ownership
3. Architecture
4. Correctness
5. Simplicity
6. Documentation

Do not start by reading code.

Start by understanding what the pull request was supposed to accomplish.

---

## Step 1: Verify Scope

Ask:

* Which issue is this PR solving?
* What was the goal?
* What files were expected to change?

Look for:

* unrelated changes
* unrelated cleanup
* opportunistic refactors
* architecture changes outside scope

A pull request should remain focused on its issue.

---

## Step 2: Verify Ownership

Ask:

* Does this code belong here?
* Which module owns this responsibility?
* Is ownership becoming clearer or more confusing?

Examples:

Good:

```text
Model loading logic remains in data/model.
```

Bad:

```text
Model loading logic moved into ElForge UI code.
```

Good:

```text
Rendering behavior remains in render.
```

Bad:

```text
Rendering behavior moved into application code.
```

Ownership matters more than implementation style.

---

## Step 3: Verify Architecture

Ask:

* Does the change respect module boundaries?
* Does it create duplicate systems?
* Does it create parallel implementations?
* Does it increase architectural debt?

Look for:

* unnecessary abstractions
* duplicate classes
* temporary systems that become permanent
* ownership leaks

Architecture should become clearer after a merge.

---

## Step 4: Verify Correctness

Ask:

* Does it solve the issue?
* Does it compile?
* Does it behave correctly?

Review:

* implementation
* edge cases
* failure paths
* verification results

Correctness is required but not sufficient.

---

## Step 5: Verify Simplicity

Ask:

* Is there a simpler solution?
* Was existing code reused?
* Was new complexity introduced unnecessarily?

Prefer:

```text
extend existing system
```

over:

```text
create new system
```

Prefer:

```text
reuse existing ownership
```

over:

```text
introduce another ownership layer
```

---

## Step 6: Verify Documentation

Ask:

* Does documentation still match reality?
* Were new concepts introduced?
* Do ownership documents require updates?

Documentation should remain aligned with the repository.

---

## Red Flags

Immediately investigate:

### Duplicate Systems

Example:

```text
ModelLoader
ModelLoader2
```

### Parallel Implementations

Example:

```text
old rendering path
new rendering path
third rendering path
```

### Ownership Violations

Example:

```text
ElForge directly decodes models.
```

### Scope Expansion

Example:

```text
Issue:
Fix cache tree selection.

PR:
Fix cache tree selection.
Refactor viewport.
Rename render types.
Update model loading.
```

### Speculative Architecture

Example:

```text
Future-proof abstraction added without a current need.
```

---

## Review Questions

For every pull request:

1. Does it solve the issue?
2. Does the code belong where it was added?
3. Does an equivalent system already exist?
4. Is the architecture clearer afterward?
5. Would a future developer understand the change?
6. Is the solution smaller than the problem?

If any answer is no, investigate further.

---

## Approval Criteria

A pull request should generally be approved when:

* issue goal is achieved
* ownership is correct
* architecture is preserved
* verification was performed
* no unnecessary complexity was introduced
* documentation remains accurate

Do not approve code solely because it works.

Correct ownership matters.

---

## Golden Rule

The goal of review is not to find code that compiles.

The goal of review is to preserve the long-term quality of Eldoria.
