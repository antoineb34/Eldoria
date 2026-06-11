# Implementation Workflow

## Purpose

This document defines the standard implementation workflow used within Eldoria.

It describes how work moves from milestone planning to merged code.

All implementation agents should follow this workflow.

---

## Core Principle

Implementation should be predictable.

Implementation should not require architectural decision making.

The Architect defines the plan.

The Agent executes the plan.

The Review validates the result.

---

# Workflow

Architecture
↓
Milestone
↓
Issue
↓
Implementation Plan
↓
Agent Prompt
↓
Implementation
↓
Pull Request
↓
Review
↓
Merge

---

# Step 1: Understand The Issue

Before making any changes:

Read:

* issue description
* implementation prompt
* relevant documentation

Identify:

* objective
* ownership
* affected systems
* acceptance criteria
* verification requirements

Do not start coding yet.

---

# Step 2: Inspect Existing Code

Inspect:

* relevant modules
* relevant systems
* referenced files

Determine:

* current implementation
* existing ownership
* existing patterns
* existing APIs

Look for existing solutions before creating new code.

Prefer extending existing systems.

---

# Step 3: Confirm Ownership

Before modifying code:

Ask:

* Which module owns this behavior?
* Which system owns this responsibility?
* Does an implementation already exist?

If ownership is unclear:

Stop.

Report the problem.

Do not guess.

---

# Step 4: Plan The Change

Before editing files:

Identify:

* files expected to change
* implementation approach
* verification strategy

The implementation should be understandable before coding begins.

Avoid exploratory implementation.

---

# Step 5: Implement

Implement only the requested change.

Follow:

* issue scope
* prompt instructions
* ownership boundaries
* repository conventions

Do not:

* expand scope
* refactor unrelated systems
* redesign architecture
* create duplicate systems

Keep changes focused.

---

# Step 6: Verify

Perform defined verification steps.

Examples:

```bash
cmake --build build
```

```bash
ctest --test-dir build
```

Manual validation:

* open viewer
* load asset
* verify expected behavior

Verification should occur before creating a pull request.

---

# Step 7: Review Your Own Changes

Before creating a pull request:

Check:

* issue goal achieved
* ownership preserved
* no unrelated changes
* verification completed
* documentation updated if required

Look for unnecessary complexity.

Simpler solutions are generally preferred.

---

# Step 8: Create Pull Request

Pull request should contain:

## Summary

What changed.

## Files Modified

Which files changed.

## Reasoning

Why those changes were made.

## Verification

What was tested.

## Known Concerns

Anything uncertain.

A reviewer should be able to understand the change quickly.

---

# Step 9: Review

The Architect reviews:

* scope
* ownership
* architecture
* correctness
* simplicity
* documentation

Review feedback should be addressed before merge.

---

# Step 10: Merge

After approval:

Pull Request
↓
Merge Into Target Branch
↓
Close Issue
↓
Update Project Board

Implementation is complete only after review and merge.

---

# Stop Conditions

Stop and report the issue if:

* ownership is unclear
* architecture is unclear
* code conflicts with documentation
* issue requires architectural decisions
* multiple valid implementation paths exist
* issue scope is insufficient

Do not invent solutions.

Do not invent architecture.

Escalate uncertainty.

---

# Success Criteria

A successful implementation:

* solves the issue
* respects ownership
* follows architecture
* passes verification
* remains within scope
* avoids unnecessary complexity

Working code is not enough.

Correctly placed working code is the goal.

---

# Golden Rule

Understand first.

Implement second.

Verify third.
