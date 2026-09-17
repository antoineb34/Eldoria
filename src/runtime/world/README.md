# world

`world` owns Eldoria's canonical runtime spatial representation.

It answers:

- what exists
- where it exists
- terrain heights
- tile state
- locations
- coordinate relationships

It does not know:

- cache/archive encoding
- TerrainData or MapLocationData
- textures
- models
- render vertices
- UVs
- materials
- GPU resources
- application UI

Pipeline:

    Assets
      ↓
    runtime/map/RegionBuilder
      ↓
    World
      ↓
    Graphics
      ↓
    Render
