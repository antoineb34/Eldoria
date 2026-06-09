# net

Purpose: owns how state and actions travel.

The net module is responsible for protocol concepts, packet definitions, serialization boundaries, session-facing abstractions, and shared networking data structures.

Dependency rule: `net` should define communication contracts without owning gameplay rules. ElServer validates actions through `game`; ElClient uses networking to request actions and receive authoritative state.
