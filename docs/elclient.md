# ElClient Roadmap

## Purpose

This document defines the development roadmap for ElClient.

ElClient is the player-facing C++ game client for Eldoria.

It should eventually become a complete RuneScape-317-feeling client for a custom private server ecosystem.

This document answers:

> How does ElClient grow from an empty application into a complete playable client?

Small implementation tasks belong in GitHub Issues.

This document defines:

- vision
- ownership boundaries
- planned app structure
- milestones
- feature areas
- issue candidates
- product completion criteria

---

## Vision

ElClient is the player's window into Eldoria.

It should feel recognizable as a RuneScape-style client while being built with a clean modern C++ architecture.

The final client should support:

- startup flow
- loading screen
- login screen
- game screen
- local/offline debug mode
- server connection
- world rendering
- player control
- camera control
- RuneScape-style interfaces
- sprites
- animations
- chat
- minimap
- inventory display
- equipment display
- NPCs
- objects
- ground items
- combat presentation
- skill/action feedback
- sound and music where practical
- client settings
- custom Eldoria content

The long-term goal is not only to reproduce RuneScape.

The long-term goal is to support a custom RuneScape-like world with custom maps, custom models, custom bosses, custom items, custom equipment, custom interfaces, and custom gameplay.

---

## Product Goal

ElClient is not finished when it opens a window.

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

## Design Priorities

### RuneScape Feel

ElClient should feel like a RuneScape-317-style client.

Important qualities:

- recognizable camera feel
- recognizable world scale
- recognizable model style
- recognizable animation style
- recognizable interface layout
- recognizable movement feel
- recognizable chat, inventory, and minimap behavior

Accuracy matters, especially visually.

### Modern Client Foundation

The client should not become a giant tangled class.

Major systems should stay separated:

- app flow
- screens
- input
- camera
- client world state
- rendering
- networking
- interfaces
- debug tools
- settings
- error handling

### Offline Debug Mode

ElClient should support loading a local world without connecting to ElServer.

This is useful for debugging:

- map rendering
- object placement
- collision
- camera
- local movement
- animations
- interfaces
- custom content

### Server Authority

When connected online, ElServer owns the truth.

ElClient may predict, display, and smooth behavior, but the server remains authoritative.

Client asks.

Server decides.

Client displays.

### Custom Content Support

ElClient should eventually support custom Eldoria content:

- custom models
- custom maps
- custom equipment
- custom NPCs
- custom bosses
- custom interfaces
- custom effects
- custom game assets exported by ElForge

### Production Reliability

ElClient should eventually handle real player conditions:

- bad login
- disconnects
- reconnect attempts
- missing assets
- incompatible content versions
- packet errors
- loading failures
- performance issues
- user settings
- clean shutdown

---

## What ElClient Owns

ElClient owns client-specific runtime behavior.

Examples:

- startup flow
- loading flow
- login flow
- screen transitions
- client session state
- local world mirror
- local player presentation
- camera behavior
- input mapping
- game screen behavior
- runtime game interfaces
- client packet handling
- offline debug mode
- client debug overlays
- client settings
- client-side error presentation
- client-side content version checks

---

## What ElClient Does Not Own

ElClient should not own shared systems.

Examples:

- cache reading belongs in `src/data/`
- model decoding belongs in `src/data/`
- texture decoding belongs in `src/data/`
- map decoding belongs in `src/data/`
- world definitions belong in `src/world/`
- gameplay authority belongs in `src/game/` and ElServer
- packet definitions belong in `src/net/`
- rendering backend belongs in `src/render/`
- platform/window/input behavior belongs in `src/platform/`
- content creation belongs in ElForge
- authoritative persistence belongs in ElServer

ElClient combines shared systems into a playable experience.

---

## Planned App Structure

```text
apps/elclient/
├── main/
├── app/
├── flow/
├── screens/
├── loading/
├── session/
├── settings/
├── input/
├── camera/
├── world/
├── render/
├── interface/
├── audio/
├── net/
├── content/
├── error/
└── debug/
```

### `main/`

Application entry point.

### `app/`

Owns the client lifetime.

Examples:

- ElClientApp
- startup
- update loop
- render loop
- shutdown

### `flow/`

Controls client state.

Examples:

- Boot
- Loading
- Login
- Connecting
- Game
- Disconnected
- Error
- Shutdown

### `screens/`

Major visible screens.

Examples:

