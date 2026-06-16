# Phase 3 - Client Foundation

## Purpose

Phase 3 builds the foundation of the player-facing Eldoria client.

The goal is not full gameplay.

The goal is not real world rendering.

The goal is to create a stable client application architecture that can eventually connect to ElServer, render the game world, display interfaces, receive player input, and present server-authoritative gameplay.

ElClient should become the player-facing product shell.

Later phases fill that shell with real data, world representation, networking, and gameplay.

---

## Outcome

Phase 3 is complete when ElClient has:

* a stable application lifecycle
* screen management
* client state ownership
* input handling foundation
* render loop foundation
* network/bootstrap placeholders
* login flow skeleton
* clear ownership boundaries between ElClient and shared modules
* documentation explaining the client architecture

This phase creates the container for the future player experience.

Later phases fill it with real world content.

---

## Core Rule

ElClient presents the game.

ElServer owns the online game truth.

Shared modules own reusable systems.

```text
data
    ↓
world
    ↓
render
    ↓
ElClient presentation
```

ElClient should not decode cache data, define world representation, implement gameplay rules, or replace shared rendering systems.

---

## Current Architectural Realization

A playable client cannot become meaningful until shared data and world systems can describe something real.

Phase 3 may include placeholders or diagnostic render paths, but those paths should remain temporary verification aids.

The real path is:

```text
Phase 3
    Client shell

Phase 4
    Data expansion

Phase 5
    World foundation

Phase 6
    Client world visualization
```

Do not over-invest in fake login/gameplay flows before the shared data/world foundation exists.

---

# 3.1 Client Application Lifecycle

## Purpose

Create a stable client runtime.

## Required Implementation

* client application entry point
* initialization flow
* update loop
* render loop
* shutdown flow
* error handling for failed initialization

## Required Integration

* use `platform/` for machine integration
* use `render/` for rendering where needed
* keep client workflow inside `apps/elclient/`

## Not Included

Not required:

* gameplay
* world simulation
* map loading
* server authority
* persistence

## Exit Criteria

* ElClient starts cleanly
* ElClient updates cleanly
* ElClient renders cleanly
* ElClient shuts down cleanly
* lifecycle behavior is understandable from code

---

# 3.2 Screen Management

## Purpose

Create the screen/state structure required for a player-facing client.

## Required Implementation

Expected screens:

```text
Startup Screen
Login Screen
Game Screen
Disconnected/Error Screen
```

Required behavior:

* screen enum or screen state type
* screen transition mechanism
* per-screen update/render behavior
* clear ownership of screen-specific state

## Required Integration

* screen state lives in ElClient app state
* screens may use shared modules
* shared modules should not know about client screens

## Not Included

Not required:

* final UI art
* final login protocol
* real gameplay UI
* full interface system
* real world rendering

## Exit Criteria

* ElClient can enter a startup screen
* ElClient can transition to login screen
* ElClient can transition to game screen placeholder
* screen flow is testable

---

# 3.3 Client State

## Purpose

Define where client runtime state lives.

## Required Implementation

Client state should represent:

* current screen
* connection state placeholder
* session state placeholder
* input state placeholder
* render state placeholder
* loading/error status
* currently visible world placeholder or future snapshot reference

## Required Integration

* app code owns client state
* systems consume explicit inputs
* shared modules do not depend on ElClient state

## Not Included

Not required:

* full world state
* full entity state
* prediction system
* inventory state
* combat state

## Exit Criteria

* client state has a clear home
* screen state is centralized
* connection/session placeholders exist
* future systems have obvious places to attach
* state does not duplicate shared data/world systems

---

# 3.4 Input Foundation

## Purpose

Create a clean foundation for player input.

## Required Implementation

* SDL event handling
* keyboard input
* mouse input
* input state update
* route input to active screen

## Required Integration

* platform integration remains in `platform/`
* client-specific input mapping lives in ElClient
* gameplay requests should eventually be sent to ElServer

## Not Included

