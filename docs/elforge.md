# ElForge Roadmap

## Purpose

This document defines the development roadmap for ElForge.

ElForge is the internal development, inspection, debugging, editing, and content creation tool for Eldoria.

It starts as a viewer and inspector.

Long-term, it becomes the tool used to create, edit, validate, save, export, and maintain Eldoria content.

This document answers:

> How does ElForge grow from a cache viewer into a complete content creation tool?

Small implementation tasks belong in GitHub Issues.

This document defines:

- vision
- ownership boundaries
- planned app structure
- milestones
- feature areas
- issue candidates
- product completion criteria

---

## Vision

ElForge is the forge of Eldoria.

Its job is to make the game understandable and editable.

The first goal is visibility.

The long-term goal is creation.

ElForge should eventually support:

- cache browsing
- asset inspection
- model viewing
- texture viewing
- map viewing
- animation viewing
- interface viewing
- item definition inspection
- NPC definition inspection
- object definition inspection
- collision visualization
- world inspection
- map editing
- object placement editing
- definition editing
- custom asset importing
- saving/exporting modified content
- content validation
- project workspace management
- debugging tools for ElClient and ElServer

ElForge is primarily internal.

If it becomes mature enough, supports multiple cache formats, and is useful beyond Eldoria, it may later become a public or paid tool.

---

## Product Goal

ElForge is not finished when it can view a model.

ElForge is finished enough when it can support the full Eldoria content workflow:

1. Open an Eldoria project.
2. Read the source cache.
3. Inspect existing assets.
4. Inspect existing world/content data.
5. Edit maps, objects, and definitions where practical.
6. Import custom assets.
7. Validate modified content.
8. Export content in a format used by ElClient and ElServer.
9. Help debug content problems quickly.

The final goal is not just a viewer.

The final goal is a usable development tool for building a custom RuneScape-like world.

---

## Design Priorities

### Inspection First

Before ElForge edits anything, it should inspect and display things correctly.

The order is:

1. Read
2. Decode
3. Display
4. Validate
5. Define save/export strategy
6. Edit
7. Save/export modified content

Editing should come after understanding.

### Validation Tool

ElForge should validate shared systems.

Examples:

- `src/data/model/` is validated by the Model Viewer.
- `src/data/texture/` is validated by the Texture Viewer.
- `src/data/map/` and `src/world/` are validated by the Map Viewer.
- `src/data/animation/` is validated by the Animation Viewer.
- `src/data/interface/` is validated by the Interface Viewer.
- `src/world/collision/` is validated by collision debug tools.

If a system cannot be inspected, it is harder to trust.

### Disposable Probes Are Allowed

Unknown systems may begin as probe apps or temporary panels.

Probe code may be messy.

Production ElForge code should be clean.

Once a system is understood, it should move into the correct shared module or clean ElForge workspace.

### Tool UI Can Be Practical

ElForge UI does not need to look like the game client.

ElForge can use tool-style UI:

- panels
- dockspaces
- inspectors
- tables
- debug views
- asset browsers
- property grids
- preview viewports

### Editing Requires Persistence

Before serious editing exists, ElForge needs a save/export strategy.

Editing without a persistence model creates messy tools.

The tool should understand:

- source cache
- Eldoria project folder
- modified content
- export target
- dirty state
- validation before save/export

### ElForge Supports ElClient and ElServer

ElForge should not exist in isolation.

Every mature ElForge editing/export feature should eventually prove that its output can be consumed by:

- ElClient
- ElServer

Example:

- A custom object definition edited in ElForge should be loadable by ElServer.
- A custom model imported in ElForge should be displayable by ElClient.
- A modified map/object placement exported by ElForge should be usable by both.

---

## What ElForge Owns

ElForge owns tool-specific behavior.

Examples:

- workspace layout
- dockspace/panel layout
- cache browsing UI
- asset selection UI
- inspector panels
- preview viewports
- debug panels
- ImGui theme/look-and-feel
- editing tools
- import/export workflows
- save/export actions
- project workspace state
- tool-specific settings
- tool-specific validation reports