- LoadingScreen
- LoginScreen
- GameScreen
- DisconnectedScreen
- ErrorScreen

### `loading/`

Client loading steps.

Examples:

- open cache
- load sprites
- load fonts
- load definitions
- check content version
- prepare first scene

### `session/`

Current client session state.

Examples:

- username
- login result
- connection state
- local player id
- current world/region
- online/offline mode

### `settings/`

Client configuration.

Examples:

- cache path
- server address
- fullscreen/windowed mode
- audio settings
- camera settings
- debug settings

### `input/`

Converts platform input into client actions.

Examples:

- camera movement
- mouse clicks
- keyboard shortcuts
- text input
- menu selection
- game interaction input

### `camera/`

Client camera behavior.

Examples:

- orbit camera
- follow player camera
- debug free camera
- zoom
- rotation
- smoothing

### `world/`

Client-side local world mirror.

Examples:

- loaded regions
- visible players
- visible NPCs
- visible objects
- ground items
- local player
- pending updates

### `render/`

Client-specific render scene construction.

This does not replace `src/render/`.

It converts client/world/interface state into renderable scenes.

### `interface/`

Runtime RuneScape-style client interface system.

Examples:

- chatbox
- inventory
- equipment
- minimap
- context menu
- gameframe
- widgets

Interface definitions come from `src/data/interface/`.

Runtime behavior belongs in ElClient.

### `audio/`

Client-side sound and music behavior.

Examples:

- sound effect requests
- music playback requests
- audio settings
- positional sound later if needed

Actual platform audio access belongs in `src/platform/audio/`.

### `net/`

Client-specific networking behavior.

Examples:

- connect to server
- send login
- send movement intent
- receive updates
- dispatch server packets
- disconnect handling
- reconnect handling

Packet definitions/codecs belong in `src/net/`.

### `content/`

Client-side custom content integration.

Examples:

- exported content loading
- content version metadata
- missing content fallback
- custom asset hooks

### `error/`

Client-side error handling and recovery.

Examples:

- login error display
- disconnect error display
- missing asset error display
- incompatible content version display

### `debug/`

Client debugging tools.

Examples:

- FPS overlay
- region overlay
- tile coordinate overlay
- collision overlay
- packet log
- entity debug view
- content version debug
- asset loading debug

---

# Milestones

---

## Milestone 1 — Client Shell

### Goal

Create the first runnable ElClient shell.

### Success Criteria

- ElClient launches.
- Window opens.
- Loading screen exists.
- Login screen exists.
- Empty game screen exists.
- Client can transition between screens.
- Clean shutdown path exists.

### Feature Areas

- application lifecycle
- screen system
- basic rendering surface
- basic input/event handling
- state transitions

### Issue Candidates

- Create `apps/elclient` target.
- Create ElClient application entry point.
- Create ElClientApp lifetime class.
- Create client state enum.
- Create basic screen interface.
- Create loading screen.
- Create login screen placeholder.
- Create empty game screen.
- Implement screen transition system.
- Add basic window title and startup config.
- Add clean shutdown path.

### Proves

ElClient exists as a separate application and has a clean app flow.

---

## Milestone 2 — Login Screen Flow

### Goal

Make the login screen usable before the full server exists.

### Success Criteria

- Username/password input exists.
- Login button exists.
- Login attempt can be triggered.
- Client can show login success or failure.
- Client can enter empty game screen after success.
- Client can show a basic disconnected/error message.

### Feature Areas

- login screen UI
- text input
- client session state
- fake/local login mode
- error messaging

### Issue Candidates

- Add username input field.
- Add password input field.
- Add login button.
- Store login form state.
- Add fake login success path.
- Add fake login failure path.
- Add login status message.
- Add transition from login screen to game screen.
- Add transition from game screen back to login screen.
- Add basic keyboard navigation for login form.
- Add basic disconnected/error screen placeholder.

### Proves

The client can handle user-driven state transitions.

---

## Milestone 3 — Tiny Server Login Loop

### Goal

Connect ElClient to a minimal ElServer login flow.

### Success Criteria

- ElClient can connect to ElServer.
- ElClient can send login data.
- ElClient can receive login response.
- ElClient enters game screen after server success.
- Failed login shows a message.
- Connection failure shows a message.
- Packet flow can be logged.

### Feature Areas

- client connection
- login packet sending
- login response handling
- connection state
- packet logging
- login error handling

