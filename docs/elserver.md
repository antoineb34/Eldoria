# ElServer Roadmap

## Purpose

This document defines the development roadmap for ElServer.

ElServer is the authoritative C++ game server for Eldoria.

It starts as a tiny login/session server.

Long-term, it becomes the server that runs the full Eldoria world, validates player actions, owns gameplay truth, loads content, persists player data, and supports a real private server community.

This document answers:

> How does ElServer grow from a tiny server into a complete authoritative game server?

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

ElServer is the authority of Eldoria.

ElClient displays the world.

ElForge creates and validates content.

ElServer decides what is true.

The final server should support:

- server startup and shutdown
- configuration
- logging
- networking
- login flow
- sessions
- account/player data
- authoritative world state
- tick processing
- movement validation
- region visibility
- entity synchronization
- chat
- inventory
- equipment
- object interactions
- NPC interactions
- skills
- combat
- banking
- trading
- shops
- dialogue
- quests
- content loading
- persistence
- admin/moderation tools
- debugging/monitoring
- deployment readiness

ElServer should respect RuneScape protocol concepts where practical, but it is primarily built to support Eldoria.

The long-term goal is a custom private server that can support real custom content and real players.

---

## Product Goal

ElServer is not finished when it accepts a socket connection.

ElServer is not finished when login works.

ElServer is finished enough for a serious server when it can support the full online game loop:

1. Start reliably.
2. Load configuration and content.
3. Accept client connections.
4. Authenticate/login players.
5. Create player sessions.
6. Own authoritative player/world state.
7. Process ticks consistently.
8. Validate movement and gameplay actions.
9. Synchronize world state to clients.
10. Persist player data.
11. Run core gameplay systems.
12. Support custom content exported by ElForge.
13. Provide admin/moderation tools.
14. Log and diagnose problems.
15. Recover from common failure cases.
16. Run long enough and stable enough for real players.

The final goal is not just a protocol experiment.

The final goal is an authoritative game server for Eldoria.

---

## Design Priorities

### Server Authority

ElServer owns gameplay truth.

Clients may request actions, but the server validates and applies them.

Examples:

- movement
- item actions
- object actions
- NPC actions
- combat
- skill progress
- trading
- banking

### Simple First, Correct Always

Early systems can be minimal, but they should respect the server-authoritative model.

A tiny login server is fine.

A client-trusting gameplay server is not.

### RuneScape Protocol Respect

ElServer should respect RuneScape protocol concepts where practical.

Examples:

- login flow
- packet structure
- opcode style
- ISAAC where applicable
- movement intent/update patterns

Perfect compatibility with every external RSPS is not the first goal.

Eldoria compatibility is the first goal.

### Deterministic Tick Model

The server should eventually run around a clear tick/update model.

Gameplay rules should be processed in a predictable order.

The server should avoid random scattered update logic.

### Data-Driven Content

ElServer should load content from data and ElForge exports where practical.

Hardcoding is acceptable for early probes, but long-term content should be data-driven.

### Observability

ElServer must be debuggable.

Logs, packet traces, session state, world state, and gameplay events should be inspectable during development.

### Production Reliability

ElServer should eventually handle real-world server concerns:

- disconnects
- bad packets
- malformed clients
- duplicate login
- persistence failures
- content mismatch
- backup needs
- admin commands
- moderation actions
- deployment configuration

---

## What ElServer Owns

ElServer owns server-specific authoritative behavior.

Examples:

- server startup and shutdown
- server configuration
- network listener
- client sessions
- login handling
- account/player session state
- authoritative world state
- tick loop
- server-side player state
- entity ownership
- gameplay validation
- persistence
- admin commands
- moderation actions
- server logs
- server diagnostics
- content loading for gameplay/runtime

---

## What ElServer Does Not Own

ElServer should not own shared systems or client/tool behavior.

Examples:

- cache reading belongs in `src/data/`
- packet definitions/codecs belong in `src/net/`
- shared world concepts belong in `src/world/`
- shared gameplay rules can belong in `src/game/`
- rendering belongs in `src/render/`
- client UI belongs in ElClient
- content editing belongs in ElForge
- raw platform services belong in `src/platform/`

