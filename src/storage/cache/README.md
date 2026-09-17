# cache

## Purpose

The `cache` subsystem provides structured access to Eldoria’s RuneScape cache.

It is responsible for:

* opening the cache
* accessing individual indexes
* finding and listing files
* reading sector chains
* returning stored or decompressed bytes
* creating files
* updating files safely

---

## Mental Model

The cache is represented as:

```text
Cache
    └── Store
          └── File
                └── Payload
                      └── Sector
```

### Cache

`Cache` represents the complete cache directory.

It knows about:

```text
main_file_cache.dat
main_file_cache.idx0
main_file_cache.idx1
main_file_cache.idx2
main_file_cache.idx3
main_file_cache.idx4
```

It validates that those files exist and opens one index as a `Store`.

### Store

`Store` represents one cache index.

The current index mapping is:

```text
idx0 = Config
idx1 = Models
idx2 = Animations
idx3 = Midi
idx4 = Maps
```

A store provides the main file-level API:

* `get`
* `find`
* `list`
* `contains`
* `count`
* `create`
* `update`

### File

`File` represents one logical file inside a store.

It contains:

* the file ID
* the index entry
* the compression type
* the payload

A file can return:

* the bytes exactly as stored
* the decompressed bytes

### Payload

`Payload` owns the ordered sectors that make up a file.

### Sector

`Sector` represents one physical block in the DAT file.

Each sector is 520 bytes:

```text
8-byte header
512-byte data region
```

The header stores:

```text
2 bytes: file ID
2 bytes: chunk ID
3 bytes: next sector ID
1 byte: index ID
```

---

## Index Entries

Each file has a six-byte entry inside its IDX file:

```text
3 bytes: file size
3 bytes: first sector ID
```

The first-sector ID points to the beginning of the file’s sector chain.

An entry is valid when both values are nonzero.

```text
size = 0 and firstSector = 0
    empty entry

size > 0 and firstSector > 0
    valid file

only one value is zero
    malformed entry
```

Malformed entries are rejected rather than treated as free space.

---

## Reading a File

Reading follows this path:

```text
IDX entry
    ↓
first sector
    ↓
sector chain
    ↓
Payload
    ↓
File
```

The reader:

1. reads the index entry
2. calculates how many sectors are expected
3. follows the sector chain
4. validates every sector
5. assembles the stored bytes
6. detects compression
7. returns a `File`

Each sector must contain:

* the expected file ID
* the expected chunk ID
* the expected index ID

The chain must also end exactly where the declared file size says it should.

---

## File Identity

A file is identified by:

```text
IndexId + fileId
```

For example:

```text
Models + 123
```

That logical identity stays stable when the file is updated.

Sector IDs and DAT offsets are only physical storage details and may change.

---

## Creating a File

Creating a file works like this:

```text
source bytes
    ↓
compression
    ↓
choose an available file ID
    ↓
append a new sector chain
    ↓
flush the DAT file
    ↓
write the IDX entry
```

The writer reuses the first completely empty IDX entry.

If none exists, it appends a new entry.

Malformed entries are never reused.

---

## Updating a File

Updating keeps the same file ID but replaces its physical sector chain.

```text
replacement bytes
    ↓
compression
    ↓
append a complete new sector chain
    ↓
flush the DAT file
    ↓
update the existing IDX entry
```

The original chain is not overwritten in place.

This means the old file stays valid until the replacement has been written successfully.

After the IDX entry is updated, the old chain becomes unreachable.

---

## Why Updates Append

In-place updates are unsafe because the replacement may:

* be larger
* require more sectors
* compress to a different size
* fail partway through writing

Appending the replacement first avoids destroying the original valid file.

The cost is that old sector chains remain in the DAT file until a future compactor removes them.

---

## Compression

The cache uses the binary subsystem for compression.

Supported types are:

```text
None
Gzip
Bzip2
```

Files are written from uncompressed source bytes and compressed according to the requested type.

When reading, the detected compression type is stored in the `File`.

Headerless RuneScape Bzip2 cannot be reliably detected from bytes alone and must be handled by format-aware higher-level code.

---

## Current Limitations

The cache currently does not support:

* deletion
* sector reuse
* compaction
* concurrent writes
* write locking
* full DAT/IDX transactions
* automatic repair

These are intentionally excluded until they can be designed safely.

---

## Future Compaction

A future compactor will rebuild the physical cache using only live files.

It would:

1. read every live file
2. assign new contiguous sector IDs
3. write a new DAT file
4. rebuild every IDX file
5. validate the new cache
6. replace the old files

Compaction may change sector positions, but it must preserve:

```text
IndexId + fileId
```

---

## Ownership Boundary

The cache subsystem owns:

* DAT and IDX interpretation
* index entries
* sector chains
* stored file bytes
* compression state
* file creation
* file updates

It does not own:

* archive contents
* model decoding
* texture decoding
* map decoding
* rendering
* gameplay
* networking
* application UI

Higher-level loaders use the cache to obtain bytes, then decode those bytes according to the asset format.

---

## Source Structure

```text
Cache
    complete cache directory

Store
    one cache index

File
    one logical cache file

Payload
    ordered sectors belonging to a file

Sector
    physical DAT block

Index
    index IDs and index entries

Reader
    reading and validation

Writer
    creation and safe replacement
```

---

## Safety

Never test writes against the only copy of a cache.

The DAT file is shared by every index, so damage to it can affect the entire cache.
