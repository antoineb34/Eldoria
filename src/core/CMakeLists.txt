# The core module previously owned texture code and other legacy assets.
#
# All owned code has been migrated to the appropriate data/* modules:
#   - Texture types, loading, and decoding -> src/data/texture
#   - Map decoding, references_rs317 -> legacy reference code, not yet migrated
#
# This target is retained only for the legacy reference headers under
# src/core/references_rs317 and src/core/assets/map/decoder. These are
# RuneScape 317 reference definitions used for validation. Once those
# reference files are no longer needed or are migrated, this target
# can be removed.
#
# No active build target links this module. It is kept as a header-only
# INTERFACE target so the include path remains available for the legacy
# reference headers.

add_library(core INTERFACE)

target_include_directories(core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
