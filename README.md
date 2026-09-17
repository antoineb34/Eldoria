# Eldoria

Eldoria is a C++20 RuneScape-317 game ecosystem built around three
applications sharing one set of reusable modules:

- **ElForge** — asset inspection/content tooling
- **ElClient** — player-facing game client
- **ElServer** — authoritative gameplay server

## Architecture

```text
data        static source content + decoding
    |
    +------> graphics -----> render
    |
    +------> audio
    |
    +------> world --------> game
               |
               +-----------> network

platform     machine/library integration

apps/
    elforge
    elclient
    elserver
```

### Module ownership

| Module | Owns |
|---|---|
| `data` | Cache/archive access, source formats, decoded static assets |
| `graphics` | Source visual assets -> normalized graphics resources |
| `render` | Camera, scenes, rendering backends, pixels |
| `audio` | Playback and runtime audio processing |
| `world` | Shared spatial/world state |
| `game` | Reusable gameplay rules |
| `network` | Client/server communication contracts |
| `math` | Shared mathematical primitives |
| `platform` | SDL/OS/library integration |
| `apps` | Application orchestration and presentation |

Runtime applications may compose shared modules, but shared modules must not
depend back on runnable apps.

## Data status

The production data layer handles the main 317 cache structures used by the
game: cache storage, archives, definitions, models, animations, maps,
interfaces, sprites/images/textures/fonts, and MIDI.

Remaining legacy source-format work:

- chat-filter (`wordenc`) data
- classic sound effects
- version/checksum manifest metadata

Old reverse-engineering/reference implementations are deliberately not kept
inside production `src/`; the production decoders are now the source of truth.

## Build

```bash
cmake -B build
cmake --build build -j"$(nproc)"

./build/bin/elforge
./build/bin/elclient
./build/bin/elserver
```

Individual targets:

```bash
cmake --build build --target elforge
cmake --build build --target elclient
cmake --build build --target elserver
```

## Development direction

The foundation is being frozen before substantial ElClient work begins.
Architecture changes after that point should be driven by concrete
client/server requirements rather than speculative reorganization.
