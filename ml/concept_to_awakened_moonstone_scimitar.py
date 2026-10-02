#!/usr/bin/env python3

import json
from pathlib import Path

import numpy as np
from PIL import Image


ROOT = Path(__file__).resolve().parents[1]

INPUT = (
    ROOT
    / "dataset/ml/awakened_moonstone_scimitar_concept_sheet.png"
)

OUTPUT = (
    ROOT
    / "dataset/ml/generated_awakened_moonstone_scimitar"
)

OBJ = OUTPUT / "awakened_moonstone_scimitar.obj"
MTL = OUTPUT / "awakened_moonstone_scimitar.mtl"
META = OUTPUT / "awakened_moonstone_scimitar.meta.json"


# ============================================================
# MATERIALS
# ============================================================

MATERIALS = [
    ("moonstone",      (132, 183, 226)),
    ("moonlight",      (181, 218, 246)),
    ("icy_edge",       (204, 239, 252)),
    ("silver",         (174, 181, 195)),
    ("dark_grip",      (48, 57, 80)),
    ("awakened_core",  (82, 214, 240)),
]

MAT_MOONSTONE = 0
MAT_MOONLIGHT = 1
MAT_EDGE = 2
MAT_SILVER = 3
MAT_GRIP = 4
MAT_CORE = 5


# ============================================================
# IMAGE MASK
# ============================================================