### Issue Candidates

- Create client connection state.
- Create client network service.
- Connect to local ElServer.
- Send login request packet.
- Receive login response packet.
- Handle login success.
- Handle login failure.
- Handle connection failure.
- Add network status to login screen.
- Add packet log debug output.
- Add disconnect handling.

### Proves

Client/server communication works at the simplest useful level.

---

## Milestone 4 — Client Settings Foundation

### Goal

Give ElClient basic configuration and startup settings.

### Success Criteria

- Client can store settings.
- Client can load settings.
- Server address can be configured.
- Cache/content path can be configured.
- Window settings can be configured.
- Debug options can be toggled.

### Feature Areas

- settings file
- startup configuration
- server config
- cache/content config
- window config
- debug config

### Issue Candidates

- Define client settings format.
- Add settings load path.
- Add settings save path.
- Add server address setting.
- Add cache path setting.
- Add content path setting.
- Add fullscreen/windowed setting.
- Add debug overlay setting.
- Add settings validation.
- Add default settings creation.
- Add settings error display.

### Proves

ElClient can be configured without hardcoding every runtime option.

---

## Milestone 5 — Asset Loading Integration

### Goal

Connect ElClient to shared data systems.

### Success Criteria

- ElClient can open the cache.
- ElClient can load required startup assets.
- ElClient can load models.
- ElClient can load textures.
- ElClient can report missing assets cleanly.
- Loading flow can show progress or status.

### Feature Areas

- cache path config
- startup loading steps
- asset loading
- asset error handling
- loading progress
- asset debug output

### Issue Candidates

- Add cache open step to loading flow.
- Add loading progress state.
- Load one known model from client startup.
- Load one known texture from client startup.
- Add missing asset error reporting.
- Add loading failure screen or message.
- Add basic asset loading log.
- Add reload/debug asset command.
- Add cache path override option.
- Add loading progress display.

### Proves

ElClient can use the same asset foundation as ElForge.

---

## Milestone 6 — Basic Render Scene

### Goal

Render something through the shared render system.

### Success Criteria

- ElClient creates a render scene.
- ElClient owns a camera.
- ElClient submits render objects.
- Shared renderer draws the scene.
- Debug camera or simple camera controls exist.
- Render errors can be surfaced cleanly.

### Feature Areas

- render scene setup
- client camera
- render object submission
- viewport handling
- debug controls
- render diagnostics

### Issue Candidates

- Create client render scene builder.
- Create client camera state.
- Add free camera controls.
- Add render viewport.
- Submit one test render object.
- Render one loaded model.
- Add camera movement keys.
- Add camera zoom.
- Add render stats overlay.
- Add clear/background rendering.
- Add render error display.

### Proves

ElClient can use `src/render/` instead of having its own separate rendering path.

---

## Milestone 7 — Local Map Region

### Goal

Load and display a local map region without a server.

### Success Criteria

- A map region can be loaded.
- Terrain can be displayed.
- Placed objects can be displayed.
- Camera can move around the region.
- Collision or tile flags can be visualized.
- Missing map/object assets are handled cleanly.

### Feature Areas

- map loading
- terrain conversion
- object placement
- render scene building
- collision debug
- coordinate conversion
- missing asset fallback

### Issue Candidates

- Load map region by region id.
- Convert map data into client region state.
- Convert tile coordinates to world coordinates.
- Build terrain render geometry.
- Render terrain height data.
- Load placed object data.
- Convert placed objects into world objects.
- Convert world objects into render objects.
- Render map objects.
- Add missing object model fallback.
- Add region coordinate debug overlay.
- Add tile hover debug output.
- Add collision flag visualization.
- Add camera region navigation.

### Proves

ElClient can display a real world area using `data`, `world`, and `render`.

---

## Milestone 8 — Local Player

### Goal

Place a controllable player into the local world.

### Success Criteria

- Local player exists.
- Player appears in the world.
- Camera can follow the player.
- Player can move locally.
- Basic collision can be respected.
- Player state is separate from rendering state.

### Feature Areas

- local player state
- player rendering
- movement input
- camera follow
- collision checking
- local world interaction

### Issue Candidates

- Create local player state.
- Add local player spawn position.
- Render local player placeholder.
- Replace placeholder with model-based player.
- Add click-to-move or keyboard movement prototype.
- Add camera follow mode.
- Add player position debug overlay.
- Check collision before local movement.
- Add simple walking queue.
- Add local movement smoothing.
- Separate player state from render object state.

