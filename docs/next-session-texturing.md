# RuneForge – Next Session Notes

Current state:
- Texture archive decoding fully works
- All archive 6 textures decode correctly
- SDL texture preview works inside CacheExplorerMode
- Correct SDL format is ABGR8888
- Texture file first 2 bytes = offset into index.dat
- index.dat contains:
  - canvas width/height
  - palette
  - texture metadata
- Texture metadata decoding confirmed working

Model findings:
- textureTriangleCount decoded correctly
- textureDataOffset contains texture triangles
- Each texture triangle = 6 bytes:
  u16 a
  u16 b
  u16 c

Current texture triangle decode works:
- 50 decoded texture triangles for model 147
- Vertex indices are valid

Important:
- face.textureFlag is NOT boolean
- Values like 42 likely reference texture mapping data
- Likely:
    textureTriangleIndex = textureFlag / 2

Rendering status:
- Renderer still only supports:
  - wireframe
  - flat fill
  - alpha
- No textured rasterization yet

Next goals:
1. Confirm textureFlag -> textureTriangleIndex mapping
2. Determine actual texture image ID source
3. Pass textureTriangles into renderer
4. Create textured triangle rasterizer
5. Add UV interpolation / texture sampling
6. Later:
   - perspective correction
   - mipmaps/filtering
   - GPU renderer path

Architecture notes:
- Current renderer path:
    ModelViewerMode
      -> drawWireframeModel()

- Texturing should probably evolve into:
    Mesh
      -> Material/Texture refs
      -> TriangleRasterizer textured path

- Avoid hardcoding RS-specific logic into renderer core