ElServer combines shared systems into an authoritative online world.

---

## Planned App Structure

```text
apps/elserver/
├── main/
├── app/
├── config/
├── logging/
├── net/
├── session/
├── login/
├── tick/
├── world/
├── player/
├── entity/
├── persistence/
├── content/
├── game/
├── command/
├── admin/
├── security/
├── monitoring/
└── debug/
```

### `main/`

Application entry point.

### `app/`

Owns the server lifetime.

Examples:

- ElServerApp
- startup
- run loop
- shutdown
- fatal error handling

### `config/`

Server configuration.

Examples:

- port
- bind address
- content path
- persistence path
- tick rate
- debug flags

### `logging/`

Server logging and diagnostic output.

Examples:

- startup logs
- packet logs
- session logs
- gameplay logs
- error logs

### `net/`

Server-specific networking behavior.

Examples:

- listener
- accept connections
- read packets
- write packets
- disconnect clients
- dispatch packets to sessions

Shared packet definitions/codecs belong in `src/net/`.

### `session/`

Connected client session management.

Examples:

- ClientSession
- session id
- connection state
- authenticated state
- associated player id

### `login/`

Login flow and login validation.

Examples:

- login request
- login response
- duplicate login handling
- account lookup
- initial player load

### `tick/`

Server tick/update flow.

Examples:

- tick clock
- update order
- scheduled tasks
- tick timing diagnostics

### `world/`

Server-owned authoritative world state.

Examples:

- loaded regions
- players
- NPCs
- world objects
- ground items
- visibility

Shared world concepts belong in `src/world/`.

### `player/`

Server-side player runtime state.

Examples:

- position
- appearance
- movement queue
- inventory link
- equipment link
- session link

### `entity/`

Server runtime entity management.

Examples:

- player entities
- NPC entities
- entity ids
- visibility sets
- update queues

### `persistence/`

Saving and loading server data.

Examples:

- accounts
- player saves
- inventories
- equipment
- position
- settings
- backups

### `content/`

Server-side content loading.

Examples:

- definitions
- spawns
- shops
- dialogues
- object actions
- custom content exported by ElForge

### `game/`

Server integration with shared gameplay rules.

Examples:

- inventory actions
- object interactions
- NPC interactions
- skill actions
- combat actions

Shared rules may live in `src/game/`.

Server owns when and how those rules are applied.

### `command/`

Server commands.

Examples:

- developer commands
- admin commands
- teleport
- spawn
- reload content
- debug player

### `admin/`

Administration and moderation systems.

Examples:

- kick
- ban
- mute
- player lookup
- staff permissions

### `security/`

Basic protection against bad clients and abuse.

Examples:

- packet validation
- rate limits
- invalid state checks
- duplicate sessions
- malformed packet handling

### `monitoring/`

Runtime server health.

Examples:

- player count
- tick time
- packet rates
- memory stats
- persistence status

### `debug/`

Development diagnostics.

Examples:

- packet dump
- session dump
- world dump
- entity dump
- gameplay event trace

---

# Milestones

---

## Milestone 1 — Server Shell

### Goal

Create the first runnable ElServer application shell.

### Success Criteria

- ElServer builds.
- ElServer launches.
- Server startup logs appear.
- Server runs until stopped.
- Clean shutdown path exists.

### Feature Areas

- application lifecycle
- startup/shutdown
- run loop
- basic logging

### Issue Candidates

- Create `apps/elserver` target.
- Create ElServer application entry point.
- Create ElServerApp lifetime class.
- Add startup log output.
- Add basic run loop.
- Add shutdown signal handling.
- Add clean shutdown path.
- Add fatal error output.
- Add server version output.

### Proves

ElServer exists as a standalone application.

---

## Milestone 2 — Configuration and Logging Foundation

### Goal

Give ElServer basic runtime configuration and useful logs.

### Success Criteria

- Server can load configuration.
- Server has default configuration.
- Server logs startup state.
- Server logs errors clearly.
- Server configuration can control port/path/debug settings.