### Proves

ElClient can represent and control a player before server authority is introduced.

---

## Milestone 9 — Animation Support

### Goal

Display animated characters or models.

### Success Criteria

- Animation data can be loaded.
- Animation can be inspected in ElForge.
- ElClient can apply a basic animation.
- Player can show idle/walk animation.
- Animation state is separated from rendering.
- Bad or missing animations fail gracefully.

### Feature Areas

- animation loading
- animation state
- model transformation
- player animation
- animation debugging
- fallback animation

### Issue Candidates

- Create animation probe if needed.
- Load basic animation data.
- Define client animation state.
- Apply idle animation to player model.
- Apply walk animation to player model.
- Add animation playback time.
- Add animation speed control.
- Add animation debug overlay.
- Add missing animation fallback.
- Separate animation state from player state.
- Validate animation visually in ElForge and ElClient.

### Proves

ElClient can present living entities instead of static models.

---

## Milestone 10 — Server-Authoritative Movement

### Goal

Replace local-only movement with server-confirmed movement.

### Success Criteria

- Client sends movement intent.
- Server validates movement.
- Server sends confirmed position.
- Client updates local player state.
- Client displays the confirmed movement.
- Client/server desync can be debugged.
- Prediction can be disabled for debugging.

### Feature Areas

- movement packets
- movement intent
- server response handling
- local prediction
- correction/smoothing
- desync debugging

### Issue Candidates

- Send movement intent packet.
- Receive movement confirmation packet.
- Add client movement pending state.
- Add server position update handler.
- Update local player from server position.
- Add movement debug log.
- Add client/server position comparison overlay.
- Add basic correction when server disagrees.
- Add option to disable prediction.
- Add movement packet dump.

### Proves

The first real online gameplay loop works.

---

## Milestone 11 — World Synchronization

### Goal

Synchronize nearby world state from ElServer.

### Success Criteria

- Nearby players can appear.
- NPCs can appear.
- Objects can update.
- Ground items can appear.
- Region changes can be received.
- Chat messages can be received.
- Client world mirror updates correctly.
- Stale/removed entities are handled.

### Feature Areas

- entity update packets
- client world mirror
- player synchronization
- NPC synchronization
- object synchronization
- ground item synchronization
- region updates
- chat updates
- entity removal

### Issue Candidates

- Add client entity registry.
- Receive nearby player update packet.
- Render nearby player placeholder.
- Receive NPC update packet.
- Render NPC placeholder.
- Receive object update packet.
- Update world object state.
- Receive ground item update packet.
- Render ground item marker/model.
- Receive region change packet.
- Load new region on update.
- Receive chat message packet.
- Display chat message in debug view.
- Handle removed entity update.
- Add entity debug overlay.
- Add world sync packet log.

### Proves

ElClient can display an online world controlled by ElServer.

---

## Milestone 12 — Sprite and Interface Foundation

### Goal

Begin supporting RuneScape-style 2D interface rendering.

### Success Criteria

- Sprites can be loaded.
- Fonts/text rendering path exists.
- Interface definitions can be loaded.
- Basic widgets can be displayed.
- Gameframe path is clear.
- Chatbox path is clear.
- Inventory path is clear.
- Minimap path is clear.
- Interface errors can be debugged.

### Feature Areas

- sprite loading
- font/text rendering
- interface definition loading
- widget tree
- 2D rendering
- layout
- input routing
- interface debug

### Issue Candidates

- Load sprite asset.
- Render sprite on screen.
- Load font/text data.
- Render text string.
- Load interface definition.
- Build runtime widget tree.
- Render basic widget rectangle.
- Render sprite widget.
- Render text widget.
- Add widget positioning.
- Add widget mouse hover detection.
- Add widget click detection.
- Add debug interface inspector.
- Add gameframe placeholder.
- Add chatbox placeholder.
- Add inventory placeholder.
- Add minimap placeholder.
- Add interface error display.

### Proves

ElClient can begin replacing placeholder screens with real RuneScape-style interfaces.

---

## Milestone 13 — Game Interface Systems

### Goal

Implement runtime client interfaces.

### Success Criteria

- Chatbox works.
- Inventory interface displays items.
- Equipment interface displays equipment.
- Context menu exists.
- Minimap exists or has a placeholder path.
- Basic interface interactions work.
- UI can update from server state.

