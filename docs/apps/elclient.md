# ElClient

## Purpose

ElClient is the player-facing game client for Eldoria.

It is responsible for presenting the game world to players and providing an interface for interacting with that world.

ElClient should feel familiar to players of RuneScape-style games while using a modern and maintainable C++ architecture.

ElClient is a product.

It is not a development tool.

---

## Architectural Role

ElClient is the player's window into Eldoria.

It does not own the data, the world, the game rules, or server authority.

It composes shared systems into a playable experience.

```text
data
    ↓
world
    ↓
render
    ↓
ElClient presentation
```

When online, the full authority flow is:

```text
Player input
    ↓
ElClient intent
    ↓
net packet
    ↓
ElServer
    ↓
server-authoritative world/game update
    ↓
net update
    ↓
ElClient presentation
```

ElClient may also support local/offline debug modes, but those modes are development conveniences.

They must not change the long-term ownership model.

---

## Current State

ElClient currently exists as an application foundation.

Its current purpose is to provide a stable location for future client systems.

Current work should stay focused on:

* lifecycle
* screens
* input routing
* render loop foundation
* state placeholders
* clean shutdown

The current client may render a known model or placeholder scene as a diagnostic step.

That diagnostic path should not become the permanent world architecture.

Meaningful world presentation depends on expanding `data/` and then building shared `world/` representation.

---

## Long-Term Direction

ElClient should eventually support:

* startup flow
* loading screen
* login screen
* game screen
* local/offline debug mode
* server connection
* world rendering
* player control
* camera control
* RuneScape-style interfaces
* sprites
* animations
* chat
* minimap
* inventory display
* equipment display
* NPCs
* objects
* ground items
* combat presentation
* skill/action feedback
* sound and music where practical
* client settings
* custom Eldoria content

The long-term goal is not only to reproduce RuneScape.

The long-term goal is to support a custom RuneScape-like world with custom maps, custom models, custom bosses, custom items, custom equipment, custom interfaces, and custom gameplay.

The client should provide a complete player experience while remaining server-authoritative.

---

## Ownership

ElClient owns client-specific runtime behavior.

Examples:

* player-facing UI
* client screens
* client workflows
* input mapping
* local client state
* client session state
* loading flow
* login flow
* client-side error presentation
* client settings
* runtime interface behavior
* local/offline debug presentation
* client-side packet handling behavior
* camera behavior
* presentation state

ElClient does not own shared systems.

Examples:

* cache reading belongs in `src/data/`
* model decoding belongs in `src/data/`
* texture decoding belongs in `src/data/`
* map decoding belongs in `src/data/`
* object/item/NPC definitions belong in `src/data/`
* spatial world representation belongs in `src/world/`
* gameplay authority belongs in `src/game/` and ElServer
* packet definitions and codecs belong in `src/net/`
* rendering backend and render pipeline behavior belong in `src/render/`
* platform/window behavior belongs in `src/platform/`
* content creation belongs in ElForge
* authoritative persistence belongs in ElServer

---

## Relationship To Shared Systems

ElClient consumes shared systems.

It should not reimplement them.

```text
ElClient
    uses data for loaded content
    uses world for spatial state
    uses render for pixels
    uses net for packet language
    uses platform for machine integration
```

Important distinction:

```text
data
= static facts loaded from cache/content sources

world
= spatial reality built from data and runtime updates

game
= rules applied to the world

net
= how state and actions travel

render
= how visible state becomes pixels

ElClient
= player-facing presentation and input workflow
```

---

## Planned App Structure

ElClient should remain thinner than the shared systems it consumes.

Prefer a small application structure:

```text
apps/elclient/
├── app/
├── state/
├── screens/
├── input/
├── ui/
├── session/
├── loading/
├── settings/
├── error/
└── debug/
```

Do not create app-local copies of shared module systems.

Avoid permanent folders such as:

```text
apps/elclient/world/
apps/elclient/data/
apps/elclient/net/
apps/elclient/render/
```

unless the contents are strictly client-specific adapters or presentation helpers and cannot reasonably belong in the shared module.

When client-specific glue is required, name it according to the workflow it supports rather than pretending it is the shared system.