---

## What ElForge Does Not Own

ElForge should not own shared systems.

Examples:

- cache reading belongs in `src/data/`
- model decoding belongs in `src/data/`
- texture decoding belongs in `src/data/`
- map decoding belongs in `src/data/`
- world concepts belong in `src/world/`
- rendering backend belongs in `src/render/`
- platform/window/input behavior belongs in `src/platform/`
- ImGui backend/vendor build plumbing belongs in `src/platform/imgui/`
- server authority belongs in ElServer
- client runtime presentation belongs in ElClient

ElForge combines shared systems into an inspection, validation, and editing experience.

### Renderer Path

ElForge currently renders model viewports through `src/render_next/`.

`render_next` is the intended future renderer architecture because it already has the newer scene, pipeline, backend, framebuffer, and software-rendering shape.

ElForge still depends on `src/render/` for legacy/reference renderer code and still-used primitives such as camera, projection, math, color, model transform, and render option types.

The final renderer ownership should eventually converge back to `src/render/` after `render_next` fully replaces the old direct model renderer path.

Until that migration is complete:

- keep `src/render/` available
- keep `render_next` separate
- do not delete old renderer code before useful behavior is preserved
- do not move camera/math/color primitives until their final ownership is settled

---

## Planned App Structure

```text
apps/elforge/
├── main/
├── app/
├── workspace/
├── panels/
├── browser/
├── inspector/
├── viewport/
├── project/
├── editor/
├── import_export/
├── validation/
├── settings/
└── debug/
```

### `main/`

Application entry point.

### `app/`

Owns the tool lifetime.

Examples:

- ElForgeApp
- startup
- update loop
- render loop
- shutdown

### `workspace/`

Owns major tool modes and workspace state.

Examples:

- CacheWorkspace
- ModelWorkspace
- MapWorkspace
- InterfaceWorkspace
- ProjectWorkspace

### `panels/`

Reusable tool panels.

Examples:

- CacheTreePanel
- AssetInspectorPanel
- ViewportPanel
- DebugPanel
- PropertiesPanel

### `browser/`

Asset and cache browsing UI.

Examples:

- cache tree
- archive list
- asset search
- selection model
- project asset browser

### `inspector/`

Detailed asset inspection.

Examples:

- model inspector
- texture inspector
- map inspector
- definition inspector
- interface inspector
- animation inspector

### `viewport/`

Tool viewport logic.

Examples:

- model viewport
- texture viewport
- map viewport
- animation viewport
- collision viewport

### `project/`

Eldoria project workspace logic.

Examples:

- source cache
- output folder
- custom content folder
- export target
- project settings

### `editor/`

Editing tools.

Examples:

- map editor
- object placement editor
- definition editor
- interface editor later

### `import_export/`

Importing, exporting, saving, and conversion workflows.

Examples:

- export model
- import custom model
- save modified map
- export content pack
- export debug dump

### `validation/`

Content validation and diagnostics.

Examples:

- missing asset checks
- invalid reference checks
- export validation
- content report generation

### `settings/`

Tool configuration.

Examples:

- cache path
- theme
- viewport settings
- debug settings
- recent projects

### `debug/`

Tool debugging utilities.

Examples:

- asset dump
- render stats
- cache diagnostics
- decode diagnostics

---

# Milestones

---

## Milestone 1 — ElForge Shell

### Goal

Create the first runnable ElForge application shell.

### Success Criteria

- ElForge launches.
- Window opens.
- Basic docking or panel layout exists.
- Main menu exists.
- Tool state can be updated and rendered.
- Clean shutdown path exists.

### Feature Areas

- application lifecycle
- platform/window integration
- tool UI setup
- workspace shell
- panel layout

### Issue Candidates

