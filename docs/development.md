# Development Workflow

## Branch Strategy

main
- Protected branch
- Stable branch
- Presentable branch
- Receives changes through pull requests only

.dev
- Active integration branch
- Receives completed feature work
- Source for future releases

feature/*
- One feature branch per issue
- Created from dev
- Merged back into dev through a pull request

## Development Flow

1. Select a GitHub Issue.
2. Create a feature branch from dev.
3. Implement the change.
4. Push the feature branch.
5. Open a pull request into dev.
6. Merge the pull request.
7. Close the issue.

Example:

Issue #43
    ↓
feature/issue-43-cmake-workspace
    ↓
Pull Request
    ↓
dev

## Release Flow

When a milestone is complete and stable:

dev
    ↓
Pull Request
    ↓
main

## Project Management

Roadmap
    ↓
Milestones
    ↓
Issues
    ↓
Project Board

Project Board Columns:
- Todo
- Doing
- Blocked
- Done

## Notes

Do not commit directly to main.

All implementation work should begin from a feature branch created from dev.