# ElServer

## Purpose

ElServer is the authoritative game server for Eldoria.

It owns the game world and determines what is true.

Clients may request actions.

The server validates and applies those actions.

ElServer is the source of authority for gameplay.

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
* world simulation
* entity management
* NPC behavior
* movement validation
* combat
* inventory management
* persistence
* administration
* production server operation

ElServer should become the authoritative runtime for all gameplay.

---

## Ownership

ElServer owns:

* gameplay authority
* world authority
* account authority
* persistence
* game rules
* simulation
* validation
* entity synchronization
* administration

ElServer does not own:

* rendering
* player-facing UI
* editor workflows
* content creation tools

Those responsibilities belong to ElClient and ElForge.

---

## Architectural Position

ElServer composes shared systems into an authoritative runtime.

```text
ElServer
    ↓
game
    ↓
world
    ↓
net
    ↓
data
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
World State
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
World Simulation
Entity Management
Combat
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
* duplicate simulation logic

ElServer owns the truth.

Clients consume that truth.

---

## Golden Rule

ElServer owns the world.

Everyone else observes it.