- Create `apps/elforge` target.
- Create ElForge application entry point.
- Create ElForgeApp lifetime class.
- Initialize platform window.
- Initialize tool UI.
- Add main menu bar.
- Add empty dockspace or panel layout.
- Add tool update loop.
- Add tool render loop.
- Add clean shutdown path.

### Proves

ElForge exists as a standalone development tool.

---

## Milestone 2 — Cache Browser

### Goal

Open a RuneScape cache and browse its contents.

### Success Criteria

- Cache path can be selected or configured.
- Cache opens successfully.
- Cache indexes can be listed.
- Files/archives can be browsed.
- Basic raw file information can be displayed.
- Selection state is clean.

### Feature Areas

- cache path configuration
- cache loading
- cache tree
- selection model
- raw file inspector
- error handling

### Issue Candidates

- Add cache path setting.
- Add open cache action.
- Display cache open success/failure.
- List cache indexes.
- Display files inside selected index.
- Display archive/file ids.
- Add cache tree panel.
- Add selected cache entry state.
- Add raw payload size display.
- Add raw hex/byte preview.
- Add cache error messages.

### Proves

ElForge can inspect the raw source of Eldoria data.

---

## Milestone 3 — Model Viewer

### Goal

Load, inspect, and render models.

### Success Criteria

- Known models can be loaded.
- Model data can be inspected.
- Vertices and faces can be displayed numerically.
- Model can be rendered in a viewport.
- Camera controls exist.
- Basic debug overlays exist.

### Feature Areas

- model loading
- model inspection
- render mesh conversion
- model viewport
- camera controls
- render debug

### Issue Candidates

- Add model selection from cache browser.
- Load model by id.
- Display model vertex count.
- Display model face count.
- Display model texture/priority/alpha info.
- Convert ModelAsset to render mesh.
- Render model in viewport.
- Add model viewport camera.
- Add wireframe toggle.
- Add filled rendering toggle.
- Add vertex debug toggle.
- Add model transform controls.
- Add model loading error display.

### Proves

The data-to-render pipeline works for the first major asset type.

### Current Viewport Controls Audit

The migrated ElForge model viewport currently uses keyboard-driven controls in
`CacheViewportPanel` and stores tool interaction state in `CacheState`.
Selecting a model cache file loads that model by file id through
`ElForgeApplication::handleSelectionChanged`. There are no next/previous model
id navigation controls yet.

Current verified controls:

- Left/right arrows rotate the model around Y.
- Up/down arrows rotate the model around X.
- Q/E rotate the model around Z.
- Equals/minus scale the model, clamped to a minimum scale of 0.1.
- W/S move the model vertically in viewport space.
- A/D move the model horizontally in viewport space.
- R resets model offset, rotation, and scale.
- T toggles the textured-face debug flag passed to the software render backend.

The model transform controls belong to ElForge UI state because they describe
how the tool presents the selected asset. The viewport submits that state to
`render_next` as a `RenderObject` transform. Shared renderer state owns camera
projection, mesh projection, face assembly, visibility, material sampling,
alpha application, framebuffer output, and backend drawing.

The current viewport initializes camera distance/FOV values, but it does not
provide runtime zoom, distance, or FOV controls. The current zoom-like behavior
is model scale, not camera distance.

The legacy `RenderOptions` state still includes fill, wireframe, vertex, and
alpha flags, but the current `render_next` viewport path does not expose or
consume fill, wireframe, or vertex debug toggles. Alpha is consumed by the
software backend for solid packets, and textured packets use texture sampling.
Priority data is assembled into render packets, but priority-specific ordering
is not currently applied by the `render_next` pipeline.

Follow-up candidates:

- Add visible viewport controls or a compact control reference.
- Add explicit camera zoom/distance controls.
- Implement fill, wireframe, and vertex debug behavior for the `render_next`
  viewport path.
- Verify and complete the textured-face debug highlight behavior.
- Audit priority-aware face ordering in `render_next`.
- Add model id navigation if it fits the cache/model inspection workflow.

---

## Milestone 4 — Texture and Material Viewer

### Goal

