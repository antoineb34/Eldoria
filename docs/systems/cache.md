# Cache System

## Purpose

The cache system reads RuneScape-style cache files from disk and returns raw payloads or archive files for higher-level data systems.

It answers:

```text
How do DAT/IDX cache files become usable byte payloads?
```

The cache system belongs to:

```text
src/data/cache/
```

---

## Current State

The cache system currently supports:

* cache directory validation
* DAT/IDX file access
* cache index lookup
* sector-chain reading
* sector header validation
* raw cache file payload extraction
* cache file listing by index
* archive decoding
* archive file lookup by hash or index

Current files:

```text
src/data/cache/
├── Archive.cpp
├── Archive.h
├── ArchiveHashes.h
├── ArchiveReader.cpp
├── ArchiveReader.h
├── Cache.cpp
├── Cache.h
└── CacheTypes.h
```

---

## Data Flow

### Raw Cache File

```text
Cache root directory
    ↓
main_file_cache.idxN
    ↓
CacheIndexEntry
    ↓
main_file_cache.dat
    ↓
sector chain
    ↓
CacheFile payload
```

### Archive File

```text
CacheFile payload
    ↓
ArchiveReader
    ↓
Archive
    ↓
ArchiveFile
```

---

## Important Types

### `CacheIndex`

Defines known cache indexes.

Current mapping:

```text
Config    = 1
Model     = 2
Animation = 3
Midi      = 4
Map       = 5
```

The enum value maps to cache files by subtracting one:

```text
CacheIndex::Config -> main_file_cache.idx0
CacheIndex::Model  -> main_file_cache.idx1
```

---

### `CacheIndexEntry`

Represents one 6-byte IDX entry.

Contains:

```text
size
firstSector
```

This tells the cache how many bytes to read and where the sector chain begins.

---

### `CacheSectorHeader`

Represents one 8-byte sector header.

Contains:

```text
fileId
chunkId
nextSector
index
```

The header is validated while reading a file.

---

### `CacheFile`

Represents one raw cache file payload.

Contains:

```text
id
index
entry
payload
```

Higher-level systems use `payload`.

---

### `Archive`

Represents a decoded archive containing multiple files.

Archive files may be found by:

```text
hash
file index
```

---

## Important Classes

### `Cache`

Main cache access class.

Public API:

```text
isValid()
hasFile(index, fileId)
readFile(index, fileId)
listFiles(index)
```

Responsibilities:

* locate DAT and IDX files
* validate cache directory
* read index entries
* follow sector chains
* validate sector headers
* return raw cache payloads

---

### `ArchiveReader`

Reads an archive from a raw cache file payload.

Responsibilities:

* read archive header
* decompress archive payload when needed
* read file metadata
* extract archive file payloads
* decompress individual files when needed
* return `Archive`

---

## Cache File Reading

`Cache::readFile()` is the main raw file loading path.

Flow:

```text
read index entry
    ↓
open main_file_cache.dat
    ↓
seek to first sector
    ↓
read sector header
    ↓
validate sector header
    ↓
read sector payload
    ↓
follow next sector
    ↓
repeat until payload size reached
    ↓
return CacheFile
```

If anything fails, the function returns:

```text
std::nullopt
```

---

## Sector Format

Current implementation uses:

```text
Index entry size   = 6 bytes
Sector size        = 520 bytes
Sector header size = 8 bytes
Sector payload     = 512 bytes
```

Sector header layout:

```text
u16 fileId
u16 chunkId
u24 nextSector
u8  index
```

Index entry layout:

```text
u24 size
u24 firstSector
```

---

## Archive Reading

`ArchiveReader::read()` turns a cache file payload into an `Archive`.

Flow:

```text
CacheFile payload
    ↓
read archive header
    ↓
decode archive payload
    ↓
read file count
    ↓
read file metadata table
    ↓
extract each file payload
    ↓
return Archive
```

Archive-level or file-level payloads may be compressed.

Compression is handled by:

```text
data/binary/Compression
```

---

## Ownership

The cache system owns:

* DAT/IDX access
* index entry parsing
* sector parsing
* sector validation
* raw payload reconstruction
* archive parsing
* archive decompression orchestration

The cache system does not own:

* model decoding
* texture decoding
* map decoding
* gameplay data
* rendering
* UI behavior
* application workflows

---

## Extension Points

Add cache-level behavior here when the behavior is about:

* reading cache files
* validating cache structures
* archive parsing
* archive lookup
* cache diagnostics
* raw cache file enumeration

Do not add asset-specific decoding here.

Examples:

Good:

```text
cache file integrity checks
archive lookup helpers
cache index diagnostics
```

Bad:

```text
model vertex decoding
texture palette decoding
map object spawn decoding
ElForge tree UI behavior
```

---

## Common Mistakes

Do not:

* decode models in `Cache`
* decode textures in `Cache`
* decode maps in `Cache`
* add ElForge UI behavior to `data/cache/`
* make cache depend on render, world, game, net, or apps
* bypass `Cache::readFile()` for normal DAT/IDX reads
* duplicate sector-reading logic elsewhere

---

## When Adding New Code

Before changing the cache system:

1. Determine whether the change is about raw cache access or higher-level data decoding.
2. Keep raw DAT/IDX logic in `data/cache/`.
3. Keep asset-specific parsing in the relevant data submodule.
4. Reuse `Cache::readFile()` when loading cache files.
5. Reuse `ArchiveReader` when reading archive payloads.
6. Avoid duplicating binary parsing logic outside this system.

---

## Verification

Useful verification steps:

```bash
cmake --build build
```

Manual checks:

```text
Open ElForge.
Verify cache browsing still works.
Verify known model loading still works.
Verify texture loading still works if archive behavior changed.
```

If sector reading changes, verify multiple cache indexes:

```text
Config
Model
Animation
Midi
Map
```

---

## Golden Rule

The cache system reads bytes correctly.

It does not decide what those bytes mean as models, textures, maps, or gameplay data.