def make_mask(crop):
    h, w, _ = crop.shape

    border = max(
        2,
        min(15, h // 8, w // 8),
    )

    samples = np.concatenate([
        crop[:border].reshape(-1, 3),
        crop[-border:].reshape(-1, 3),
        crop[:, :border].reshape(-1, 3),
        crop[:, -border:].reshape(-1, 3),
    ]).astype(np.float64)

    background = np.median(
        samples,
        axis=0,
    )

    difference = np.linalg.norm(
        crop.astype(np.float64)
        - background[None, None, :],
        axis=2,
    )

    return difference > 34.0


def longest_vertical_region(mask):
    active = (
        np.count_nonzero(mask, axis=1)
        >= 2
    )

    regions = []
    start = None

    for y, value in enumerate(active):
        if value and start is None:
            start = y

        elif not value and start is not None:
            regions.append(
                (start, y)
            )

            start = None

    if start is not None:
        regions.append(
            (start, len(active))
        )

    if not regions:
        raise RuntimeError(
            "Could not locate weapon silhouette."
        )

    return max(
        regions,
        key=lambda region:
            region[1] - region[0],
    )


def row_runs(row):
    indices = np.flatnonzero(row)

    if not len(indices):
        return []

    runs = []

    start = int(indices[0])
    previous = start

    for value in indices[1:]:
        value = int(value)

        if value > previous + 1:
            runs.append(
                (start, previous)
            )

            start = value

        previous = value

    runs.append(
        (start, previous)
    )

    return runs


def sample_runs(
    mask,
    y0,
    y1,
    t,
):
    y = int(
        round(
            y0
            + np.clip(t, 0.0, 1.0)
            * max(1, y1 - y0 - 1)
        )
    )

    all_runs = []

    for offset in (
        0,
        -1,
        1,
        -2,
        2,
    ):
        yy = y + offset

        if yy < y0 or yy >= y1:
            continue

        for x0, x1 in row_runs(
            mask[yy]
        ):
            if x1 - x0 >= 2:
                all_runs.append(
                    (x0, x1)
                )

    if not all_runs:
        return []

    # Merge nearly-identical runs from neighboring rows.
    all_runs.sort()

    merged = []

    for run in all_runs:
        if not merged:
            merged.append(run)
            continue

        previous = merged[-1]

        if (
            abs(run[0] - previous[0]) <= 4
            and
            abs(run[1] - previous[1]) <= 4
        ):
            merged[-1] = (
                int(
                    (run[0] + previous[0])
                    * 0.5
                ),
                int(
                    (run[1] + previous[1])
                    * 0.5
                ),
            )
        else:
            merged.append(run)

    return merged


# ============================================================
# MESH BUILDER
# ============================================================

class Mesh:
    def __init__(self):
        self.vertices = []
        self.faces = []
        self.face_materials = []
        self.face_smooth = []
        self.parts = {}

    def begin(self, name):
        self.parts[name] = {
            "vertex_begin":
                len(self.vertices),

            "vertex_end":
                None,
        }

    def end(self, name):
        self.parts[name][
            "vertex_end"
        ] = len(self.vertices)

    def v(self, x, y, z):
        self.vertices.append(
            (
                float(x),
                float(y),
                float(z),
            )
        )

        return len(self.vertices) - 1

    def f(
        self,
        a,
        b,
        c,
        material,
        smooth=False,
    ):
        self.faces.append(
            (a, b, c)
        )

        self.face_materials.append(
            int(material)
        )

        self.face_smooth.append(
            bool(smooth)
        )

    def quad(
        self,
        a,
        b,
        c,
        d,
        material,
        smooth=False,
    ):
        self.f(
            a,
            b,
            c,
            material,
            smooth,
        )

        self.f(
            a,
            c,
            d,
            material,
            smooth,
        )


def octahedron(
    mesh,
    name,
    center,
    rx,
    ry,
    rz,
    material,
):
    mesh.begin(name)

    cx, cy, cz = center

    top = mesh.v(
        cx,
        cy + ry,
        cz,
    )

    bottom = mesh.v(
        cx,
        cy - ry,
        cz,
    )

    left = mesh.v(
        cx - rx,
        cy,
        cz,
    )

    front = mesh.v(
        cx,
        cy,
        cz + rz,
    )

    right = mesh.v(
        cx + rx,
        cy,
        cz,
    )

    back = mesh.v(
        cx,
        cy,
        cz - rz,
    )

    ring = [
        left,
        front,
        right,
        back,
    ]

    for i in range(4):
        j = (
            i + 1
        ) % 4

        mesh.f(
            top,
            ring[i],
            ring[j],
            material,
            False,
        )

        mesh.f(
            bottom,
            ring[j],
            ring[i],
            material,
            False,
        )

    mesh.end(name)


# ============================================================
# LOAD CONCEPT
# ============================================================

if not INPUT.exists():
    raise RuntimeError(
        f"Missing concept image: {INPUT}"
    )

OUTPUT.mkdir(
    parents=True,
    exist_ok=True,
)

image = Image.open(
    INPUT
).convert("RGB")

pixels = np.asarray(
    image
)

height, width, _ = (
    pixels.shape
)


# Generated concept layout:
#
# 3/4       SIDE        FRONT
#
# We deliberately use the middle SIDE weapon.
side_box = (
    int(width * 0.275),
    int(height * 0.105),
    int(width * 0.475),
    int(height * 0.865),
)

front_box = (
    int(width * 0.485),
    int(height * 0.105),
    int(width * 0.590),
    int(height * 0.865),
)


side = pixels[
    side_box[1]:side_box[3],
    side_box[0]:side_box[2],
]

front = pixels[
    front_box[1]:front_box[3],
    front_box[0]:front_box[2],
]


side_mask = make_mask(
    side
)

front_mask = make_mask(
    front
)


side_region = (
    longest_vertical_region(
        side_mask
    )
)

front_region = (
    longest_vertical_region(
        front_mask
    )
)


Image.fromarray(
    side
).save(
    OUTPUT / "side_crop.png"
)

Image.fromarray(
    front
).save(
    OUTPUT / "front_crop.png"
)

Image.fromarray(
    side_mask.astype(
        np.uint8
    ) * 255
).save(
    OUTPUT / "side_mask.png"
)

Image.fromarray(
    front_mask.astype(
        np.uint8
    ) * 255
).save(
    OUTPUT / "front_mask.png"
)


sy0, sy1 = side_region
fy0, fy1 = front_region

weapon_height = float(
    max(
        1,
        sy1 - sy0,
    )
)


# ============================================================
# LOCAL COORDINATES
# ============================================================

# Determine handle center from lower part of SIDE view.
handle_centers = []

crop_center_x = (
    side_mask.shape[1]
    * 0.5
)

for t in np.linspace(
    0.78,
    0.90,
    8,
):
    runs = sample_runs(
        side_mask,
        sy0,
        sy1,
        float(t),
    )

    if not runs:
        continue

    selected = min(
        runs,
        key=lambda r:
            abs(
                (
                    (r[0] + r[1])
                    * 0.5
                )
                - crop_center_x
            ),
    )

    handle_centers.append(
        (
            selected[0]
            + selected[1]
        )
        * 0.5
    )


if not handle_centers:
    raise RuntimeError(
        "Could not identify handle axis."
    )


neutral_x = float(
    np.median(
        handle_centers
    )
)


def lx(pixel_x):
    return (
        (
            float(pixel_x)
            - neutral_x
        )
        / weapon_height
        * 2.0
    )


def ly(t):
    return (
        1.0
        - 2.0 * float(t)
    )


def front_depth(t):
    runs = sample_runs(
        front_mask,
        fy0,
        fy1,
        t,
    )

    if not runs:
        return 0.040

    x0 = min(
        run[0]
        for run in runs
    )

    x1 = max(
        run[1]
        for run in runs
    )

    raw = (
        float(x1 - x0)
        / weapon_height
        * 2.0
    )

    return float(
        np.clip(
            raw,
            0.026,
            0.095,
        )
    )


# ============================================================
# SIDE-VIEW SEMANTIC EXTRACTION
# ============================================================

def main_blade(t):
    runs = sample_runs(
        side_mask,
        sy0,
        sy1,
        t,
    )

    if not runs:
        raise RuntimeError(
            f"No blade silhouette at t={t}"
        )

    # Main outer crescent is the right-most
    # substantial silhouette in our SIDE view.
    run = max(
        runs,
        key=lambda r:
            (
                r[0] + r[1]
            )
            * 0.5,
    )

    return (
        lx(run[0]),
        lx(run[1]),
        front_depth(t),
    )


def inner_hook(t):
    runs = sample_runs(
        side_mask,
        sy0,
        sy1,
        t,
    )

    if len(runs) < 2:
        return None

    main = max(
        runs,
        key=lambda r:
            (
                r[0] + r[1]
            )
            * 0.5,
    )

    main_center = (
        main[0] + main[1]
    ) * 0.5

    candidates = []

    for run in runs:
        if run == main:
            continue

        center = (
            run[0] + run[1]
        ) * 0.5

        width_px = (
            run[1] - run[0]
        )

        if (
            center
            < main_center - 5
            and width_px >= 4
        ):
            candidates.append(
                run
            )

    if not candidates:
        return None

    # The Awakened concept has three structures here:
    #
    #     LEFT   = inner crescent / hook
    #     MIDDLE = floating crystal / shards
    #     RIGHT  = main crescent
    #
    # Selecting the widest candidate is WRONG because the
    # central crystal is sometimes wider than the thin hook.
    #
    # Follow the left-most substantial structure instead.
    run = min(
        candidates,
        key=lambda r:
            (r[0] + r[1]) * 0.5,
    )

    return (
        lx(run[0]),
        lx(run[1]),
        max(
            0.022,
            front_depth(t)
            * 0.72,
        ),
    )


def negative_space_center(t):
    hook = inner_hook(t)

    outer = main_blade(t)

    if hook is None:
        return (
            outer[0]
            - 0.10
        )

    hook_right = hook[1]
    main_left = outer[0]

    return (
        hook_right
        + main_left
    ) * 0.5


# ============================================================
# BUILD
# ============================================================

mesh = Mesh()


# ============================================================
# MAIN CRESCENT
# ============================================================

mesh.begin(
    "main_blade"
)

main_ts = [
    0.055,
    0.110,
    0.180,
    0.265,
    0.350,
    0.435,
    0.515,
    0.585,
    0.645,
]


tip_profile = main_blade(
    0.020
)

tip = mesh.v(
    (
        tip_profile[0]
        + tip_profile[1]
    )
    * 0.5,
    ly(0.020),
    0.0,
)


sections = []

for t in main_ts:
    x0, x1, depth = (
        main_blade(t)
    )

    width_local = max(
        0.025,
        x1 - x0,
    )

    ridge_x = (
        x0
        + width_local
        * 0.58
    )

    half_depth = max(
        0.014,
        min(
            depth * 0.50,
            width_local * 0.38,
        ),
    )

    y = ly(t)

    sections.append(
        (
            mesh.v(
                x0,
                y,
                0.0,
            ),
            mesh.v(
                ridge_x,
                y,
                half_depth,
            ),
            mesh.v(
                x1,
                y,
                0.0,
            ),
            mesh.v(
                ridge_x,
                y,
                -half_depth,
            ),
        )
    )


# ------------------------------------------------------------
# Semantic blade root.
#
# Do NOT sample the guard/blade intersection from the image.
# The blade must terminate on the common weapon axis x=0.
# ------------------------------------------------------------

blade_root_t = 0.695

blade_root_width = 0.105

blade_root_depth = 0.052

blade_root_y = ly(
    blade_root_t
)

blade_root_half_width = (
    blade_root_width * 0.5
)

blade_root_half_depth = (
    blade_root_depth * 0.5
)


sections.append(
    (
        # spine
        mesh.v(
            -blade_root_half_width,
            blade_root_y,
            0.0,
        ),

        # front ridge
        mesh.v(
            0.006,
            blade_root_y,
            blade_root_half_depth,
        ),

        # cutting edge
        mesh.v(
            blade_root_half_width,
            blade_root_y,
            0.0,
        ),

        # back ridge
        mesh.v(
            0.006,
            blade_root_y,
            -blade_root_half_depth,
        ),
    )
)


first = sections[0]

for i in range(4):
    mesh.f(
        tip,
        first[i],
        first[(i + 1) % 4],
        MAT_EDGE,
        False,
    )


for segment in range(
    len(sections) - 1
):
    a = sections[segment]
    b = sections[segment + 1]

    edge_material = (
        MAT_MOONLIGHT
        if segment < 4
        else MAT_MOONSTONE
    )

    mesh.quad(
        a[0],
        a[1],
        b[1],
        b[0],
        MAT_MOONSTONE,
        False,
    )

    mesh.quad(
        a[1],
        a[2],
        b[2],
        b[1],
        edge_material,
        False,
    )

    mesh.quad(
        a[2],
        a[3],
        b[3],
        b[2],
        edge_material,
        False,
    )

    mesh.quad(
        a[3],
        a[0],
        b[0],
        b[3],
        MAT_MOONSTONE,
        False,
    )


last = sections[-1]

base_center = mesh.v(
    np.mean([
        mesh.vertices[i][0]
        for i in last
    ]),
    np.mean([
        mesh.vertices[i][1]
        for i in last
    ]),
    0.0,
)

for i in range(4):
    mesh.f(
        last[i],
        last[(i + 1) % 4],
        base_center,
        MAT_MOONSTONE,
        False,
    )


mesh.end(
    "main_blade"
)


# ============================================================
# INNER HOOK
# ============================================================

mesh.begin(
    "inner_hook"
)

hook_samples = []

for t in [
    0.395,
    0.450,
    0.485,
    0.515,
    0.555,
]:
    profile = inner_hook(t)

    if profile is not None:
        hook_samples.append(
            (t, profile)
        )


if len(hook_samples) < 4:
    raise RuntimeError(
        "Concept extraction found too little inner-hook geometry."
    )


first_t, first_profile = (
    hook_samples[0]
)

hook_tip = mesh.v(
    (
        first_profile[0]
        + first_profile[1]
    )
    * 0.5,
    ly(first_t),
    0.0,
)


hook_sections = []

for t, profile in hook_samples[1:]:
    x0, x1, depth = profile

    width_local = max(
        0.020,
        x1 - x0,
    )

    ridge = (
        x0
        + width_local * 0.53
    )

    half_depth = max(
        0.012,
        min(
            depth * 0.50,
            width_local * 0.35,
        ),
    )

    y = ly(t)

    hook_sections.append(
        (
            mesh.v(
                x0,
                y,
                0.0,
            ),
            mesh.v(
                ridge,
                y,
                half_depth,
            ),
            mesh.v(
                x1,
                y,
                0.0,
            ),
            mesh.v(
                ridge,
                y,
                -half_depth,
            ),
        )
    )


first = hook_sections[0]

for i in range(4):
    mesh.f(
        hook_tip,
        first[i],
        first[(i + 1) % 4],
        MAT_EDGE,
        False,
    )


for segment in range(
    len(hook_sections) - 1
):
    a = hook_sections[segment]
    b = hook_sections[segment + 1]

    mesh.quad(
        a[0],
        a[1],
        b[1],
        b[0],
        MAT_MOONLIGHT,
        False,
    )

    mesh.quad(
        a[1],
        a[2],
        b[2],
        b[1],
        MAT_EDGE,
        False,
    )

    mesh.quad(
        a[2],
        a[3],
        b[3],
        b[2],
        MAT_EDGE,
        False,
    )

    mesh.quad(
        a[3],
        a[0],
        b[0],
        b[3],
        MAT_MOONLIGHT,
        False,
    )


last = hook_sections[-1]

# Semantic attachment point where the inner crescent merges
# back into the main blade.
#
# At ~t=.61 the concept's hook and main blade visually join.
hook_attach_t = 0.610

hook_attach_profile = main_blade(
    hook_attach_t
)

hook_base = mesh.v(
    hook_attach_profile[0],
    ly(hook_attach_t),
    0.0,
)

for i in range(4):
    mesh.f(
        last[i],
        last[(i + 1) % 4],
        hook_base,
        MAT_MOONLIGHT,
        False,
    )


mesh.end(
    "inner_hook"
)


# ============================================================
# SUSPENDED CORE + SHARDS
#
# Do NOT guess their vertical positions anymore.
#
# The concept intentionally contains disconnected floating
# moonstone pieces. Detect those components directly from the
# SIDE VIEW mask and use their image positions.
# ============================================================

from collections import deque


def mask_components(mask):
    height, width = mask.shape

    visited = np.zeros(
        mask.shape,
        dtype=bool,
    )

    components = []

    # 8-connected neighborhood.
    neighbors = [
        (-1, -1),
        (-1,  0),
        (-1,  1),
        ( 0, -1),
        ( 0,  1),
        ( 1, -1),
        ( 1,  0),
        ( 1,  1),
    ]

    ys, xs = np.nonzero(mask)

    for start_y, start_x in zip(
        ys.tolist(),
        xs.tolist(),
    ):
        if visited[start_y, start_x]:
            continue

        queue = deque([
            (start_y, start_x)
        ])

        visited[
            start_y,
            start_x
        ] = True

        pixels_found = []

        while queue:
            y, x = queue.popleft()

            pixels_found.append(
                (y, x)
            )

            for dy, dx in neighbors:
                yy = y + dy
                xx = x + dx

                if (
                    yy < 0
                    or yy >= height
                    or xx < 0
                    or xx >= width
                ):
                    continue

                if visited[yy, xx]:
                    continue

                if not mask[yy, xx]:
                    continue

                visited[yy, xx] = True

                queue.append(
                    (yy, xx)
                )

        if not pixels_found:
            continue

        values = np.asarray(
            pixels_found,
            dtype=np.float64,
        )

        component_y = values[:, 0]
        component_x = values[:, 1]

        components.append({
            "area":
                len(pixels_found),

            "cx":
                float(
                    component_x.mean()
                ),

            "cy":
                float(
                    component_y.mean()
                ),

            "xmin":
                int(
                    component_x.min()
                ),

            "xmax":
                int(
                    component_x.max()
                ),

            "ymin":
                int(
                    component_y.min()
                ),

            "ymax":
                int(
                    component_y.max()
                ),
        })

    return components


components = mask_components(
    side_mask
)

components.sort(
    key=lambda c:
        c["area"],
    reverse=True,
)


if not components:
    raise RuntimeError(
        "Could not find connected components "
        "in SIDE VIEW."
    )


# Largest connected component is the actual weapon body.
weapon_component = components[0]


floating_candidates = []

for component in components[1:]:

    # Convert component centroid to generator t.
    t = (
        (
            component["cy"]
            - sy0
        )
        / max(
            1.0,
            float(sy1 - sy0)
        )
    )

    # Floating lunar core/shards live inside the blade region.
    #
    # Ignore tiny antialiasing noise.
    if (
        0.25 <= t <= 0.68
        and component["area"] >= 4
    ):
        floating_candidates.append(
            (
                t,
                component,
            )
        )


# ------------------------------------------------------------
# Prefer concept-derived floating geometry.
#
# Largest disconnected component = main crystal.
# Remaining components above/below it = shards.
#
# Fall back gracefully if the mask threshold happened to join
# something to the blade.
# ------------------------------------------------------------

if floating_candidates:

    floating_candidates.sort(
        key=lambda item:
            item[1]["area"],
        reverse=True,
    )


    core_t = float(
        floating_candidates[0][0]
    )

    core_component = (
        floating_candidates[0][1]
    )

    core_x = lx(
        core_component["cx"]
    )


    other_components = sorted(
        floating_candidates[1:],
        key=lambda item:
            item[0],
    )


    above = [
        item
        for item in other_components
        if item[0] < core_t
    ]

    below = [
        item
        for item in other_components
        if item[0] > core_t
    ]


    if above:
        upper_t, upper_component = (
            max(
                above,
                key=lambda item:
                    item[0],
            )
        )

        upper_x = lx(
            upper_component["cx"]
        )
    else:
        upper_t = (
            core_t - 0.075
        )

        upper_x = (
            negative_space_center(
                upper_t
            )
        )


    if below:
        lower_t, lower_component = (
            min(
                below,
                key=lambda item:
                    item[0],
            )
        )

        lower_x = lx(
            lower_component["cx"]
        )
    else:
        lower_t = (
            core_t + 0.075
        )

        lower_x = (
            negative_space_center(
                lower_t
            )
        )


    print()
    print(
        "Floating concept components detected:"
    )

    print(
        "  core  :",
        f"t={core_t:.4f}",
        f"x={core_x:.4f}",
        f"area={core_component['area']}",
    )

    print(
        "  upper :",
        f"t={upper_t:.4f}",
        f"x={upper_x:.4f}",
    )

    print(
        "  lower :",
        f"t={lower_t:.4f}",
        f"x={lower_x:.4f}",
    )

else:

    print()
    print(
        "WARNING: floating components were not "
        "cleanly separated in the mask."
    )

    # Conservative fallback.
    core_t = 0.400

    core_x = (
        negative_space_center(
            core_t
        )
    )

    upper_t = 0.335

    upper_x = (
        negative_space_center(
            upper_t
        )
    )

    lower_t = 0.470

    lower_x = (
        negative_space_center(
            lower_t
        )
    )


octahedron(
    mesh,
    "crystal_core",
    (
        core_x,
        ly(core_t),
        0.0,
    ),
    0.060,
    0.105,
    0.050,
    MAT_CORE,
)


octahedron(
    mesh,
    "upper_shard",
    (
        upper_x,
        ly(upper_t),
        0.0,
    ),
    0.018,
    0.036,
    0.016,
    MAT_EDGE,
)


octahedron(
    mesh,
    "lower_shard",
    (
        lower_x,
        ly(lower_t),
        0.0,
    ),
    0.018,
    0.034,
    0.016,
    MAT_EDGE,
)


# ============================================================
# WINGED GUARD
# ============================================================

mesh.begin(
    "guard"
)

guard_y = ly(
    0.715
)

guard_depth = 0.050


outline = [
    (-0.33,  0.000),
    (-0.20,  0.060),
    (-0.11,  0.082),
    (-0.050, 0.028),
    ( 0.000, 0.100),
    ( 0.050, 0.028),
    ( 0.11,  0.082),
    ( 0.20,  0.060),
    ( 0.33,  0.000),
    ( 0.16, -0.065),
    ( 0.055,-0.030),
    ( 0.000,-0.105),
    (-0.055,-0.030),
    (-0.16, -0.065),
]


front_indices = []
back_indices = []

for x, dy in outline:
    front_indices.append(
        mesh.v(
            x,
            guard_y + dy,
            guard_depth,
        )
    )

    back_indices.append(
        mesh.v(
            x,
            guard_y + dy,
            -guard_depth,
        )
    )


front_center = mesh.v(
    0.0,
    guard_y,
    guard_depth,
)

back_center = mesh.v(
    0.0,
    guard_y,
    -guard_depth,
)


for i in range(len(outline)):
    j = (
        i + 1
    ) % len(outline)

    mesh.f(
        front_center,
        front_indices[i],
        front_indices[j],
        MAT_SILVER,
        False,
    )

    mesh.f(
        back_center,
        back_indices[j],
        back_indices[i],
        MAT_SILVER,
        False,
    )

    mesh.quad(
        front_indices[i],
        back_indices[i],
        back_indices[j],
        front_indices[j],
        MAT_SILVER,
        False,
    )


mesh.end(
    "guard"
)


octahedron(
    mesh,
    "guard_gem",
    (
        0.0,
        guard_y,
        0.060,
    ),
    0.045,
    0.067,
    0.030,
    MAT_CORE,
)


# ============================================================
# GRIP
# ============================================================

mesh.begin(
    "grip"
)

grip_ts = [
    0.755,
    0.815,
    0.870,
    0.920,
]

grip_rings = []

for ring_index, t in enumerate(
    grip_ts
):
    taper = (
        1.0
        - ring_index * 0.035
    )

    rx = (
        0.055
        * taper
    )

    rz = (
        0.042
        * taper
    )

    phase = (
        ring_index
        * np.pi / 10.0
    )

    ring = []

    for i in range(8):
        angle = (
            2.0
            * np.pi
            * i / 8.0
            + phase
        )

        ring.append(
            mesh.v(
                np.cos(angle) * rx,
                ly(t),
                np.sin(angle) * rz,
            )
        )

    grip_rings.append(
        ring
    )


for segment in range(
    len(grip_rings) - 1
):
    a = grip_rings[segment]
    b = grip_rings[segment + 1]

    for i in range(8):
        j = (
            i + 1
        ) % 8

        mesh.quad(
            a[i],
            a[j],
            b[j],
            b[i],
            MAT_GRIP,
            True,
        )


mesh.end(
    "grip"
)


# ============================================================
# POMMEL
# ============================================================

octahedron(
    mesh,
    "pommel",
    (
        0.0,
        ly(0.965),
        0.0,
    ),
    0.078,
    0.090,
    0.055,
    MAT_SILVER,
)


octahedron(
    mesh,
    "pommel_gem",
    (
        0.0,
        ly(0.965),
        0.053,
    ),
    0.037,
    0.055,
    0.024,
    MAT_CORE,
)


# ============================================================
# CONVERT
# ============================================================

vertices = np.asarray(
    mesh.vertices,
    dtype=np.float64,
)

faces = np.asarray(
    mesh.faces,
    dtype=np.int32,
)


# ============================================================
# WRITE MTL
# ============================================================

with MTL.open(
    "w",
    encoding="utf-8",
) as file:

    for name, rgb in MATERIALS:
        r, g, b = (
            np.asarray(
                rgb,
                dtype=np.float64,
            )
            / 255.0
        )

        file.write(
            f"newmtl {name}\n"
        )

        file.write(
            f"Kd {r:.6f} "
            f"{g:.6f} "
            f"{b:.6f}\n"
        )

        file.write(
            "Ka 0 0 0\n"
        )

        file.write(
            "Ks 0 0 0\n"
        )

        file.write(
            "illum 1\n\n"
        )


# ============================================================
# WRITE OBJ
# ============================================================

material_names = [
    name
    for name, _ in MATERIALS
]


with OBJ.open(
    "w",
    encoding="utf-8",
) as file:

    file.write(
        f"mtllib {MTL.name}\n"
    )

    file.write(
        "o Awakened_Moonstone_Scimitar\n"
    )

    for x, y, z in vertices:
        file.write(
            f"v {x:.6f} "
            f"{y:.6f} "
            f"{z:.6f}\n"
        )


    current_material = None
    current_smooth = None

    for (
        face,
        material_index,
        is_smooth,
    ) in zip(
        mesh.faces,
        mesh.face_materials,
        mesh.face_smooth,
    ):

        material_name = (
            material_names[
                material_index
            ]
        )

        if (
            material_name
            != current_material
        ):
            file.write(
                f"usemtl {material_name}\n"
            )

            current_material = (
                material_name
            )


        if (
            bool(is_smooth)
            != current_smooth
        ):
            file.write(
                "s 1\n"
                if is_smooth
                else "s off\n"
            )

            current_smooth = bool(
                is_smooth
            )


        a, b, c = (
            int(face[0]) + 1,
            int(face[1]) + 1,
            int(face[2]) + 1,
        )

        file.write(
            f"f {a} {b} {c}\n"
        )


# ============================================================
# SEMANTIC META
# ============================================================

grip_part = (
    mesh.parts["grip"]
)

grip_vertices = vertices[
    grip_part["vertex_begin"]:
    grip_part["vertex_end"]
]

# ------------------------------------------------------------
# Semantic HAND anchor.
#
# IMPORTANT:
#
# The grip centroid is NOT necessarily where a player's hand
# should hold the weapon.
#
# Evolution variants may have:
#
#   longer grips
#   larger guards
#   larger pommels
#   different ring spacing
#
# Keep "grip" as a semantic mesh component, but expose an
# independent attachment point for the actual hand.
#
# This position corresponds to approximately the successful
# hold location established by Moonstone V3.
# ------------------------------------------------------------

hand_t = 0.865

hand_anchor = np.asarray([
    0.0,
    ly(hand_t),
    0.0,
], dtype=np.float64)


meta = {
    "name":
        "Awakened Moonstone Scimitar",

    "family":
        "moonstone_scimitar",

    "evolution_tier":
        2,

    "archetype":
        "scimitar",

    "worn_reference_model":
        490,

    "parts":
        mesh.parts,

    "anchors": {
        "grip":
            grip_vertices.mean(
                axis=0
            ).tolist(),

        "hand":
            hand_anchor.tolist(),

        "blade_tip":
            vertices[tip].tolist(),

        "crystal_core": [
            float(core_x),
            float(ly(core_t)),
            0.0,
        ],
    },
}


META.write_text(
    json.dumps(
        meta,
        indent=2,
    )
    + "\n",
    encoding="utf-8",
)


# ============================================================
# REPORT
# ============================================================

flat_faces = sum(
    not value
    for value in mesh.face_smooth
)

smooth_faces = sum(
    value
    for value in mesh.face_smooth
)


print()
print(
    "========================================"
)

print(
    " AWAKENED MOONSTONE SCIMITAR"
)

print(
    "========================================"
)

print(
    "Vertices :",
    len(vertices),
)

print(
    "Triangles:",
    len(faces),
)

print(
    "Flat     :",
    flat_faces,
)

print(
    "Smooth   :",
    smooth_faces,
)

print()

print(
    "Semantic parts:"
)

for name, part in mesh.parts.items():
    begin = part[
        "vertex_begin"
    ]

    end = part[
        "vertex_end"
    ]

    print(
        f"  {name:18s}"
        f"{begin:3d} .. "
        f"{end - 1:3d}"
        f"   ({end - begin} verts)"
    )


print()

print(
    "Grip:",
    grip_part["vertex_begin"],
    "..",
    grip_part["vertex_end"] - 1,
)

print()

print(
    "OBJ :",
    OBJ,
)

print(
    "MTL :",
    MTL,
)

print(
    "META:",
    META,
)
