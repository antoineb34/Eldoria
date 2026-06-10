#pragma once

// Public texture API for Eldoria.
//
// Ownership: src/data/texture
//
// Consumers should include this header to access texture asset types.
// For texture loading, include "texture/TextureLoader.h".
//
// Internal decoder and builder headers are not part of the public API
// and may change without notice.

#include "texture/TextureAsset.h"
#include "texture/TextureFile.h"