### Feature Areas

- config file
- defaults
- logging
- error reporting
- startup diagnostics

### Issue Candidates

- Define server config format.
- Add default config creation.
- Load server config file.
- Add port setting.
- Add bind address setting.
- Add content path setting.
- Add persistence path setting.
- Add debug logging flag.
- Add startup config summary.
- Add config validation errors.
- Add log level support.

### Proves

ElServer can be configured without hardcoding every runtime option.

---

## Milestone 3 — Network Listener

### Goal

Accept raw client connections.

### Success Criteria

- Server opens a listening socket.
- Server accepts a client connection.
- Server can detect disconnects.
- Server can shut down listener cleanly.
- Basic connection logs exist.

### Feature Areas

- listener
- connection accept
- disconnect detection
- connection registry
- socket error handling

### Issue Candidates

- Create server listener.
- Bind listener to configured port.
- Accept client connection.
- Assign temporary connection id.
- Store active connections.
- Detect client disconnect.
- Remove closed connection.
- Log new connection.
- Log disconnected connection.
- Shut down listener cleanly.
- Handle bind failure.

### Proves

ElServer can communicate with the outside world.

---

## Milestone 4 — Tiny Login Protocol

### Goal

Implement the smallest useful login flow.

### Success Criteria

- Server receives login request.
- Server validates basic login input.
- Server sends login success/failure.
- Duplicate or invalid attempts are handled.
- Packet flow can be logged.

### Feature Areas

- login packet handling
- connection state
- login response
- validation
- packet logging

### Issue Candidates

- Define login connection state.
- Receive login request packet.
- Decode login request.
- Validate username format.
- Validate password placeholder.
- Send login success response.
- Send login failure response.
- Handle login packet in wrong state.
- Handle malformed login packet.
- Add login packet log.
- Add failed login reason.

### Proves

ElServer can participate in the first client/server loop.

---

## Milestone 5 — Session Foundation

### Goal

Represent authenticated client sessions.

### Success Criteria

- Successful login creates a session.
- Session has a unique id.
- Session has connection state.
- Session can be associated with a player.
- Session is removed on disconnect.

### Feature Areas

- session registry
- authenticated state
- connection/session mapping
- lifecycle
- disconnect cleanup

### Issue Candidates

- Create ClientSession type.
- Create session id generator.
- Create session registry.
- Link connection to session.
- Create session on login success.
- Store username on session.
- Add authenticated state.
- Remove session on disconnect.
- Add session debug dump.
- Prevent duplicate active session for same username.

### Proves

ElServer can manage connected players beyond raw sockets.

---

## Milestone 6 — Account and Player Data Foundation

### Goal

Create basic account/player data handling.

### Success Criteria

- Player profile can be created.
- Player profile can be loaded.
- Player profile can be saved.
- Basic player fields exist.
- Failed persistence is handled.

### Feature Areas

- account identity
- player profile
- persistence format
- load/save
- error handling

### Issue Candidates

- Define account/player id concept.
- Define PlayerProfile data.
- Add default new player profile.
- Add player position field.
- Add appearance placeholder field.
- Add inventory placeholder field.
- Add file-based player save format.
- Load player profile on login.
- Save player profile on logout.
- Handle missing player save.
- Handle corrupt player save.
- Add player save debug log.

### Proves

ElServer can remember player state.

---

## Milestone 7 — Tick Loop Foundation

### Goal

Create the server update model.

### Success Criteria

- Server runs a clear tick loop.
- Tick rate is configurable.
- Tick duration can be measured.
- Tick order is defined.
- Slow ticks can be logged.

### Feature Areas

- tick clock
- update order
- timing
- diagnostics
- shutdown safety

### Issue Candidates

- Define server tick rate.
- Add tick clock.
- Add fixed tick loop.
- Add tick counter.
- Add tick duration measurement.
- Add slow tick warning.
- Define update order.
- Add per-tick session update hook.
- Add per-tick world update hook.
- Add graceful shutdown from tick loop.

### Proves

ElServer has a predictable runtime heartbeat.

---

## Milestone 8 — Server World Foundation

