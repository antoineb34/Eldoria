#!/usr/bin/env bash

set -euo pipefail

ROOT="$(
    cd "$(
        dirname "${BASH_SOURCE[0]}"
    )/.."
    pwd
)"

cd "$ROOT"

DOC="docs/procedural-weapon-pipeline.md"

if [[ ! -f "$DOC" ]]; then
    echo
    echo "ERROR: documentation file does not exist:"
    echo "  $DOC"
    echo
    exit 1
fi


SECTION_MARKER="# 40. Awakened Moonstone Scimitar - Tier 2"


if grep -Fq "$SECTION_MARKER" "$DOC"; then
    echo
    echo "Awakened Moonstone section is already documented."
    echo "No changes made."
    exit 0
fi


BACKUP="/tmp/procedural-weapon-pipeline.before-awakened.md"

cp \
    "$DOC" \
    "$BACKUP"


cat >> "$DOC" <<'DOCBLOCK'


---

# 40. Awakened Moonstone Scimitar - Tier 2

## Status

The second major procedural weapon experiment is the:

~~~text
Awakened Moonstone Scimitar
~~~

This weapon represents the first evolution of the successful Moonstone
Scimitar V3.

The purpose of this experiment was to determine whether the procedural weapon
pipeline could handle a substantially more complicated concept while
preserving:

- RuneScape-style visual readability,
- intentional low-poly topology,
- correct shading behavior,
- correct worn positioning,
- semantic components,
- and the visual identity of the original weapon family.

The experiment was successful.

The Awakened weapon confirmed that the Moonstone V3 result was not merely a
one-off success.

---

# 41. Evolution Goal

The original Moonstone Scimitar V3 proved that a concept image could become a
convincing custom RuneScape-style weapon.

Tier 2 deliberately increased complexity.

The Awakened concept introduced:

~~~text
large outer crescent blade
secondary inner crescent / hook
large negative-space opening
suspended moonstone crystal core
floating crystal shards
larger winged guard
guard gemstone
more complex grip
faceted pommel
pommel gemstone
~~~

The goal was not simply to increase polygon count.

The goal was to test whether the semantic topology approach could survive a
weapon containing multiple overlapping visual structures.

---

# 42. Concept Sheet

The evolved concept was generated as a controlled weapon concept sheet.

Input:

~~~text
dataset/ml/awakened_moonstone_scimitar_concept_sheet.png
~~~

The sheet contains:

~~~text
3/4 view
SIDE VIEW
FRONT VIEW
crystal core detail
inner hook detail
guard detail
pommel detail
color palette
silhouette reference
~~~

The center SIDE VIEW remains the primary source for procedural silhouette
analysis.

The FRONT VIEW is used as a reference for approximate thickness.

The 3/4 view is primarily useful for understanding the intended form.

---

# 43. New Generator

The Awakened weapon received its own generator instead of modifying Moonstone
V3.

Generator:

~~~text
ml/concept_to_awakened_moonstone_scimitar.py
~~~

Build helper:

~~~text
tools/build_awakened_moonstone.sh
~~~

Generated output directory:

~~~text
dataset/ml/generated_awakened_moonstone_scimitar/
~~~

Important generated files:

~~~text
awakened_moonstone_scimitar.obj
awakened_moonstone_scimitar.mtl
awakened_moonstone_scimitar.meta.json

side_crop.png
side_mask.png
front_crop.png
front_mask.png
~~~

Keeping Tier 1 and Tier 2 generators separate is intentional.

A successful evolution tier should become a frozen asset-generation stage
rather than continually mutating the previous tier.

---

# 44. Geometry Complexity

The generated Awakened weapon contains:

~~~text
158 vertices
264 triangles
~~~

Shading:

~~~text
216 flat-shaded faces
48 smooth-shaded faces
~~~

This is significantly more complicated than Moonstone V3:

~~~text
Moonstone V3
64 vertices
100 triangles
~~~

The increase in geometry comes primarily from additional semantic structures,
not uniform subdivision.

That distinction is important.

---

# 45. Semantic Parts

The generator records the following vertex ranges:

~~~text
main_blade          0 .. 41     42 vertices
inner_hook         42 .. 59     18 vertices

crystal_core       60 .. 65      6 vertices
upper_shard        66 .. 71      6 vertices
lower_shard        72 .. 77      6 vertices

guard              78 .. 107    30 vertices
guard_gem         108 .. 113     6 vertices

