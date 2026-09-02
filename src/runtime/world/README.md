# world

Purpose: owns shared spatial reality.

The world module is responsible for spatial state, positions, regions, tiles, maps as runtime spaces, entity placement, movement representation, collision/pathing concepts, and world-state structures shared by tools, client, server, game, and net.

World is built from clean data structures provided by `data`.

World does not decode cache formats, own gameplay rules, render pixels, define packet language, own app UI, or own server authority.

Dependency rule: `world` may use data concepts where needed, but should not depend on runnable apps, rendering backends, UI panels, game rules, networking behavior, or platform-specific application code.
