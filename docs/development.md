# Development Workflow

## Branch Strategy

### main

- Protected branch
- Stable branch
- Presentable branch
- Receives changes through pull requests only
- Should only receive completed milestone or release-ready work from `dev`

### dev

- Active integration branch
- Receives completed feature work
- Source branch for future stable updates to `main`

### feature/*

- One feature branch per issue or tightly related group of issues
- Created from `dev`
- Merged back into `dev` through a pull request

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

1. Select a GitHub Issue.
2. Move the issue to `Doing` on the project board.
3. Create a feature branch from `dev`.
4. Implement the change.
5. Push the feature branch.
6. Open a pull request into `dev`.
7. Merge the pull request into `dev`.
8. Close the issue.
9. Move the issue to `Done` on the project board.

Example:

```text
Issue #43
    ↓
feature/issue-43-cmake-workspace
    ↓
Pull Request
    ↓
dev
    ↓
Close issue
```

Issues are considered complete when their pull request is merged into `dev`.

Issues do not need to wait for `dev` to merge into `main` before being closed.

---

## Release Flow

When a milestone is complete and stable:

```text
dev
    ↓
Pull Request
    ↓
main
```

A `dev` to `main` pull request should represent a stable milestone or release-ready snapshot.

---

## Project Management

```text
Roadmap
    ↓
Milestones
    ↓
Issues
    ↓
Project Board
```

Project Board Columns:

- Todo
- Doing
- Blocked
- Done

Milestones define major goals.

Issues define concrete pieces of work.

The project board defines what is actively being worked on.

---

## Rules

Do not commit directly to `main`.

Do not use `main` for active development.

All implementation work should begin from a feature branch created from `dev`.

Prefer small pull requests that map clearly to one issue.