grip              114 .. 145    32 vertices

pommel            146 .. 151     6 vertices
pommel_gem        152 .. 157     6 vertices
~~~

This is the first weapon where semantic separation became essential rather
than merely convenient.

---

# 46. Metadata Output

The generator also produces:

~~~text
awakened_moonstone_scimitar.meta.json
~~~

The metadata describes information such as:

~~~text
weapon name
weapon family
evolution tier
archetype
worn reference
semantic parts
grip anchor
blade tip
crystal core location
~~~

Conceptually:

~~~json
{
    "name": "Awakened Moonstone Scimitar",
    "family": "moonstone_scimitar",
    "evolution_tier": 2,
    "archetype": "scimitar",
    "worn_reference_model": 490,

    "parts": {
        "main_blade": {},
        "inner_hook": {},
        "crystal_core": {},
        "upper_shard": {},
        "lower_shard": {},
        "guard": {},
        "guard_gem": {},
        "grip": {},
        "pommel": {},
        "pommel_gem": {}
    },

    "anchors": {
        "grip": [],
        "blade_tip": [],
        "crystal_core": []
    }
}
~~~

This is the beginning of moving weapon-specific information out of
`Client.cpp` and into the generated asset description.

---

# 47. Image Mask Strategy

The Awakened generator uses the same general mask approach established during
the Moonstone experiments.

Concept background color is estimated from crop borders.

Pixels sufficiently different from the background are classified as weapon
pixels.

The generator then inspects horizontal runs through the mask.

A run represents a contiguous visible structure at a particular vertical
location.

For a simple sword this is straightforward.

For the Awakened weapon it became substantially more difficult because a
single row can contain:

~~~text
inner hook

floating crystal

main blade
~~~

at the same time.

This exposed an important weakness in naive silhouette extraction.

---

# 48. Important Failure - Width Is Not Semantics

The original Awakened `inner_hook()` implementation selected a secondary
silhouette using approximately:

~~~text
choose the widest non-main structure
~~~

This initially seemed reasonable.

It was wrong.

At several positions the suspended crystal was wider than the thin inner
hook.

The parser therefore interpreted:

~~~text
floating crystal
~~~

as:

~~~text
inner blade structure
~~~

The generator was not merely placing one component incorrectly.

It was reconstructing the wrong topology.

---

# 49. Correct Inner-Hook Rule

The concept sheet is deliberately controlled.

In the SIDE VIEW, the structures have a predictable spatial relationship:

~~~text
LEFT
    inner crescent / hook

MIDDLE
    suspended crystal and shards

RIGHT
    main crescent blade
~~~

The corrected rule therefore chooses:

~~~text
left-most substantial secondary structure
~~~

instead of:

~~~text
widest secondary structure
~~~

This is a semantic rule derived from the controlled concept-sheet layout.

It is much more reliable for this weapon.

---

# 50. Main Blade Extraction

The primary crescent remains the right-most major silhouette in the SIDE
VIEW.

Meaningful positions are sampled along the blade.

The blade continues to use purposeful low-poly rails rather than full ring
extrusion.

Conceptually:

~~~text
spine
    \
     front ridge
        \
         cutting edge
        /
     back ridge
~~~

Each section contributes only the geometry required to describe the blade
surface.

---

# 51. Blade Root Failure

The initial Awakened generator sampled the main blade too far downward.

The final sample occurred approximately at:

~~~text
t = 0.690
~~~

At that location the concept image is already entering the complicated
blade/guard intersection.

Image-derived geometry therefore contaminated the weapon's structural root.

The visible symptom was:

~~~text
blade does not appear aligned with
guard -> grip -> pommel
~~~

The individual pieces looked good, but they did not read as one continuous
weapon.

---

# 52. Semantic Blade Root

The final solution separates two responsibilities.

The concept image determines:

~~~text
interesting blade silhouette
blade curvature
blade width
blade facets
~~~

But the weapon grammar determines:

~~~text
where the blade attaches to the hilt
~~~

Main image-derived blade sampling now ends before the guard contamination
region.

An explicit semantic blade root is then created on the common weapon axis:

~~~text
x = 0
~~~

Conceptually:

~~~text
         CURVED BLADE
              \
               \
                \
                 |
               x = 0
                 |
               GUARD
                 |
                GRIP
                 |
               POMMEL
~~~

This preserves the custom crescent shape while guaranteeing structural
continuity.

---