### Goal

Create the authoritative server world state.

### Success Criteria

- Server owns world state.
- Server can place players in the world.
- Server can load basic region/world data.
- Server can query world positions.
- Server world state is separate from client state.

### Feature Areas

- server world
- region state
- player placement
- world queries
- world loading

### Issue Candidates

- Create ServerWorld type.
- Add loaded region registry.
- Load starting region data.
- Add player spawn point.
- Add player to server world on login.
- Remove player from server world on logout.
- Query player position.
- Query region by coordinate.
- Add world debug dump.
- Add world load error handling.

### Proves

ElServer owns the world, not the client.

---

## Milestone 9 — Packet Handling Foundation

### Goal

Create clean server packet dispatch.

### Success Criteria

- Incoming packets are decoded.
- Packets are routed by connection state.
- Packets are dispatched to handlers.
- Unknown packets are handled safely.
- Packet errors do not crash the server.

### Feature Areas

- packet decode
- packet dispatch
- state-based handling
- unknown packet handling
- packet diagnostics

### Issue Candidates

- Create server packet dispatcher.
- Dispatch login-state packets.
- Dispatch game-state packets.
- Add unknown packet handler.
- Add malformed packet handler.
- Add packet size validation.
- Add packet direction validation.
- Add handler registration.
- Add packet trace option.
- Add packet error log.

### Proves

ElServer can grow packet handling without turning into a giant switch mess.

---

## Milestone 10 — Player Spawn and Initial Game State

### Goal

Send enough game state for ElClient to enter the world.

### Success Criteria

- Player enters game state after login.
- Server assigns player position.
- Server sends initial player state.
- Server sends initial region/world state placeholder.
- Client can display server-provided initial state.

### Feature Areas

- game-state transition
- initial player state
- initial region data
- initial server packets
- login-to-world flow

### Issue Candidates

- Add session game-state transition.
- Assign player spawn position.
- Send initial player info packet.
- Send initial position packet.
- Send initial region info packet.
- Send welcome/server message packet.
- Handle client ready/game-loaded packet.
- Add initial state debug log.
- Add failed initial state handling.
- Save initial login position.

### Proves

Login leads into a server-owned game world.

---

## Milestone 11 — Server-Authoritative Movement

### Goal

Validate and apply player movement on the server.

### Success Criteria

- Client sends movement intent.
- Server receives movement intent.
- Server validates path or destination.
- Server updates authoritative player position.
- Server sends confirmed movement/position.
- Invalid movement is rejected or corrected.

### Feature Areas

- movement intent packets
- walking queue
- collision validation
- path validation
- position updates
- desync diagnostics

### Issue Candidates

- Receive movement intent packet.
- Decode movement destination.
- Add server walking queue.
- Validate destination bounds.
- Validate collision for movement.
- Advance walking queue per tick.
- Update player position.
- Send position update packet.
- Reject invalid movement.
- Add movement debug log.
- Add desync correction packet.
- Add teleport/debug movement command.

### Proves

The first real online gameplay loop works.

---

## Milestone 12 — Region and Visibility System

### Goal

Track what each player should know about nearby world state.

### Success Criteria

- Server knows which region a player is in.
- Server tracks nearby entities.
- Server detects region changes.
- Server can decide what updates to send.
- Visibility logic is separated from gameplay logic.

### Feature Areas

- region membership
- visibility sets
- region changes
- nearby players
- nearby NPCs
- update scopes

### Issue Candidates

- Track player current region.
- Detect player region change.
- Create visibility set per player.
- Add nearby player lookup.
- Add nearby NPC lookup.
- Add nearby object lookup.
- Add visibility update step.
- Add enter visibility event.
- Add leave visibility event.
- Add visibility debug dump.
- Add region boundary debug logs.

### Proves

ElServer can scale beyond one player standing alone.

---

## Milestone 13 — Entity Synchronization

### Goal

Synchronize players, NPCs, objects, and ground items to clients.

### Success Criteria

- Nearby players are sent to clients.
- NPCs are sent to clients.
- Object changes are sent to clients.
- Ground items are sent to clients.
- Removed entities are removed on clients.
- Entity updates are batched or organized predictably.