### Feature Areas

- chatbox
- inventory UI
- equipment UI
- context menu
- minimap
- interface input
- interface state updates

### Issue Candidates

- Implement chatbox layout.
- Display incoming chat messages.
- Add chat text input.
- Send chat message packet.
- Implement inventory grid.
- Display inventory item icons.
- Display inventory item amounts.
- Handle inventory item hover.
- Handle inventory item click.
- Implement equipment slots.
- Display equipped item icons.
- Add context menu system.
- Add right-click menu options.
- Implement minimap placeholder.
- Add minimap player marker.
- Route mouse input to interfaces.
- Add interface debug overlay.

### Proves

The client is becoming a real game client, not only a world renderer.

---

## Milestone 14 — Gameplay Presentation

### Goal

Display gameplay systems controlled by ElServer.

### Success Criteria

- Inventory changes are displayed.
- Equipment changes are displayed.
- Object interactions show feedback.
- NPC interactions show feedback.
- Skill progress can be displayed.
- Combat feedback can be displayed.
- Chat/messages display correctly.
- Client never becomes the gameplay authority.

### Feature Areas

- inventory updates
- equipment updates
- object interaction feedback
- NPC interaction feedback
- skill feedback
- combat feedback
- server message display

### Issue Candidates

- Receive inventory update packet.
- Update inventory UI from server state.
- Receive equipment update packet.
- Update equipment UI from server state.
- Send object interaction packet.
- Display object interaction response.
- Send NPC interaction packet.
- Display NPC interaction response.
- Receive skill XP update packet.
- Display skill XP/level change.
- Receive combat hit update packet.
- Display hit splat placeholder.
- Display combat animation/effect placeholder.
- Add server message display.
- Add gameplay event debug log.

### Proves

ElClient can present real gameplay without owning gameplay authority.

---

## Milestone 15 — Audio and Feedback

### Goal

Add client-side sound, music, and feedback hooks where practical.

### Success Criteria

- Client can play a basic sound effect.
- Client can play music or ambient track if supported.
- Gameplay events can trigger sound feedback.
- Audio can be enabled/disabled in settings.
- Missing audio fails gracefully.

### Feature Areas

- sound effect playback
- music playback
- audio settings
- gameplay audio hooks
- missing audio fallback

### Issue Candidates

- Add audio settings.
- Initialize client audio layer.
- Play test sound effect.
- Play test music track.
- Trigger sound from interface click.
- Trigger sound from gameplay event.
- Add mute toggle.
- Add volume setting.
- Add missing sound fallback.
- Add audio debug output.

### Proves

ElClient can provide feedback beyond visuals.

---

## Milestone 16 — Custom Content Presentation

### Goal

Display custom Eldoria content.

### Success Criteria

- Custom models can appear.
- Custom maps can appear.
- Custom NPCs can appear.
- Custom items can appear.
- Custom bosses can appear.
- Custom interfaces can appear if supported.
- Client handles custom content without architecture changes.
- Client can detect incompatible or missing custom content.

### Feature Areas

- custom asset loading
- custom map rendering
- custom NPC/player models
- custom item presentation
- custom boss presentation
- custom interface support
- content version checks
- content error handling

### Issue Candidates

- Load custom model asset.
- Render custom equipment model.
- Load custom map region.
- Render custom object placement.
- Display custom NPC.
- Display custom item icon/model.
- Display custom boss model.
- Add missing custom asset fallback.
- Add custom content debug output.
- Add content version/hash check.
- Display incompatible content error.
- Validate custom content through ElForge and ElClient.

### Proves

ElClient supports the long-term custom server vision.

---

## Milestone 17 — Disconnects, Errors, and Recovery

### Goal

Handle real-world failure cases cleanly.

### Success Criteria

- Client handles server disconnect.
- Client handles failed connection.
- Client handles failed login.
- Client handles incompatible content version.
- Client handles missing assets.
- Client can return to login screen after disconnect.
- Errors are understandable.

### Feature Areas

- disconnect handling
- reconnect path
- error screens
- content mismatch errors
- missing asset errors
- packet error handling

### Issue Candidates

- Add disconnected screen.
- Add failed connection message.
- Add failed login reason display.
- Add return-to-login action.
- Add reconnect attempt action.
- Add incompatible content version screen.
- Add missing asset error screen.
- Add packet decode error handling.
- Add fatal error logging.
- Add recoverable error logging.
- Add user-readable error messages.