Load and inspect textures and materials.

### Success Criteria

- Textures can be loaded.
- Texture palettes can be inspected.
- Texture pixels can be displayed.
- Texture preview exists.
- Model viewer can use material/texture data where available.

### Feature Areas

- texture loading
- texture inspection
- palette inspection
- texture preview
- material debugging
- model/material integration

### Issue Candidates

- Add texture selection from cache browser.
- Load texture by id.
- Display texture dimensions.
- Display texture palette information.
- Render texture preview.
- Add zoom controls for texture preview.
- Add pixel hover/debug info.
- Add material information panel.
- Connect texture data to model viewer.
- Add missing texture fallback display.
- Add texture decode error display.

### Proves

ElForge can inspect 2D visual assets and validate material data.

---

## Milestone 5 — Map Region Viewer

### Goal

Load and inspect map regions.

### Success Criteria

- Map region data can be loaded.
- Terrain can be decoded.
- Object spawns can be decoded.
- Region can be inspected numerically.
- Region can be viewed visually.
- Collision or tile flags can be visualized.

### Feature Areas

- map loading
- terrain inspection
- object spawn inspection
- region viewport
- collision visualization
- coordinate debugging

### Issue Candidates

- Add region id input.
- Load map region by id.
- Decode terrain data.
- Decode object spawn data.
- Display region coordinate information.
- Display tile height information.
- Display overlay/underlay information.
- Display placed objects list.
- Render terrain preview.
- Render placed object markers.
- Render placed object models where available.
- Add tile hover inspector.
- Add collision flag overlay.
- Add region navigation controls.

### Proves

ElForge can inspect world-space data, not only isolated assets.

---

## Milestone 6 — Definition Inspector

### Goal

Inspect static definitions.

### Success Criteria

- Item definitions can be loaded and searched.
- NPC definitions can be loaded and searched.
- Object definitions can be loaded and searched.
- Definitions can be linked to models where possible.
- Definitions can be inspected clearly.

### Feature Areas

- item definitions
- NPC definitions
- object definitions
- definition search
- definition/model links
- property tables

### Issue Candidates

- Load item definitions.
- Add item definition list.
- Add item definition search.
- Display item properties.
- Display item model references.
- Load NPC definitions.
- Add NPC definition list.
- Add NPC definition search.
- Display NPC properties.
- Display NPC model references.
- Load object definitions.
- Add object definition list.
- Add object definition search.
- Display object properties.
- Display object model references.
- Add jump-to-model action from definition inspector.

### Proves

ElForge can inspect the static data that drives game content.

---

## Milestone 7 — Animation Viewer

### Goal

Explore, inspect, and preview animation data.

### Success Criteria

- Animation format is explored.
- Animation data can be decoded.
- Animation sequences can be inspected.
- Animation can be applied to a model.
- Basic playback controls exist.

### Feature Areas

- animation probing
- animation loading
- sequence inspection
- transform data inspection
- animation playback
- model animation preview

### Issue Candidates

- Create animation probe if needed.
- Load animation/sequence data.
- Display animation ids.
- Display sequence properties.
- Display frame information.
- Display transform information.
- Select model for animation preview.
- Apply animation to model.
- Add play/pause controls.
- Add frame step controls.
- Add playback speed control.
- Add animation debug overlay.
- Validate animation output visually.

### Proves

ElForge can validate animation systems before ElClient depends on them.

---

## Milestone 8 — Interface and Sprite Viewer

### Goal

Inspect RuneScape-style interface and sprite data.

### Success Criteria

- Sprites can be loaded and previewed.
- Interface definitions can be decoded.
- Interface/widget hierarchy can be inspected.
- Basic interface preview exists.
- Widget properties can be displayed.

### Feature Areas

- sprite loading
- sprite preview
- interface decoding
- widget tree
- interface preview
- widget inspector

### Issue Candidates

