# Agent Rules

## Purpose

This document defines the rules that implementation agents must follow when working on Eldoria.

Implementation agents are responsible for executing a plan.

Implementation agents are not responsible for architecture, ownership decisions, milestone planning, or repository direction.

When uncertain, stop and report the uncertainty.

Do not invent solutions.

---

## Core Principle

The Architect performs the thinking.

The Agent performs the implementation.

If an architectural decision is required, the issue, prompt, or documentation should already define it.

If it does not, stop and ask for clarification.

Do not create architecture during implementation.

---

## Read Before Coding

Before making changes:

1. Read the issue.
2. Read the implementation prompt.
3. Read relevant documentation.
4. Inspect the referenced source files.
5. Understand the current implementation.

Do not start coding before understanding the existing system.

## Feature Branch Gate

Before modifying any files:

```bash
git checkout dev
git pull
git checkout -b feature/<issue-number>-<short-name>
git branch --show-current
```

Verify the reported branch matches the assigned issue.

Example:

```text
Issue #118
feature/118-screen-management
```

Do not modify repository files until branch verification has been completed.

Working directly on:

```text
dev
main
```

is prohibited.

---

## GitHub Issue Verification

GitHub Issues are the implementation source of truth.

Before implementation:

```bash
gh issue view <issue-number>
```

Verify:

* scope
* requirements
* acceptance criteria

Do not rely solely on memory or previous discussions.

---

## Placeholder Rule

Do not implement future systems to verify architecture.

If a concrete implementation is required for testing:

Prefer:

```text
PlaceholderScreen
TestRenderer
DummyProvider
```

Avoid:

```text
LoginScreen
GameScreen
InventorySystem
NetworkClient
```

unless the issue explicitly requires those systems.

Implementation agents should not introduce future product features simply to validate current architecture.

---

## Completion Verification

Before reporting completion:

Verify:

```bash
git status
git branch --show-current
git log --oneline --decorate -3
```

Include these results in the final report.

Do not report implementation complete unless:

* changes are committed
* changes are pushed
* branch is correct
* verification has been performed

---

## Scope Enforcement

Before opening a pull request:

Verify:

* only issue requirements were implemented
* no future issue work was added
* no unrelated systems were introduced
* no speculative architecture was created

If implementation exceeds issue scope:

```text
Remove the additional work
or
Create a separate issue
```

Pull requests should map cleanly to a single issue whenever possible.

---

## Pull Request Discipline

After opening a pull request:

STOP.

Do not:

* continue implementing
* start the next issue
* expand scope
* add opportunistic features
* perform unrelated refactors

Wait for Architect review.

The pull request is the handoff point between the implementation agent and the Architect.


---

## Scope Discipline

Only perform work required by the issue.

Do not:

* expand scope
* add unrelated features
* perform unrelated cleanup
* rename unrelated files
* reformat unrelated code
* update unrelated documentation

Stay focused on the assigned objective.

---

## Ownership Rules

Respect existing ownership boundaries.

Before adding code, determine:

* which module owns the behavior
* which system owns the responsibility
* whether an existing implementation already exists

Prefer extending existing systems.

Avoid creating new ownership boundaries.

---

## Existing Systems First

Always look for an existing solution before creating a new one.

Prefer:

```text
extend existing system
```

over:

```text
create replacement system
```

Prefer:

```text
reuse existing ownership
```

over:

```text
create parallel ownership
```

Avoid duplicate systems.

---

## Architecture Restrictions

Do not:

* create new top-level modules
* create new ownership boundaries
* redesign architecture
* restructure large systems
* move responsibilities between modules
* create parallel implementations

unless the issue explicitly requires it.

Architectural decisions belong to the Architect.

---

## Refactoring Restrictions

Do not perform refactors unless:

* the issue explicitly requires it
* the implementation cannot be completed otherwise

Refactoring is not a default activity.

Working code should not be rewritten without justification.

---

## Documentation Restrictions

Do not update documentation unless:

* the issue explicitly requires documentation updates
* the implementation changes documented behavior

Avoid speculative documentation changes.

---

## Naming Restrictions

Follow existing naming patterns.

Before introducing a new type:

* inspect nearby code
* inspect related systems
* follow repository conventions

Do not introduce new naming conventions within an existing system.

---

## Simplicity Rule

Prefer the simplest solution that satisfies the issue.

Avoid:

* extra abstraction
* future-proofing
* generic frameworks
* configuration systems
* extension systems

unless explicitly required.

Solve today's problem first.

---

## Verification Rule

Before completion:

* build the project when possible
* run defined verification steps
* verify issue acceptance criteria

Do not assume correctness.

Verify.

---

## Completion Requirements

When implementation is complete, provide:

### Files Modified

List all modified files.

### Summary

Explain what changed.

### Verification

Explain what was tested.

### Concerns

Identify anything uncertain.

### Follow-Up Work

Only if genuinely required.

---

## Stop Conditions

Stop and report the problem if:

* ownership is unclear
* architecture is unclear
* documentation conflicts with code
* the issue requires architectural decisions
* multiple implementation paths appear valid
* the requested change exceeds issue scope

Do not guess.

Do not invent architecture.

Do not silently choose a direction.

---

## Red Flags

Immediately investigate before continuing:

* duplicate systems
* duplicate loaders
* duplicate render paths
* duplicate ownership
* large unrelated changes
* architecture changes
* moving code between modules
* creating replacement implementations

These are often signs that the wrong solution is being implemented.

---

## Golden Rule

Implement the plan.

Do not create the plan.
