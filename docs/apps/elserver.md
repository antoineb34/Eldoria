# ElServer

## Purpose

ElServer is the authoritative game server for Eldoria.

It owns online truth.

Clients may request actions.

The server validates and applies those actions.

ElServer is the source of authority for online gameplay.

This does not mean ElServer owns the reusable `world/`, `game/`, `net/`, or `data/` modules.

ElServer composes those shared modules into an authoritative runtime.

---

## Current State

ElServer currently exists as an application foundation.

Its current purpose is to provide a stable location for future server systems.

Implementation is intentionally minimal during early milestones.

---

## Long-Term Direction

ElServer should eventually support:

* login
* sessions
* account management
* authoritative world state
* entity management
* NPC behavior
* movement validation
* combat
* inventory management
* persistence
* administration
* production server operation

ElServer should become the authoritative runtime for all online gameplay.

---

## Ownership

ElServer owns server-specific authority and runtime behavior.

Examples:

* gameplay authority
* online world authority
* account authority
* persistence
* simulation execution
* validation execution
* session management
* entity synchronization behavior
* administration behavior

ElServer does not own reusable shared systems.

Examples:

* static data loading belongs in `src/data/`
* spatial representation belongs in `src/world/`
* reusable gameplay rules belong in `src/game/`
* packet definitions/codecs belong in `src/net/`
* rendering belongs in `src/render/`
* player-facing UI belongs in ElClient
* editor workflows belong in ElForge
* content creation tools belong in ElForge

---

## Architectural Position

ElServer composes shared systems into an authoritative runtime.

```text
data
    ↓
world
    ↓
game
    ↓
net
    ↓
ElServer authority/runtime
```

ElServer consumes systems.

It should not reimplement them.

---

## Future Workflows

### Login

```text
Login Request
    ↓
Validation
    ↓
Session Creation
    ↓
World Entry
```

---

### Movement

```text
Movement Request
    ↓
Validation
    ↓
World Update
    ↓
Synchronization
```

---

### Combat

```text
Combat Request
    ↓
Rule Validation
    ↓
Simulation
    ↓
Synchronization
```

---

### Persistence

```text
Authoritative State
    ↓
Persistence
    ↓
Storage
```

---

## Relationship To Systems

ElServer will consume systems such as:

```text
Networking
World State
Entity Management
Combat Rules
Persistence
Synchronization
```

ElServer should orchestrate systems.

It should not replace them.

---

## Common Mistakes

Do not:

* move gameplay authority into ElClient
* move world authority into ElClient
* move persistence into ElClient
* make shared modules depend on ElServer
* duplicate simulation logic inside app code when it belongs in shared systems
* define reusable packet language inside ElServer when it belongs in `net/`
* decode cache data inside ElServer when it belongs in `data/`

ElServer owns online truth.

Clients consume that truth.

Shared modules define reusable building blocks.

---

## Golden Rule

ElServer owns online authority.

Shared modules own reusable systems.

Everyone else observes or tools around that truth.