### Feature Areas

- player updates
- NPC updates
- object updates
- ground item updates
- update batching
- removal updates

### Issue Candidates

- Send nearby player spawn/update packet.
- Send nearby player movement update.
- Send nearby player removal update.
- Add NPC entity type.
- Send NPC spawn/update packet.
- Send NPC movement update.
- Send NPC removal update.
- Add object update packet.
- Add ground item spawn/update packet.
- Add ground item removal packet.
- Batch entity updates per tick.
- Add entity sync debug log.
- Add entity update packet dump.

### Proves

ElServer can drive a multiplayer world that clients can mirror.

---

## Milestone 14 — Chat and Server Messages

### Goal

Add basic communication.

### Success Criteria

- Player can send chat.
- Server receives chat.
- Server broadcasts chat to nearby/global scope.
- Server can send system messages.
- Chat abuse basics can be added later.

### Feature Areas

- chat packet
- message broadcast
- message scope
- server messages
- moderation hooks

### Issue Candidates

- Receive chat message packet.
- Validate chat message length.
- Validate chat message characters.
- Broadcast chat to nearby players.
- Send system message packet.
- Add server welcome message.
- Add command prefix detection.
- Add chat debug log.
- Add mute check placeholder.
- Add chat rate limit placeholder.

### Proves

Clients can communicate through the server.

---

## Milestone 15 — Inventory Foundation

### Goal

Implement server-authoritative inventory.

### Success Criteria

- Player inventory exists.
- Items can be added.
- Items can be removed.
- Inventory state can be saved.
- Inventory updates are sent to client.
- Invalid item changes are rejected.

### Feature Areas

- inventory storage
- item stack rules
- add/remove
- persistence
- client updates
- validation

### Issue Candidates

- Create player inventory state.
- Define inventory slot count.
- Add item stack representation.
- Add add item operation.
- Add remove item operation.
- Add move item operation.
- Validate inventory slot.
- Validate item id.
- Save inventory in player profile.
- Load inventory from player profile.
- Send inventory update packet.
- Add inventory debug command.
- Add item spawn/test command.

### Proves

The server can own player item state.

---

## Milestone 16 — Equipment Foundation

### Goal

Implement server-authoritative equipment.

### Success Criteria

- Equipment slots exist.
- Items can be equipped.
- Items can be unequipped.
- Equipment affects appearance where applicable.
- Equipment state is saved.
- Equipment updates are sent to client.

### Feature Areas

- equipment slots
- equip rules
- unequip rules
- appearance updates
- persistence
- client updates

### Issue Candidates

- Create equipment state.
- Define equipment slots.
- Validate equip slot.
- Equip item from inventory.
- Unequip item to inventory.
- Reject invalid equipment.
- Update player appearance after equip.
- Save equipment in player profile.
- Load equipment from player profile.
- Send equipment update packet.
- Send appearance update packet.
- Add equipment debug command.

### Proves

The server can own player appearance-affecting gameplay state.

---

## Milestone 17 — Object Interaction Foundation

### Goal

Allow players to interact with world objects.

### Success Criteria

- Client can send object interaction.
- Server validates object exists.
- Server validates distance/reach.
- Server dispatches object action.
- Simple object actions can run.
- Result is sent to client.

### Feature Areas

- object interaction packet
- object lookup
- distance validation
- action dispatch
- simple actions
- feedback

### Issue Candidates

- Receive object interaction packet.
- Lookup world object by position/id.
- Validate object exists.
- Validate player distance to object.
- Validate interaction option.
- Create object action dispatcher.
- Add simple door/open test action.
- Add simple tree/chop test action.
- Send object interaction feedback.
- Add object interaction debug log.
- Add invalid object interaction handling.

### Proves

The server can process world interactions authoritatively.

---

## Milestone 18 — NPC Foundation

### Goal

Add server-owned NPCs.

### Success Criteria

- NPC definitions can be loaded.
- NPCs can spawn.
- NPCs can be synchronized to clients.
- NPCs can move or idle.
- NPC interaction can be received.