# 53. Shared Weapon Axis

The hilt-side structures use a common local axis:

~~~text
x = 0
~~~

This includes:

~~~text
blade root
guard center
grip center
pommel center
~~~

The important distinction is that the entire blade is NOT forced straight.

Only its structural attachment point is constrained.

The crescent is still free to curve away from the centerline.

This creates a useful procedural modeling rule:

> semantic joints should be constrained even when decorative geometry is
> concept-driven.

---

# 54. Inner Hook Sampling

The final inner-hook sample range was also narrowed.

The earlier version sampled portions where the hook had not clearly begun or
had already merged into unrelated structures.

The refined samples focus on the actual crescent region.

Approximately:

~~~text
t = 0.395
t = 0.450
t = 0.485
t = 0.515
t = 0.555
~~~

This greatly reduces contamination from:

~~~text
floating crystals
guard structures
main blade junction
~~~

---

# 55. Semantic Hook Attachment

The inner hook should not simply end with an arbitrary cap.

The concept depicts it as a secondary crescent that visually reconnects into
the primary blade structure.

Therefore the final hook is attached back into the main blade around a
semantic connection region.

Approximately:

~~~text
t = 0.610
~~~

The connection is derived from the inner/spine side of the main blade.

This gives the structure the intended visual grammar:

~~~text
MAIN BLADE
      \
       \
        \
         \
          \
     ____/
    /
   /
INNER HOOK
~~~

rather than two unrelated floating blade objects.

---

# 56. Floating Crystal Components

The concept contains disconnected geometry:

~~~text
upper shard
main crystal core
lower shard
~~~

This required a new concept-processing technique.

Instead of assuming all interesting geometry belongs to the primary weapon
silhouette, the generator performs connected-component analysis on the SIDE
VIEW mask.

Disconnected image regions can therefore become disconnected 3D components.

This is important for increasingly fantastical RSPS weapon designs.

---

# 57. Connected Components

The SIDE VIEW mask is analyzed using 8-connected pixel regions.

The largest component generally represents the primary connected weapon
structure.

Smaller disconnected components inside the expected blade region are
considered candidates for:

~~~text
crystal core
floating shards
ornaments
~~~

The largest meaningful floating component becomes the primary crystal core.

Components above and below it can become secondary shards.

This means floating structures can be derived from the actual concept rather
than invented entirely from hardcoded coordinates.

---

# 58. Failed Symptom Fix - Moving The Core

During debugging the floating core appeared too low.

A temporary adjustment moved the entire cluster upward by modifying its
longitudinal `t` values.

Conceptually:

~~~text
FLOATING_CLUSTER_SHIFT_T = negative value
~~~

This moved the effect toward the blade tip.

Although useful diagnostically, this did not solve the real discrepancy.

The deeper problem was that the inner-hook parser had confused different
semantic structures.

Once the hook extraction was corrected, the manual floating-cluster shift
was removed.

Important lesson:

> if moving a decorative component repeatedly does not make the model match
> the concept, verify that the surrounding topology itself was classified
> correctly.

---

# 59. Avoid Fighting The Concept

The final Awakened process uses image-derived positions for disconnected
crystal structures whenever they can be reliably detected.

Manual offsets should be treated as:

~~~text
fine adjustment
~~~

not:

~~~text
primary reconstruction strategy
~~~

The concept image should remain the source of truth for visual placement when
its structure is unambiguous.

---

# 60. Worn Placement

The complicated new blade did NOT require changing the successful worn-model
placement strategy.

The existing sequence still works:

~~~text
load custom OBJ
        |
        v
fitToWornReference(custom, native490)
        |
        v
mirror fitted Y profile
        |
        v
calculate semantic grip centroid
        |
        v
translate grip to native hand position
        |
        v
equip player
~~~

This is an important result.

Visual complexity can increase substantially without requiring a different
attachment system.

---

# 61. Awakened Grip

The generated Awakened grip occupies:

~~~text
vertices 114 .. 145
~~~

The generator exports this range through semantic metadata.

The client therefore does not need to guess where the grip is by inspecting
the full weapon bounds.

This preserves the successful semantic anchoring approach established by
Moonstone V3.

---

# 62. Native Worn Anchor

The Awakened weapon continues to use native scimitar model:

~~~text
490
~~~

as its worn archetype.

The established native hand target remains:

~~~text
X = -29.0
Y = -98.25
Z =  2.50
~~~

During successful testing the generated grip was translated onto this native
location.