- Load sprite data.
- Display sprite list.
- Render selected sprite preview.
- Add sprite zoom controls.
- Decode interface definitions.
- Display interface id list.
- Display widget tree.
- Display widget properties.
- Render basic widget preview.
- Render sprite widget preview.
- Render text widget placeholder.
- Add widget selection.
- Add interface debug dump.

### Proves

ElForge can inspect 2D UI data used later by ElClient.

---

## Milestone 9 — Collision and World Debug Tools

### Goal

Inspect world mechanics visually.

### Success Criteria

- Collision flags can be displayed.
- Tile flags can be inspected.
- Object blocking can be inspected.
- Pathfinding can be tested.
- World coordinate debugging exists.

### Feature Areas

- collision visualization
- tile debug
- object blocking
- pathfinding debug
- coordinate overlays

### Issue Candidates

- Display tile flags overlay.
- Display collision flags overlay.
- Display blocked movement directions.
- Display object blocking data.
- Add tile coordinate overlay.
- Add world coordinate overlay.
- Add region/chunk boundary overlay.
- Add pathfinding test start/end selection.
- Render computed path.
- Add pathfinding failure reason output.

### Proves

ElForge can validate the world systems needed by ElClient and ElServer.

---

## Milestone 10 — ElForge Project Workspace

### Goal

Define the concept of an Eldoria project inside ElForge.

ElForge should not only open a cache. It should understand the project being worked on.

### Success Criteria

- Project workspace can be created or opened.
- Source cache path is stored.
- Output/export folder is stored.
- Custom content folder is stored.
- Project settings are stored.
- Recent projects can be tracked.

### Feature Areas

- project file
- source cache reference
- output folder
- custom content folder
- project settings
- recent projects

### Issue Candidates

- Define ElForge project file format.
- Add create project action.
- Add open project action.
- Store source cache path.
- Store output/export path.
- Store custom content path.
- Store tool project settings.
- Add recent projects list.
- Display current project name/path.
- Add project validation errors.

### Proves

ElForge can operate as a real content tool, not only a cache viewer.

---

## Milestone 11 — Save and Export Foundation

### Goal

Define how modified content is saved or exported before building full editors.

### Success Criteria

- Modified content storage strategy is defined.
- Export directory structure is defined.
- Dirty/modified state exists.
- Validation before export exists.
- Export errors can be displayed.
- ElClient/ElServer loading path for exported content is considered.

### Feature Areas

- save strategy
- export strategy
- dirty state
- validation
- export reports
- client/server integration path

### Issue Candidates

- Define exported content directory structure.
- Define modified map export strategy.
- Define modified definition export strategy.
- Define custom asset export strategy.
- Add dirty state tracking.
- Add export validation step.
- Add export error report.
- Add export success report.
- Add export all modified content action.
- Add ElClient load exported content test.
- Add ElServer load exported definition test.
- Add content version metadata.

### Proves

ElForge can persist work before serious editing begins.

---

## Milestone 12 — Editing Foundation

### Goal

Prepare ElForge for editing workflows.

### Success Criteria

- Tool can distinguish view mode from edit mode.
- Selection tools exist.
- Undo/redo strategy is defined.
- Dirty/modified state is used.
- Tool actions are represented cleanly.

### Feature Areas

- edit mode
- selection
- tool actions
- undo/redo
- dirty state
- save/export integration

### Issue Candidates

- Add view/edit mode state.
- Add selectable world objects.
- Add selection outline/highlight.
- Add editing action interface.
- Use dirty state after edit actions.
- Define undo command structure.
- Add simple undo stack.
- Add simple redo stack.
- Add unsaved changes warning.
- Connect edit actions to export foundation.

### Proves

ElForge is ready to become a creator, not only a viewer.

---

## Milestone 13 — Map and Object Editing

### Goal

Edit world content.

### Success Criteria

- Objects can be selected.
- Objects can be moved.
- Objects can be added.
- Objects can be removed.
- Object rotation/type can be changed.
- Modified map/object data can be saved or exported.

### Feature Areas

