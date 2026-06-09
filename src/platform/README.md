# platform

Purpose: owns how Eldoria talks to the machine.

The platform module is responsible for system and library integration boundaries such as SDL setup, windowing/context wrappers, timing/input abstractions, filesystem/platform helpers, and other OS/library-facing adapters.

Dependency rule: `platform` should stay low-level and should not own game rules, content definitions, app-specific UI panels, or rendering policy.