### Feature Areas

- NPC loading
- NPC spawning
- NPC state
- NPC updates
- NPC interaction
- basic behavior

### Issue Candidates

- Load NPC definitions.
- Define NPC spawn data.
- Spawn NPC in world.
- Add NPC to entity registry.
- Send NPC spawn to client.
- Add NPC idle state.
- Add NPC movement placeholder.
- Receive NPC interaction packet.
- Validate NPC interaction distance.
- Add NPC debug command.
- Add NPC despawn handling.

### Proves

The server can own non-player entities.

---

## Milestone 19 — Skill Foundation

### Goal

Implement basic skill progress.

### Success Criteria

- Player skills exist.
- XP can be added.
- Levels can be calculated.
- Skill state is saved.
- Skill updates are sent to client.
- At least one simple skill loop exists.

### Feature Areas

- skill state
- XP table
- level calculation
- persistence
- client updates
- simple skilling action

### Issue Candidates

- Create player skill state.
- Define XP table.
- Calculate level from XP.
- Add XP operation.
- Save skills in player profile.
- Load skills from player profile.
- Send skill update packet.
- Add debug add-XP command.
- Add simple woodcutting test action.
- Add skill action timing placeholder.
- Add skill failure/success message.

### Proves

The server can manage player progression.

---

## Milestone 20 — Combat Foundation

### Goal

Implement basic combat.

### Success Criteria

- Player can attack NPC.
- NPC can take damage.
- Hit timing exists.
- Death/respawn placeholder exists.
- Combat updates are sent to client.
- Combat remains server-authoritative.

### Feature Areas

- attack request
- target validation
- damage calculation
- hit timing
- death
- respawn
- combat updates

### Issue Candidates

- Receive attack NPC packet.
- Validate target NPC exists.
- Validate combat distance.
- Add combat state to player.
- Add combat state to NPC.
- Add basic attack timer.
- Calculate basic damage.
- Apply damage to NPC.
- Send hit update packet.
- Add NPC death placeholder.
- Add NPC respawn placeholder.
- Add combat debug log.
- Add stop combat action.

### Proves

The server can run the first real combat loop.

---

## Milestone 21 — Banking, Trading, and Shops

### Goal

Implement core economy interaction systems.

### Success Criteria

- Player can use a bank.
- Player can trade with another player.
- Player can buy/sell from shops.
- Item transfers are validated.
- State is persisted where needed.

### Feature Areas

- banking
- trading
- shops
- item transfer validation
- transaction safety
- client updates

### Issue Candidates

- Create bank state.
- Save/load bank state.
- Open bank interface packet.
- Deposit item.
- Withdraw item.
- Create trade session.
- Send trade request.
- Accept/decline trade.
- Add/remove trade offer item.
- Confirm trade.
- Validate trade item ownership.
- Create shop definition.
- Open shop packet.
- Buy item from shop.
- Sell item to shop.
- Validate shop transaction.
- Add transaction debug logs.

### Proves

The server can safely manage major item movement systems.

---

## Milestone 22 — Dialogue and Quest Foundation

### Goal

Support structured content interactions.

### Success Criteria

- Dialogue can be defined.
- NPC can start dialogue.
- Player can choose dialogue options.
- Quest state can exist.
- Quest progress can be saved.

### Feature Areas

- dialogue definitions
- dialogue state
- dialogue options
- quest state
- quest requirements
- quest rewards

### Issue Candidates

- Define dialogue data format.
- Load dialogue definitions.
- Start dialogue from NPC interaction.
- Send dialogue text to client.
- Send dialogue options to client.
- Receive dialogue option selection.
- Advance dialogue state.
- Create quest state storage.
- Save/load quest state.
- Add quest requirement check.
- Add quest reward operation.
- Add dialogue debug command.

### Proves

The server can support authored content beyond simple actions.

---

## Milestone 23 — Content Loading and ElForge Export Integration

### Goal

Load content produced or modified by ElForge.

### Success Criteria

- Server can load exported definitions.
- Server can load exported spawns.
- Server can load exported shops/dialogues where applicable.
- Server validates content on startup.
- Content version or metadata can be checked.