- object selection
- object placement
- object movement
- object deletion
- object properties
- map save/export

### Issue Candidates

- Select placed object in map viewport.
- Show selected object properties.
- Move selected object to another tile.
- Change selected object rotation.
- Change selected object type.
- Add new object placement tool.
- Remove selected object.
- Add placement preview.
- Add undo for object placement.
- Add undo for object deletion.
- Export modified object placement data.
- Reload modified map data for validation.

### Proves

ElForge can modify world content.

---

## Milestone 14 — Definition Editing

### Goal

Edit static content definitions where practical.

### Success Criteria

- Item definitions can be edited.
- NPC definitions can be edited.
- Object definitions can be edited.
- Changes can be saved or exported.
- Edited definitions can be validated in ElForge and ElClient.

### Feature Areas

- item editor
- NPC editor
- object editor
- validation
- export/save

### Issue Candidates

- Add editable item definition form.
- Edit item name.
- Edit item model reference.
- Edit item options/actions.
- Export modified item definition.
- Add editable NPC definition form.
- Edit NPC name.
- Edit NPC model reference.
- Edit NPC combat/size fields where supported.
- Export modified NPC definition.
- Add editable object definition form.
- Edit object name.
- Edit object model reference.
- Edit object actions.
- Export modified object definition.
- Validate edited definition in viewer.

### Proves

ElForge can support custom content creation beyond map editing.

---

## Milestone 15 — Custom Asset Import

### Goal

Import custom assets for Eldoria.

### Success Criteria

- Custom models can be imported or prepared.
- Custom textures can be imported or prepared.
- Imported assets can be previewed.
- Imported assets can be linked to definitions.
- Import limitations are documented.

### Feature Areas

- model import
- texture import
- asset validation
- definition linking
- export/save

### Issue Candidates

- Define custom model import requirements.
- Add import model action.
- Validate imported model structure.
- Preview imported model.
- Add import texture action.
- Validate imported texture format.
- Preview imported texture.
- Link imported model to item definition.
- Link imported model to NPC definition.
- Link imported model to object definition.
- Add import error messages.
- Document supported import formats.

### Proves

ElForge can support real custom Eldoria content.

---

## Milestone 16 — Content Validation

### Goal

Validate custom content before it is used by ElClient or ElServer.

### Success Criteria

- Missing references can be detected.
- Invalid model references can be detected.
- Invalid object placement can be detected.
- Invalid definition data can be detected.
- Export validation produces a readable report.

### Feature Areas

- reference validation
- asset validation
- map validation
- definition validation
- export reports

### Issue Candidates

- Add missing model reference check.
- Add missing texture reference check.
- Add invalid object id check.
- Add invalid NPC definition check.
- Add invalid item definition check.
- Add invalid map placement check.
- Add validation report panel.
- Add validation before export.
- Add warning severity levels.
- Add error severity levels.
- Add jump-to-invalid-asset action.

### Proves

ElForge can prevent broken content from reaching the game.

---

## Milestone 17 — ElClient and ElServer Integration Checks

### Goal

Prove that ElForge output works in the rest of Eldoria.

### Success Criteria

- ElClient can load exported visual content.
- ElServer can load exported gameplay/content data.
- Content version or metadata can be checked.
- Mismatched content can be detected.

### Feature Areas

- client export testing
- server export testing
- content metadata
- version checks
- integration diagnostics

### Issue Candidates

- Add exported content metadata.
- Add content version file.
- Add ElClient exported content load test.
- Add ElServer exported definition load test.
- Add mismatch detection plan.
- Add export compatibility report.
- Add launch/test ElClient with exported content action.
- Add launch/test ElServer with exported content action if practical.
- Add integration checklist panel.

### Proves

ElForge is part of the full Eldoria ecosystem, not a standalone toy.

---

## Milestone 18 — Multi-Cache Support

### Goal

Support multiple cache revisions or cache styles where practical.

### Success Criteria

