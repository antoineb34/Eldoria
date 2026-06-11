# Prompt Philosophy

## Purpose

This document defines how implementation prompts should be written for AI agents working on Eldoria.

The goal is to maximize implementation quality while minimizing architectural mistakes.

Implementation agents perform best when uncertainty is reduced.

A prompt should remove as much decision making as possible.

---

## Core Principle

The Architect performs the thinking.

The Prompt communicates the plan.

The Agent performs the implementation.

A prompt should guide execution.

A prompt should not require the agent to discover architecture.

---

## Prompt Goal

A good prompt should answer:

* What should be built?
* Why does it exist?
* Where does it belong?
* What may change?
* What must not change?
* How is success verified?

The agent should not need to determine these answers independently.

---

## Reduce Freedom

Implementation agents should have limited freedom.

Prefer:

```text
Implement model caching in ModelLoader.
```

Over:

```text
Improve model loading.
```

Prefer:

```text
Use the existing ModelLoader.
```

Over:

```text
Choose the best approach.
```

Reduce interpretation.

Increase clarity.

---

## Prompt Structure

Every implementation prompt should contain:

### Goal

The objective.

### Context

Why the task exists.

### Relevant Documentation

Architecture or system documents.

### Files To Inspect

Specific files and directories.

### Scope

Allowed changes.

### Non-Goals

Forbidden changes.

### Architecture Constraints

Ownership rules.

### Implementation Approach

Recommended implementation direction.

### Verification

Build and validation steps.

### Deliverables

Expected outcome.

---

## Goal Section

The goal should be concise.

Good:

```text
Add model caching to ModelLoader.
```

Bad:

```text
Improve model loading.
```

Bad:

```text
Make the model system better.
```

The agent should immediately know what success looks like.

---

## Context Section

The context explains why the task exists.

The context should not contain implementation decisions.

Good:

```text
Models are currently decoded every time they are requested.
```

Good:

```text
Repeated requests should reuse loaded models.
```

---

## Files To Inspect

Always identify likely sources of truth.

Examples:

```text
src/data/model/
src/data/cache/
```

```text
src/apps/elforge/panels/cache/
```

The goal is to prevent agents from searching the entire repository unnecessarily.

---

## Scope

Define what may change.

Example:

Allowed:

* ModelLoader
* related model cache structures
* associated tests

This tells the agent where work should occur.

---

## Non-Goals

Define what must not change.

Example:

Do not:

* redesign model loading
* move ownership
* change ElForge UI
* modify renderer architecture

Non-goals are often more important than goals.

---

## Architecture Constraints

Every prompt should reinforce ownership.

Example:

```text
Model loading belongs in data/model.

Do not decode models in ElForge.
```

Example:

```text
Rendering belongs in render.

Do not move rendering logic into application code.
```

The agent should be reminded of boundaries before implementation begins.

---

## Implementation Approach

Whenever possible, provide implementation direction.

Example:

```text
Extend the existing ModelLoader cache.

Do not create a second loader.
```

Example:

```text
Reuse the existing viewport state structure.

Do not introduce a parallel state system.
```

Avoid asking the agent to determine strategy.

The Architect should determine strategy.

---

## Verification

Every prompt should define verification.

Examples:

```text
cmake --build build
```

```text
Open model 147 in ElForge.
```

```text
Verify cache tree still expands correctly.
```

Verification should be defined before implementation begins.

---

## Deliverables

Require a summary.

The agent should report:

* files modified
* reason for each modification
* verification performed
* unresolved concerns

This improves review quality.

---

## Bad Prompts

Avoid:

```text
Improve rendering.
```

```text
Refactor model loading.
```

```text
Clean up ElForge.
```

```text
Make the architecture better.
```

These require architectural decisions during implementation.

---

## Good Prompts

Prefer:

```text
Move viewport settings into ViewportState.
```

```text
Add cache lookup to ModelLoader.
```

```text
Replace duplicated projection code with existing helper.
```

These define a clear outcome.

---

## Golden Rule

A prompt should communicate a plan.

The agent should execute the plan.

The agent should not be responsible for creating the plan.