### Feature Areas

- exported content loading
- content metadata
- validation
- startup checks
- reload workflow

### Issue Candidates

- Load exported content metadata.
- Check content version.
- Load exported item definitions.
- Load exported NPC definitions.
- Load exported object definitions.
- Load exported NPC spawns.
- Load exported shop data.
- Load exported dialogue data.
- Validate missing model/definition references.
- Add startup content validation report.
- Add reload content command.
- Add content mismatch error handling.

### Proves

ElServer is integrated into the full Eldoria content pipeline.

---

## Milestone 24 — Admin and Developer Commands

### Goal

Provide tools to operate and debug the server.

### Success Criteria

- Server command system exists.
- Developer commands exist.
- Admin/moderation commands exist.
- Permission checks exist.
- Commands are logged.

### Feature Areas

- command registry
- permissions
- developer commands
- moderation commands
- logging

### Issue Candidates

- Create command registry.
- Add server console command input.
- Add player command packet handling.
- Add permission levels.
- Add teleport command.
- Add spawn item command.
- Add spawn NPC command.
- Add save player command.
- Add reload content command.
- Add kick command.
- Add mute command.
- Add ban placeholder.
- Add command audit log.

### Proves

ElServer can be operated and debugged without editing code every time.

---

## Milestone 25 — Security and Abuse Protection

### Goal

Protect the server from invalid clients and basic abuse.

### Success Criteria

- Malformed packets are handled.
- Packet spam can be limited.
- Invalid state transitions are rejected.
- Duplicate logins are handled.
- Dangerous commands require permissions.
- Suspicious behavior can be logged.

### Feature Areas

- packet validation
- rate limits
- state validation
- duplicate session handling
- permissions
- abuse logs

### Issue Candidates

- Add packet size limit checks.
- Add packet rate tracking.
- Add packet rate limit disconnect/warning.
- Reject packets in invalid connection state.
- Reject duplicate login.
- Add command permission validation.
- Log malformed packet.
- Log suspicious packet rate.
- Add invalid movement warning.
- Add suspicious action log.
- Add basic IP/session tracking.

### Proves

ElServer is not only functional, but safer to expose to real clients.

---

## Milestone 26 — Monitoring and Diagnostics

### Goal

Understand server health while it runs.

### Success Criteria

- Server can report player count.
- Server can report tick timing.
- Server can report packet rates.
- Server can report memory/content stats where practical.
- Diagnostics can be displayed or dumped.

### Feature Areas

- metrics
- tick diagnostics
- packet diagnostics
- session diagnostics
- world diagnostics
- dumps/reports

### Issue Candidates

- Add player count metric.
- Add session count metric.
- Add tick duration metric.
- Add average tick time.
- Add packet received count.
- Add packet sent count.
- Add packet rate metric.
- Add loaded region count.
- Add entity count metric.
- Add diagnostic dump command.
- Add server status command.
- Add periodic health log.

### Proves

ElServer can be monitored during development and future live use.

---

## Milestone 27 — Backup and Persistence Safety

### Goal

Protect player data.

### Success Criteria

- Player saves are not easily corrupted.
- Save failures are logged.
- Backups can be created.
- Shutdown saves active players.
- Persistence format limitations are documented.

### Feature Areas

- save safety
- backup
- shutdown save
- persistence errors
- data migration later

### Issue Candidates

- Add atomic player save strategy.
- Add save failure logging.
- Save all players on shutdown.
- Add periodic player save.
- Add manual save-all command.
- Add player save backup copy.
- Add corrupt save detection.
- Add restore-from-backup plan.
- Add persistence format version.
- Document persistence limitations.

### Proves

ElServer can protect player progress.

---

## Milestone 28 — Deployment Foundation

### Goal

Prepare ElServer to run outside the development environment.

### Success Criteria

- Server can run from a release folder.
- Config paths are clear.
- Content paths are clear.
- Logs are written to a predictable location.
- Startup failure messages are clear.
- Basic deployment checklist exists.

### Feature Areas

