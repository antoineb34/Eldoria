# data

Purpose: owns static content and the process of loading it.

Current production responsibilities:

- cache access
- binary reading
- compression
- archive reading
- model loading
- texture loading

Future expansion responsibilities:

- config archives
- map data
- object definitions
- item definitions
- NPC definitions
- sprites
- interfaces
- animations

Data describes what things are and how static content is loaded.

It does not own live world state, gameplay rules, rendering behavior, networking behavior, app UI, or server authority.

Dependency rule: `data` should stay independent from runnable apps and should not depend on `world`, `game`, `net`, `render`, UI, or platform-specific application code.