The weapon therefore retains the same player-hand relationship as its Tier 1
form.

---

# 63. Evolution System Implication

The Moonstone family now demonstrates an important property.

A weapon can evolve visually while keeping the same logical attachment
contract.

Conceptually:

~~~text
Tier 1 Moonstone Scimitar
          |
          v
Tier 2 Awakened Moonstone Scimitar
          |
          v
future Tier 3
~~~

The following can change dramatically:

~~~text
blade topology
blade size
negative space
floating components
guard complexity
materials
effects
ornamentation
~~~

while the following remain stable:

~~~text
archetype
grip semantics
hand attachment
animation compatibility
worn reference
~~~

This is highly useful for an upgrade or evolution system.

---

# 64. Complexity Does Not Require Ring Topology

The Awakened weapon contains substantially more geometry than V3.

However the solution was not:

~~~text
increase sampling density everywhere
~~~

Instead complexity comes from adding meaningful semantic structures:

~~~text
another blade
another opening
floating components
larger guard
additional gems
more grip topology
~~~

Each structure receives its own small topology.

This continues the central rule discovered during Moonstone V3:

> complexity should come from additional forms, not arbitrary subdivision.

---

# 65. Image-Derived vs Semantic Geometry

The Awakened experiment established a useful distinction.

## Image-derived information

Use the concept for:

~~~text
silhouette
curve
width
relative placement
negative space
floating ornament positions
overall proportions
~~~

## Semantic weapon grammar

Use procedural rules for:

~~~text
blade-to-hilt attachment
grip construction
guard construction
pommel construction
cross sections
facet structure
shading mode
worn anchor
animation metadata
~~~

Neither side should completely replace the other.

The successful pipeline combines both.

---

# 66. Controlled Concept Sheets Matter

The current image-processing rules depend on a predictable concept layout.

For the Awakened SIDE VIEW:

~~~text
left-most major secondary structure
    = inner hook

middle disconnected structure
    = core / shards

right-most major structure
    = primary blade
~~~

This would not necessarily be safe for arbitrary artwork.

Therefore future concept-generation prompts should continue requesting:

~~~text
clean orthographic-ish side view
plain background
clear negative space
consistent views
minimal overlap
no character
no environment
readable low-poly forms
~~~

Better source concepts reduce procedural ambiguity dramatically.

---

# 67. Semantic Parsing Is Becoming The Core Problem

Moonstone V3 primarily tested:

~~~text
good topology generation
~~~

Awakened Moonstone additionally tested:

~~~text
semantic interpretation of a complicated silhouette
~~~

As weapons become more complicated, the difficult question increasingly
becomes:

> what does each visible region represent?

Examples:

~~~text
main blade?
secondary blade?
gem?
floating shard?
guard?
decorative hole?
material transition?
~~~

The generator cannot rely only on raw pixel geometry.

It increasingly needs a weapon grammar.

---

# 68. Current Weapon Grammar

The current procedural grammar now understands concepts including:

~~~text
main curved blade
secondary curved blade / hook
raised blade ridge
cutting edge
negative space
floating gem
floating shard
winged guard
guard gem
low-poly grip
faceted pommel
pommel gem
~~~

This is already considerably more expressive than the original ring-based
generator.

---

# 69. Awakened Result

The final Awakened Moonstone Scimitar is considered successful.

It preserves:

~~~text
Moonstone identity
scimitar identity
one-handed proportions
RuneScape-style readability
low-poly visual language
correct player attachment
~~~

while adding:

~~~text
large evolved crescent
inner hook
large negative space
suspended crystal
floating fragments
more elaborate guard
additional gemstones
greater topology complexity
~~~

The result looks visibly evolved rather than simply recolored or enlarged.

---

# 70. Proof Of Evolution

The project has now produced two successful concept-driven stages:

~~~text
TIER 1
Moonstone Scimitar V3

64 vertices
100 triangles


TIER 2
Awakened Moonstone Scimitar

158 vertices
264 triangles
~~~

Both use original generated geometry.

Both use the same general RuneScape worn archetype.

Both successfully render in-game.

This provides initial evidence that the pipeline can support an actual visual
weapon-evolution system.

---

# 71. Important Lessons From Tier 2

The most important lessons are:

## 71.1 Never confuse geometric size with semantic identity

The widest secondary shape is not necessarily the secondary blade.

## 71.2 Semantic joints should be constrained

