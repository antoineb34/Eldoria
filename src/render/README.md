# render

Purpose: owns how Eldoria data becomes pixels.

The render module is responsible for rendering math, scene representation, camera/projection code, geometry, materials, pipelines, software/GPU backends, text rendering, viewports, and rendering debug helpers.

Dependency rule: `render` may depend on `data` for renderable asset structures where needed, but should not depend on runnable apps or gameplay authority.
