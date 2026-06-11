# Net Module

## Purpose

`src/net/` owns networking protocol systems.

It answers:

```text
How do state and actions travel?
```

The net module defines the shared language used by ElClient and ElServer.

---

## Current State

The net module currently exists as a shared architectural module.

Implementation is intentionally minimal.

Current dependency shape:

```text
net
    depends on
        data
        world
```

The module exists to provide a dedicated home for future protocol systems.

---

## Direction

Future protocol systems should live under `src/net/`.

Examples:

```text
src/net/packet/
src/net/codec/
src/net/protocol/
src/net/login/
src/net/update/
```

If a feature is about packets, encoding, decoding, or protocol structure, it probably belongs in net.

---

## What Belongs Here

Examples:

```text
Packet definitions
Packet codecs
Protocol states
Login protocol
Update protocol
Packet readers
Packet writers
```

Questions that belong to net:

```text
How is a packet encoded?
What opcode represents movement?
How is a login request decoded?
```

---

## What Does Not Belong Here

Examples:

```text
Combat
Inventory
Rendering
SDL
Cache decoding
Client UI
Server authority
```

Questions that do NOT belong to net:

```text
Can a player attack?
How is a model rendered?
Can an item be equipped?
```

---

## Future Structure

Likely shape:

```text
src/net/
├── packet/
├── codec/
├── protocol/
├── login/
└── update/
```

Structure may evolve as implementation grows.

---

## Dependency Rules

May depend on:

* data
* world

Should not depend on:

* render
* game
* apps

Applications may depend on net.

Net should not depend on applications.

---

## Boundary Examples

```text
net/packet/MovementPacket
= protocol message

game/player/MovementRules
= gameplay validation

apps/elserver/session
= authoritative handling
```

```text
net/login/LoginRequest
= protocol structure

apps/elclient/login
= client workflow

apps/elserver/login
= server workflow
```

---

## Common Mistakes

Do not:

* put gameplay rules in net
* put rendering behavior in net
* put session ownership in net
* put client UI in net
* put server authority in net
* make net depend on applications

---

## When Adding New Code

Ask:

```text
Is this about protocol structure or packet transport?
```

If yes, it probably belongs in net.

If it is about gameplay, it probably belongs in game.

---

## Golden Rule

Net owns protocol language.

Applications own protocol behavior.