- release config
- runtime paths
- logs
- content paths
- startup validation
- deployment docs

### Issue Candidates

- Add release server config.
- Add runtime directory structure.
- Add logs directory.
- Add content directory.
- Add saves directory.
- Validate required folders on startup.
- Add clear startup failure messages.
- Add deployment README.
- Add server start script.
- Add server stop/shutdown instructions.
- Add firewall/port note.
- Add production checklist.

### Proves

ElServer can eventually run like a real server, not only inside the IDE.

---

## Milestone 29 — Performance and Scalability Pass

### Goal

Improve server performance and prepare for more players/content.

### Success Criteria

- Major bottlenecks are identified.
- Tick time is acceptable.
- Packet handling is not obviously wasteful.
- Entity updates are organized.
- Content loading is not repeatedly expensive.
- Server behavior remains correct after optimization.

### Feature Areas

- tick performance
- packet performance
- entity update optimization
- region lookup optimization
- content caching
- profiling

### Issue Candidates

- Profile tick loop.
- Profile packet dispatch.
- Profile movement validation.
- Profile visibility update.
- Profile entity synchronization.
- Cache repeated region lookups.
- Optimize visibility sets.
- Batch packet writes where practical.
- Add performance regression notes.
- Add stress test with simulated sessions.
- Add stress test with simulated entities.

### Proves

ElServer has a path toward supporting real player load.

---

## Milestone 30 — Production Readiness

### Goal

Prepare ElServer for real private server testing.

### Success Criteria

- Server runs reliably for long sessions.
- Accounts/player data persist correctly.
- Major gameplay systems work.
- Admin tools exist.
- Logs are useful.
- Backups exist.
- Deployment checklist exists.
- Known limitations are documented.
- Test plan exists.

### Feature Areas

- stability
- release workflow
- server operations
- backups
- monitoring
- admin tools
- documentation
- testing

### Issue Candidates

- Add long-running server test checklist.
- Add login/movement test checklist.
- Add persistence test checklist.
- Add gameplay smoke test checklist.
- Add admin command checklist.
- Add backup/restore checklist.
- Add deployment checklist.
- Add known limitations document.
- Add production config template.
- Add server version display.
- Add content version display.
- Add release notes template.
- Add public test readiness checklist.

### Proves

ElServer is moving from development server toward real private server infrastructure.

---

## Long-Term Completion Criteria

ElServer can be considered complete enough for a serious server when:

- It starts and shuts down reliably.
- It accepts client connections.
- It handles login correctly.
- It manages sessions safely.
- It owns authoritative player/world state.
- It runs a stable tick loop.
- It validates movement.
- It synchronizes world state to clients.
- It persists player data.
- It supports inventory and equipment.
- It supports object and NPC interactions.
- It supports skills and combat.
- It supports banking, trading, and shops.
- It supports dialogue and quest state.
- It loads custom content exported by ElForge.
- It has admin/developer commands.
- It handles bad packets and invalid clients safely.
- It has useful logs and diagnostics.
- It has backup and persistence safety.
- It can be deployed outside the dev environment.
- It is stable enough for real player testing.

---

## GitHub Issues Strategy

GitHub Issues should be generated from milestone issue candidates.

Recommended issue size:

> Small enough that one focused implementation can complete it.

Bad issue:

```text
Implement server
```

Better issues:

```text
Create server listener
Create ClientSession type
Receive login request packet
Create player profile save format
Send initial player position packet
Validate movement destination bounds
```

Recommended labels:

- `elserver`
- `net`
- `world`
- `game`
- `persistence`
- `login`
- `movement`
- `entity-sync`
- `content`
- `admin`
- `security`
- `monitoring`
- `deployment`
- `bug`
- `cleanup`

---

## Relationship to Other Documents

### `architecture.md`

Defines where server code belongs.

### `roadmap.md`

Defines the global build order.

### `elclient.md`

Defines the player-facing client ElServer supports.

### `elforge.md`

Defines the content creation tool that produces content ElServer may load.

---

## Status

This document is the current ElServer roadmap baseline.

It is expected to evolve as ElServer becomes real.