Examples:

```text
loading/AssetLoadStep
screens/GameScreen
ui/InventoryPanel
session/ConnectionState
debug/PacketLogOverlay
```

---

## Client State

Client state should represent what the client currently needs to present and operate.

Examples:

* current screen
* connection status
* session status
* loading status
* input state
* render status
* local/offline mode status
* current visible world placeholder or snapshot reference
* interface state
* error state

Client state should not become authoritative game state.

When online, ElServer remains authoritative.

When offline/debugging, local state may be used to exercise rendering and interaction workflows, but it should be clearly identified as local/debug state.

---

## Future Workflows

### Startup

```text
main()
    ↓
ElClientApp
    ↓
Initialize platform/window
    ↓
Initialize client state
    ↓
Enter loading/startup screen
```

---

### Loading

```text
Client settings
    ↓
Open cache/content source
    ↓
Load required data/assets
    ↓
Prepare first screen or local debug scene
```

Loading uses `data/`.

Loading does not decode cache formats inside ElClient.

---

### Login

```text
Player
    ↓
Login Screen
    ↓
Login Request
    ↓
net packet
    ↓
ElServer
```

Login UI belongs in ElClient.

Login packet structures belong in `net/`.

Login authority belongs in ElServer.

---

### Gameplay

```text
Player input
    ↓
Client intent
    ↓
ElServer request
    ↓
Server validation
    ↓
World/game update
    ↓
Client presentation
```

ElClient presents the result.

It does not own the game truth.

---

### Local World Visualization

```text
data map/object/model/texture content
    ↓
world snapshot or local world representation
    ↓
render scene input
    ↓
GameScreen presentation
```

This is the path that turns the client from an empty shell into a useful game-viewer.

It depends on `data/` and `world/` foundations.

---

## Development Sequence

ElClient should grow in a dependency-aware order.

Recommended sequence:

1. Client shell.
2. Client state and screen flow.
3. Loading/settings/error foundations.
4. Data expansion in `src/data/`.
5. World foundation in `src/world/`.
6. Local world visualization in ElClient.
7. Server/network integration.
8. Gameplay presentation.
9. Interface polish and production reliability.

This means the client may temporarily render placeholders or a known model for verification.

However, the next meaningful client milestone after the shell is not more fake login polish.

The next meaningful client milestone is displaying a real local world snapshot once data and world foundations exist.

---

## Product Goal

ElClient is not finished when it opens a window.

ElClient is not finished when it renders one model.

ElClient is not finished when it renders one map.

ElClient is finished enough for a serious server when it can support the full player experience:

1. Launch reliably.
2. Load required game assets.
3. Connect to ElServer.
4. Log in successfully.
5. Display the world correctly.
6. Display the local player, other players, NPCs, objects, and ground items.
7. Present RuneScape-style interfaces.
8. Present gameplay controlled by ElServer.
9. Handle custom Eldoria content.
10. Recover cleanly from errors, disconnects, missing data, and bad packets.
11. Feel stable and polished enough for real players.

The final goal is not just a technical client.

The final goal is a playable client that can support a real private server community.

---

## Common Mistakes

Do not:

* implement gameplay authority in ElClient
* implement persistence in ElClient
* duplicate server logic
* decode cache formats in ElClient
* create a second world representation inside ElClient
* create app-local packet definitions in ElClient
* create app-local render pipelines in ElClient
* make shared modules depend on ElClient
* turn `GameScreen` into the whole game
* treat local/offline debug state as production authority

ElClient presents the game.

It does not own the game.

---

## Verification

```bash
cmake --build build --target elclient
```

Manual verification depends on the current phase.

For client foundation work:

```text
Launch ElClient.
Verify window opens.
Verify screen flow works.
Verify input events route correctly.
Verify render loop runs.
Verify shutdown is clean.
```

For later world visualization work:

```text
Launch ElClient.
Load a local world/region snapshot.
Verify terrain renders.
Verify placed objects render.
Verify camera movement works.
Verify missing data fails clearly.
```

---

## Golden Rule

ElClient shows the world.

Shared systems describe and render the world.

ElServer owns the online truth.
