# game

Purpose: owns reusable gameplay rules.

The game module is responsible for rules such as inventory, equipment, combat, skills, prayer, magic, item interactions, object interactions, NPC interactions, dialogue, shops, trade, bank, quests, and commands.

Game rules may use static definitions from `data` and spatial state from `world`.

Game does not own static data loading, spatial world representation, packet language, rendering, app UI, persistence, or server sessions.

Dependency rule: `game` may depend on `data` and `world`. It should not depend on apps, rendering, UI, networking transport/protocol details, or platform-specific application code.

Authority rule: ElServer executes gameplay authoritatively online. ElClient may only use limited game logic for prediction or UI presentation when explicitly planned.
