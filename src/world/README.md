# world

Purpose: owns where things are and how they move.

The world module is responsible for spatial state, positions, regions, maps as runtime spaces, entity placement, movement representation, pathing concepts, and world-state structures shared by the client and server.

Dependency rule: `world` may use data concepts where needed, but should not depend on runnable apps, rendering backends, UI panels, or platform-specific application code.
