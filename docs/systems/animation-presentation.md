# Animation Presentation

Eldoria keeps gameplay meaning separate from cache sequence IDs.

The server/gameplay layer should eventually communicate semantic events such as
`Attack`, `Death`, `Defend` or `Cast`. Client presentation resolves those events
to model sequences, SpotAnimations and projectiles.

## Sources of truth

Some relationships already live in the 317 cache and remain authoritative:

- NPC idle, walk and turn sequences come from `NpcDefinition`.
- world-object default animation comes from `LocationDefinition::animationId`.
- SpotAnimation model + sequence come from `SpotAnimationDefinition`.
- interface model animation IDs come from interface widgets.

Gameplay relationships that are not in those definitions can be authored in:

```text
content/animation_bindings.csv
```

The initial format is:

```text
kind,id,action,sequence,spotanim,projectile
```

Supported kinds are `npc` and `item`. Supported actions currently include idle,
walk, turns, attack, defend, death, special_attack, cast, use and emote.

This catalog is presentation data. Render remains unaware of gameplay actions or
RS317 sequence semantics.

## Composite action preview

ElForge can preview a semantic NPC action as synchronized visual parts:

```text
AnimationBinding
  |- optional body SequenceDefinition
  `- zero or more SpotAnimation effects
       |- attached graphic
       `- projectile preview
```

The viewport scene supports extra presentation objects so the NPC body stays in
the normal render pipeline while GFX/projectiles are independent models.
Projectile trajectory in ElForge is a presentation/debug trajectory, not combat
authority. The Action composer is deliberately labelled as research/debug so a
manually tested relationship is not confused with cache provenance.

## Interactive action target

ElForge's NPC Action composer can show a local X/Z ground grid and place the
projectile target by clicking the grid in the viewport.

The click is converted into a camera ray and intersected with the transformed
local ground plane. The resulting hit is stored in NPC-local coordinates, so
the target, grid and projectile continue to follow viewport model transforms.

Projectile preview now interpolates from the configured source height to the
chosen 3D target and adds the existing arc height along the preview's local up
axis.