Blade-to-hilt attachment should not depend entirely on noisy image sampling.

## 71.3 Negative space is first-class geometry

The empty region between the outer crescent and inner hook contributes
strongly to the weapon's identity.

## 71.4 Disconnected geometry is valid

RuneScape-style fantasy weapons can contain separate floating components.

## 71.5 Preserve component identity throughout the pipeline

Do not generate a core, hook, grip, or guard and then forget what it was.

## 71.6 Fix causes before offsets

Repeated positional nudges can hide a classification mistake without fixing
it.

## 71.7 Complexity should remain purposeful

158 useful vertices are preferable to hundreds of mechanically generated
cross-section vertices.

---

# 72. Pipeline After Tier 2

The current successful conceptual pipeline is now:

~~~text
AI concept sheet
        |
        v
controlled view crops
        |
        v
background segmentation
        |
        v
connected-component analysis
        |
        v
semantic silhouette classification
        |
        +-----------------------+
        |                       |
        v                       v
image-derived forms      semantic constraints
        |                       |
        +-----------+-----------+
                    |
                    v
purposeful topology construction
                    |
                    v
component-specific shading
                    |
                    v
OBJ + MTL
                    |
                    v
semantic metadata JSON
                    |
                    v
native worn-archetype fitting
                    |
                    v
profile correction
                    |
                    v
semantic grip anchoring
                    |
                    v
Eldoria ModelData
                    |
                    v
in-game weapon
~~~

---

# 73. Next Evolution Candidate

A logical Tier 3 would continue evolving the same weapon family.

Possible name:

~~~text
Eclipse Moonstone Scimitar
~~~

Potential new structures:

~~~text
split primary crescent
two internal blade structures
larger eclipse core
multiple floating fragments
asymmetric guard
additional negative spaces
stronger material hierarchy
effect attachment anchors
~~~

The next experiment should increase semantic complexity while preserving the
same attachment contract.

---

# 74. Tier 3 Target

A reasonable next target would be roughly:

~~~text
200 - 300 vertices
300 - 500 triangles
~~~

but polygon count should remain a consequence of the design.

It should not become a target by itself.

The important test is whether a substantially more complex weapon can still
maintain:

~~~text
clean silhouette
large purposeful facets
readable shading
clear semantic parts
stable worn placement
RuneScape visual language
~~~

---

# 75. Long-Term Evolution Architecture

The Moonstone family suggests a possible future structure:

~~~text
moonstone_scimitar/
    tier_1/
        weapon.obj
        weapon.mtl
        weapon.meta.json

    tier_2_awakened/
        weapon.obj
        weapon.mtl
        weapon.meta.json

    tier_3_eclipse/
        weapon.obj
        weapon.mtl
        weapon.meta.json
~~~

Metadata could eventually contain:

~~~json
{
    "family": "moonstone_scimitar",
    "tier": 3,
    "previous_tier": 2,
    "archetype": "scimitar",

    "anchors": {
        "grip": [],
        "blade_tip": [],
        "core": [],
        "effect_origin": [],
        "trail_start": [],
        "trail_end": []
    }
}
~~~

This would allow visual evolution, effects, and gameplay systems to share the
same semantic asset definition.

---

# 76. Current Conclusion

The Awakened Moonstone experiment demonstrates that the procedural modeling
strategy scales beyond a simple single-blade weapon.

The pipeline successfully handled:

~~~text
multiple blade structures
negative space
disconnected geometry
floating crystals
increased topology
semantic components
component metadata
stable worn attachment
~~~

The largest challenge was no longer polygon generation itself.

It was semantic interpretation.

That suggests the next stage of the project should focus increasingly on:

> turning visual concepts into structured weapon parts before generating
> geometry.

The core principle remains:

> Do not directly convert pixels into polygons.

Instead:

> understand the weapon represented by the pixels, then construct that weapon
> using a RuneScape-oriented low-poly grammar.

The Awakened Moonstone Scimitar is the second successful proof of that
approach.
DOCBLOCK


echo
echo "========================================"
echo " DOCUMENTATION UPDATED"
echo "========================================"
echo
echo "File:"
echo "  $DOC"
echo
echo "Backup:"
echo "  $BACKUP"
echo
echo "Added section:"
echo "  Awakened Moonstone Scimitar - Tier 2"
echo
echo "Total document lines:"
wc -l "$DOC"
echo
echo "Last 25 lines:"
tail -25 "$DOC"