- Cache metadata can be detected.
- Tool can open more than one supported cache style.
- Unsupported caches fail gracefully.
- Cache-specific behavior is isolated.
- Supported cache list is documented.

### Feature Areas

- cache detection
- cache profiles
- compatibility handling
- graceful failure
- documentation

### Issue Candidates

- Add cache profile concept.
- Detect cache revision/profile.
- Display cache metadata.
- Add unsupported cache error message.
- Isolate cache-specific assumptions.
- Add profile-specific decoder path if needed.
- Add supported cache list.
- Add cache compatibility diagnostics.
- Add sample cache validation checklist.

### Proves

ElForge can grow beyond one hardcoded cache setup.

---

## Milestone 19 — Tool Polish

### Goal

Make ElForge pleasant and reliable to use.

### Success Criteria

- Tool layout feels good.
- Panels are easy to navigate.
- Errors are readable.
- Large assets do not freeze the tool unnecessarily.
- Debug tools are useful.
- Common workflows are fast.

### Feature Areas

- UI polish
- layout persistence
- performance
- error handling
- search
- usability
- debug quality

### Issue Candidates

- Add persistent layout settings.
- Add recent cache paths.
- Add recent projects.
- Add asset search.
- Add model search.
- Add definition search.
- Improve panel organization.
- Improve viewport controls.
- Improve error messages.
- Add loading/progress indicators.
- Add performance stats.
- Add keyboard shortcuts.
- Add help/about panel.
- Clean up temporary probe code.

### Proves

ElForge is becoming a real development tool, not only a debug experiment.

---

## Milestone 20 — Product Readiness

### Goal

Prepare ElForge for long-term daily use, and possibly public release later.

### Success Criteria

- Tool can be installed or run cleanly.
- Settings persist reliably.
- Projects open reliably.
- Errors are understandable.
- Common workflows are documented.
- Tool is stable enough for regular use.
- Public/commercial release path is understood if desired.

### Feature Areas

- packaging
- documentation
- stability
- onboarding
- release workflow
- licensing/public release consideration

### Issue Candidates

- Add release build configuration.
- Add basic user documentation.
- Add project setup guide.
- Add cache setup guide.
- Add content export guide.
- Add known limitations document.
- Add crash/error log output.
- Add portable settings path.
- Add version/about information.
- Add release checklist.
- Define public release requirements.
- Define paid tool requirements if pursued.

### Proves

ElForge is not only functional, but maintainable as a serious tool.

---

## Long-Term Completion Criteria

ElForge can be considered mature when:

- It opens supported caches reliably.
- It inspects major asset types.
- It validates models, textures, maps, animations, interfaces, and definitions.
- It visualizes world/collision data.
- It understands an Eldoria project workspace.
- It exports modified content cleanly.
- It edits maps and object placement.
- It edits definitions where practical.
- It imports custom assets.
- It validates content before export.
- It exports content usable by ElClient and ElServer.
- It supports the Eldoria custom content pipeline.
- It is stable enough for regular development use.
- It has enough polish that future public release is possible if desired.

---

## GitHub Issues Strategy

GitHub Issues should be generated from milestone issue candidates.

Recommended issue size:

> Small enough that one focused implementation can complete it.

Bad issue:

```text
Implement map editor
```

Better issues:

```text
Select placed object in map viewport
Move selected object to another tile
Add object placement preview
Export modified object placement data
```

Recommended labels:

- `elforge`
- `data`
- `render`
- `world`
- `editor`
- `viewer`
- `debug`
- `import-export`
- `project`
- `validation`
- `integration`
- `bug`
- `cleanup`

---

## Relationship to Other Documents

### `architecture.md`

Defines where tool code belongs.

### `roadmap.md`

Defines the global build order.

### `elclient.md`

Defines the player-facing client ElForge supports.

### `elserver.md`

Defines the authoritative server ElForge supports through content/data workflows.

---

## Status

This document is the current ElForge roadmap baseline.

It is expected to evolve as ElForge becomes real.
