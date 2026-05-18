1. Clean up engine architecture
   - separate cache format from engine format
   - maybe rename ModelDef → CacheModel

2. Understand full RS317 model binary layout
   - footer
   - block offsets
   - smart deltas
   - triangle opcodes
   - texture triangles

3. Improve renderer pipeline
   - camera abstraction
   - filled triangles
   - backface culling
   - depth buffer

4. Learn rasterization
   - how triangles become pixels
   - barycentric coordinates
   - scanline rendering

5. Add camera movement
   - WASD
   - pitch/yaw
   - view matrix

6. Add colors
   - render triColor
   - understand RS packed HSL

7. Learn projection matrices properly
   - instead of manual projection function

8. Explore animation data
   - vertexSkin
   - triSkin
   - SeqDef / frame transforms

9. Eventually move to GPU/OpenGL
   - VBOs
   - shaders
   - hardware rasterization


- matrices are packaged transforms
- Mat4 * Vertex is the heart of rendering
- projection converts 3D → 2D
- indices define triangle topology
- cache format != engine format
- software rendering pipeline is understandable
