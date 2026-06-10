# data/texture

Texture asset types, loading, and decoding.

## Ownership

This module owns:
- texture asset types (`TextureAsset`, `TextureFile`, `TexturePalette`, `TextureMetadata`)
- texture file reading (`TextureFileReader`)
- texture decoding (`TextureCanvasDecoder`, `TexturePixelDecoder`)
- texture building (`TextureBuilder`)
- texture loading from cache (`TextureLoader`)

## Public API

Consumers should use:

```cpp
#include "texture/TextureApi.h"    // asset types (TextureAsset, TextureFile, etc.)
#include "texture/TextureLoader.h" // loading textures from cache
```

Internal headers (`decoder/*`, `TextureBuilder.h`, `TextureFileReader.h`) are not part of the public API and may change without notice.

## Namespace

All types live in `rf::texture`.

## Dependency Rule

`data/texture` depends on:
- `data/cache` (for `Cache`, `ArchiveReader`)
- `data/binary` (for `ByteBuffer`)

`data/texture` does not depend on `core`, `render`, `render_next`, or any application.

## Migration Notes

Texture code was migrated from `src/core/assets/texture` to `src/data/texture` as part of the M5 texture ownership migration (issues #88, #89, #90). The namespace `rf::texture` was preserved for continuity.