### Proves

ElClient can handle more than the happy path.

---

## Milestone 18 — Client Debug and Developer Tools

### Goal

Make ElClient easy to debug during development.

### Success Criteria

- Developer overlays exist.
- Packet logs can be viewed.
- Entity/world debug information can be viewed.
- Rendering stats can be viewed.
- Asset loading information can be viewed.
- Debug tools can be toggled.

### Feature Areas

- debug overlay
- packet logging
- world debug
- render debug
- asset debug
- performance stats

### Issue Candidates

- Add debug overlay toggle.
- Add FPS display.
- Add render timing display.
- Add asset loading timing display.
- Add packet log overlay.
- Add entity count display.
- Add player coordinate display.
- Add tile coordinate hover display.
- Add collision debug overlay.
- Add region debug overlay.
- Add content version debug display.
- Add debug settings.

### Proves

ElClient can be developed and diagnosed efficiently.

---

## Milestone 19 — Client Polish

### Goal

Improve quality, feel, and usability.

### Success Criteria

- Camera feels good.
- Input feels good.
- Rendering performance is acceptable.
- Loading behavior is clean.
- Error handling is clear.
- Debug tools are useful.
- Client feels stable.

### Feature Areas

- camera polish
- input polish
- rendering performance
- loading UX
- error handling
- debug tools
- stability

### Issue Candidates

- Smooth camera movement.
- Tune camera zoom.
- Tune camera rotation.
- Improve click handling.
- Improve loading progress display.
- Improve disconnect screen.
- Add crash/error reporting path.
- Add FPS/performance overlay.
- Add render timing stats.
- Add asset loading timing stats.
- Add packet timing stats.
- Clean up temporary debug code.
- Improve client configuration.
- Add fullscreen/windowed option.

### Proves

ElClient is moving from technical prototype toward real product.

---

## Milestone 20 — Production Readiness

### Goal

Prepare ElClient for real players.

### Success Criteria

- Client can be packaged or distributed.
- Required assets/content can be located reliably.
- Settings persist reliably.
- User-facing errors are clear.
- Client version is visible.
- Content version is visible.
- Release checklist exists.
- Known limitations are documented.

### Feature Areas

- packaging
- release configuration
- version display
- content versioning
- user documentation
- stability
- distribution readiness

### Issue Candidates

- Add release build configuration.
- Add client version display.
- Add content version display.
- Add asset/content directory discovery.
- Add portable settings path.
- Add release mode logging.
- Add basic user documentation.
- Add install/run instructions.
- Add known limitations document.
- Add release checklist.
- Add basic compatibility checklist.
- Add public test checklist.

### Proves

ElClient is not only a dev prototype, but a client that could eventually be given to real users.

---

## Long-Term Completion Criteria

ElClient can be considered complete enough for a serious server when:

- It launches reliably.
- It loads required assets reliably.
- It logs into ElServer.
- It handles failed logins and disconnects cleanly.
- It displays the world correctly.
- It displays the local player correctly.
- It displays other players and NPCs.
- It displays world objects and ground items.
- It supports RuneScape-style interfaces.
- It presents inventory, equipment, chat, combat, and skills.
- It handles custom Eldoria content.
- It detects missing or incompatible content.
- It respects server authority.
- It has debug tools for development.
- It has enough polish to feel like a real game client.
- It can be packaged or distributed for testing.

---

## GitHub Issues Strategy

GitHub Issues should be generated from milestone issue candidates.

Recommended issue size:

> Small enough that one focused implementation can complete it.

Bad issue:

```text
Implement local world
```

Better issues:

```text
Load map region by region id
Convert map data into client region state
Build terrain render geometry
Render placed world objects
Add tile coordinate debug overlay
```

Recommended labels:

- `elclient`
- `data`
- `world`
- `render`
- `net`
- `interface`
- `audio`
- `debug`
- `settings`
- `content`
- `error-handling`
- `release`
- `bug`
- `cleanup`

---

## Relationship to Other Documents

### `architecture.md`

Defines where client code belongs.

### `roadmap.md`

Defines the global build order.

### `elforge.md`

Defines the tool used to inspect, validate, and create content.

### `elserver.md`

Defines the authoritative server ElClient connects to.

---

## Status

This document is the current ElClient roadmap baseline.

It is expected to evolve as the client becomes real.
