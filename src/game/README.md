# game

Purpose: owns what rules apply.

The game module is responsible for gameplay rules such as players, inventory, equipment, combat, skills, prayer, magic, NPC behavior, items, objects, dialogue, shops, trade, bank, quests, and commands.

Dependency rule: `game` may depend on `data` and `world`. It should not depend on apps, rendering, UI, networking transport details, or platform-specific application code.

Server rule: ElServer is authoritative for gameplay rules. ElClient may only use limited game logic for prediction or UI presentation.