Not required:

* final keybind system
* combat input
* inventory dragging
* camera polish
* click-to-move gameplay

## Exit Criteria

* input is processed in ElClient
* active screen can respond to input
* input does not bypass screen/client state
* input handling is ready for future gameplay requests

---

# 3.5 Rendering Integration

## Purpose

Connect ElClient to the existing render foundation.

The goal is not final world rendering.

The goal is establishing where and how the client renders.

## Required Implementation

* render initialization
* frame rendering hook
* clear screen behavior
* placeholder or diagnostic game rendering
* render ownership rules

## Required Integration

* use `render/` for rendering systems
* use `platform/` for SDL/window integration
* keep screen-specific presentation in ElClient
* avoid duplicating render pipeline behavior

## Not Included

Not required:

* complete world renderer
* complete interface renderer
* animation rendering
* minimap
* final camera system
* map terrain rendering

## Exit Criteria

* ElClient has a render path
* screen rendering works
* game screen placeholder or diagnostic render works
* render responsibilities are clear
* diagnostic render code does not become permanent world architecture

---

# 3.6 Network Bootstrap

## Purpose

Create the first foundation for future client/server communication.

The goal is not the full gameplay protocol.

The goal is establishing connection structure and future protocol ownership.

## Required Implementation

* connection state placeholder
* network client placeholder or skeleton
* connect/disconnect flow placeholder
* error handling placeholder
* protocol ownership notes

## Required Integration

* shared packet/protocol structures should live in `net/`
* client workflow lives in `apps/elclient/`
* server workflow lives in `apps/elserver/`

## Not Included

Not required:

* full login protocol
* encryption
* entity synchronization
* gameplay packets
* persistence

## Exit Criteria

* ElClient has a defined place for future networking behavior
* connection state can be represented
* connection attempt flow can be attached later
* net/app boundary is clear

---

# 3.7 Login Flow Skeleton

## Purpose

Create the first player-facing login workflow skeleton.

The goal is not authentication security.

The goal is creating the UI and state flow that future login protocol work will use.

## Required Implementation

* login screen skeleton
* username/password input placeholders
* login button
* login state
* failed login state placeholder
* transition to game placeholder on mock success

## Required Integration

* login screen uses client state
* future login packets belong in `net/`
* future validation belongs in ElServer

## Not Included

Not required:

* real authentication
* account database
* encryption
* production login security
* real gameplay behind login

## Exit Criteria

* login screen exists
* user can enter placeholder credentials
* login action changes state
* game screen placeholder can be reached
* boundaries between UI, network, and server authority are clear

---

# 3.8 Client Documentation

## Purpose

Document ElClient as an application and document its early architecture.

## Required Documentation

```text
docs/apps/elclient.md
```

Potential future system docs:

```text
docs/systems/client-screen-flow.md
docs/systems/client-network-bootstrap.md
docs/systems/login-flow.md
docs/systems/client-world-visualization.md
```

Only create system documents when the workflow becomes complex enough to need them.

## Required Updates

Update documentation when ElClient gains:

* new screens
* new client workflows
* new network behavior
* new render behavior
* new ownership boundaries
* permanent state flow changes

---

## Not Included In Phase 3

Phase 3 does not require:

* real gameplay
* real map loading
* authoritative world simulation
* inventory system
* combat system
* skills
* quests
* full interface system
* multiplayer synchronization
* production authentication

Those belong to later phases.

Phase 3 creates the client structure those systems will use.

---

## Phase 3 Completion Criteria

Phase 3 is complete when:

* ElClient starts, updates, renders, and shuts down cleanly
* screen management exists
* client state ownership is clear
* input foundation exists
* rendering integration exists
* network/bootstrap placeholders exist
* login flow skeleton exists
* shared-system boundaries are clear
* client architecture is documented

---

## Golden Rule

Phase 3 builds the client shell.

ElClient presents gameplay.

Shared systems describe gameplay reality.

ElServer owns online gameplay truth.
