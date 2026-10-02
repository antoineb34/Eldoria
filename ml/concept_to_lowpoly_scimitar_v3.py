#!/usr/bin/env python3

import argparse
from collections import Counter
from pathlib import Path

import numpy as np
from PIL import Image

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection


# ============================================================
# MATERIALS
#
# Intentionally SMALL palette.
#
# We are NOT painting fake highlights/shadows into the model.
# Geometry + Eldoria lighting should do most of the work.
# ============================================================

MATERIALS = [
    ("moonstone",        (135, 185, 225)),
    ("moonstone_accent", (170, 215, 240)),
    ("silver",           (170, 180, 195)),
    ("dark_grip",        (55, 65, 85)),
    ("cyan_gem",         (105, 220, 235)),
]

MAT_MOONSTONE = 0
MAT_ACCENT = 1
MAT_SILVER = 2
MAT_GRIP = 3
MAT_GEM = 4


def parse_args():
    p = argparse.ArgumentParser()

    p.add_argument(
        "image",
        type=Path,
    )

    p.add_argument(
        "--output-dir",
        type=Path,
        default=Path(
            "dataset/ml/generated_moonstone_scimitar_v3"
        ),
    )

    return p.parse_args()


# ============================================================
# MASK / CONCEPT EXTRACTION
# ============================================================

def make_mask(crop):
    crop = crop.astype(np.uint8)

    h, w, _ = crop.shape

    border_size = max(
        2,
        min(15, h // 8, w // 8),
    )

    border = np.concatenate([
        crop[:border_size].reshape(-1, 3),
        crop[-border_size:].reshape(-1, 3),
        crop[:, :border_size].reshape(-1, 3),
        crop[:, -border_size:].reshape(-1, 3),
    ]).astype(np.float64)

    background = np.median(
        border,
        axis=0,
    )

    difference = np.linalg.norm(
        crop.astype(np.float64)
        - background[None, None, :],
        axis=2,
    )

    return difference > 35.0


def longest_vertical_region(mask):
    active = np.count_nonzero(
        mask,
        axis=1,
    ) >= 2

    best_start = None
    best_end = None
    current_start = None

    for y, value in enumerate(active):
        if value:
            if current_start is None:
                current_start = y
        else:
            if current_start is not None:
                if (
                    best_start is None
                    or y - current_start
                    > best_end - best_start
                ):
                    best_start = current_start
                    best_end = y

                current_start = None

    if current_start is not None:
        y = len(active)

        if (
            best_start is None
            or y - current_start
            > best_end - best_start
        ):
            best_start = current_start
            best_end = y

    if best_start is None:
        raise RuntimeError(
            "Could not find weapon silhouette."
        )

    return best_start, best_end


def sample_extent(mask, y0, y1, t):
    y = int(
        round(
            y0
            + np.clip(t, 0.0, 1.0)
            * max(1, y1 - y0 - 1)
        )
    )

    xs = []

    # Small vertical neighborhood protects us from one noisy
    # image row without globally smoothing away the silhouette.
    for yy in range(
        max(y0, y - 2),
        min(y1, y + 3),
    ):
        found = np.flatnonzero(
            mask[yy]
        )

        if len(found):
            xs.extend(
                found.tolist()
            )

    if not xs:
        # Search outward if the exact station happens to cross
        # a tiny mask gap.
        for radius in range(3, 15):
            for yy in (
                y - radius,
                y + radius,
            ):
                if yy < y0 or yy >= y1:
                    continue

                found = np.flatnonzero(
                    mask[yy]
                )

                if len(found):
                    xs.extend(
                        found.tolist()
                    )

            if xs:
                break

    if not xs:
        raise RuntimeError(
            f"Could not sample silhouette at t={t:.3f}"
        )

    xmin = float(np.percentile(xs, 5))
    xmax = float(np.percentile(xs, 95))

    return xmin, xmax, max(1.0, xmax - xmin)


def robust_profile(
    t,
    side_mask,
    side_region,
    front_mask,
    front_region,
    neutral_center,
    pixel_height,
):
    centers = []
    widths = []
    depths = []

    # Very local median only.
    #
    # We're trying to remove raster noise, not make a smooth
    # noodle out of the shape.
    for dt in (
        -0.010,
        0.0,
        0.010,
    ):
        sample_t = float(
            np.clip(
                t + dt,
                0.0,
                1.0,
            )
        )

        sx0, sx1, sw = sample_extent(
            side_mask,
            *side_region,
            sample_t,
        )

        _, _, fw = sample_extent(
            front_mask,
            *front_region,
            sample_t,
        )

        centers.append(
            (sx0 + sx1) * 0.5
        )

        widths.append(sw)
        depths.append(fw)

    center_px = float(
        np.median(centers)
    )

    width_px = float(
        np.median(widths)
    )

    depth_px = float(
        np.median(depths)
    )

    center_x = (
        (center_px - neutral_center)
        / pixel_height
        * 2.0
    )

    width = (
        width_px
        / pixel_height
        * 2.0
    )

    depth = (
        depth_px
        / pixel_height
        * 2.0
    )

    return (
        center_x,
        max(width, 0.025),
        max(depth, 0.025),
    )


# ============================================================
# MESH HELPERS
# ============================================================

def add_vertex(vertices, x, y, z):
    vertices.append(
        (
            float(x),
            float(y),
            float(z),
        )
    )

    return len(vertices) - 1


def add_face(
    faces,
    materials,
    smooth,
    a,
    b,
    c,
    material,
    is_smooth,
):
    faces.append(
        (a, b, c)
    )

    materials.append(
        material
    )

    smooth.append(
        bool(is_smooth)
    )


def add_quad(
    faces,
    materials,
    smooth,
    a,
    b,
    c,
    d,
    material,
    is_smooth,
):
    add_face(
        faces,
        materials,
        smooth,
        a, b, c,
        material,
        is_smooth,
    )

    add_face(
        faces,
        materials,
        smooth,
        a, c, d,
        material,
        is_smooth,
    )


# ============================================================
# BLADE
#
# NO RING EXTRUSION.
#
# Cross section:
#
#                  ridge_front
#                      *
#                    /   \
#                  /       \
#             spine *       * cutting edge
#                  \       /
#                    \   /
#                      *
#                  ridge_back
#
# Spine and cutting edge have zero thickness.
#
# The blade is intentionally flat shaded, like the decorative
# blade region we found on native Dragon scimitar 6031.
# ============================================================

def build_blade(
    vertices,
    faces,
    materials,
    smooth,
    profile,
):
    # One actual tip followed by only SEVEN meaningful stations.
    #
    # Non-uniform positions:
    # more control where silhouette changes matter.
    station_ts = [
        0.07,
        0.15,
        0.27,
        0.40,
        0.53,
        0.64,
        0.74,
    ]

    tip_center, _, _ = profile(
        0.02
    )

    tip = add_vertex(
        vertices,
        tip_center,
        1.0,
        0.0,
    )

    rings = []

    for t in station_ts:
        center, width, depth = profile(
            t
        )

        y = 1.0 - 2.0 * t

        spine_x = (
            center - width * 0.50
        )

        edge_x = (
            center + width * 0.50
        )

        # Raised longitudinal ridge.
        #
        # Slightly biased toward the cutting edge instead of
        # sitting mathematically in the center.
        ridge_x = (
            spine_x + width * 0.55
        )

        # Don't let front-view noise make enormous thickness.
        depth = min(
            depth,
            width * 0.45,
        )

        half_depth = max(
            0.015,
            depth * 0.50,
        )

        spine = add_vertex(
            vertices,
            spine_x,
            y,
            0.0,
        )

        ridge_front = add_vertex(
            vertices,
            ridge_x,
            y,
            half_depth,
        )

        edge = add_vertex(
            vertices,
            edge_x,
            y,
            0.0,
        )

        ridge_back = add_vertex(
            vertices,
            ridge_x,
            y,
            -half_depth,
        )

        rings.append(
            (
                spine,
                ridge_front,
                edge,
                ridge_back,
            )
        )

    # --------------------------------------------------------
    # Tip fan.
    # --------------------------------------------------------

    first = rings[0]

    for i in range(4):
        a = first[i]
        b = first[
            (i + 1) % 4
        ]

        add_face(
            faces,
            materials,
            smooth,
            tip,
            a,
            b,
            MAT_ACCENT,
            False,
        )

    # --------------------------------------------------------
    # Deliberate longitudinal facets.
    #
    # 8 triangles per station interval:
    #
    # front:
    #     spine -> ridge
    #     ridge -> edge
    #
    # back:
    #     edge -> ridge
    #     ridge -> spine
    #
    # That's it.
    # --------------------------------------------------------

    for segment in range(
        len(rings) - 1
    ):
        a = rings[segment]
        b = rings[segment + 1]

        # Only the upper cutting-edge region gets our slightly
        # brighter material.
        #
        # This is semantic decoration, NOT baked lighting.
        edge_material = (
            MAT_ACCENT
            if segment <= 1
            else MAT_MOONSTONE
        )

        # Front spine -> ridge.
        add_quad(
            faces,
            materials,
            smooth,
            a[0],
            a[1],
            b[1],
            b[0],
            MAT_MOONSTONE,
            False,
        )

        # Front ridge -> cutting edge.
        add_quad(
            faces,
            materials,
            smooth,
            a[1],
            a[2],
            b[2],
            b[1],
            edge_material,
            False,
        )

        # Back cutting edge -> ridge.
        add_quad(
            faces,
            materials,
            smooth,
            a[2],
            a[3],
            b[3],
            b[2],
            edge_material,
            False,
        )

        # Back ridge -> spine.
        add_quad(
            faces,
            materials,
            smooth,
            a[3],
            a[0],
            b[0],
            b[3],
            MAT_MOONSTONE,
            False,
        )

    # Cap at guard connection.
    last = rings[-1]

    center = np.mean(
        np.asarray([
            vertices[i]
            for i in last
        ]),
        axis=0,
    )

    base_center = add_vertex(
        vertices,
        *center,
    )

    for i in range(4):
        add_face(
            faces,
            materials,
            smooth,
            last[i],
            last[(i + 1) % 4],
            base_center,
            MAT_MOONSTONE,
            False,
        )


# ============================================================
# GUARD
#
# Separate component.
#
# Smooth shaded so the silver furniture doesn't look like the
# same crystal surface as the blade.
# ============================================================

def build_guard(
    vertices,
    faces,
    materials,
    smooth,
    profile,
):
    t = 0.77

    center, width, depth = profile(
        t
    )

    y = 1.0 - 2.0 * t

    half_span = max(
        width * 0.58,
        0.18,
    )

    half_y = 0.055

    half_z = max(
        depth * 0.55,
        0.035,
    )

    outline = [
        (
            center - half_span,
            y,
        ),
        (
            center - half_span * 0.22,
            y + half_y,
        ),
        (
            center + half_span * 0.22,
            y + half_y,
        ),
        (
            center + half_span,
            y,
        ),
        (
            center + half_span * 0.22,
            y - half_y,
        ),
        (
            center - half_span * 0.22,
            y - half_y,
        ),
    ]

    front = []
    back = []

    for x, yy in outline:
        front.append(
            add_vertex(
                vertices,
                x,
                yy,
                half_z,
            )
        )

        back.append(
            add_vertex(
                vertices,
                x,
                yy,
                -half_z,
            )
        )

    # Front/back hexagons.
    for i in range(1, 5):
        add_face(
            faces,
            materials,
            smooth,
            front[0],
            front[i],
            front[i + 1],
            MAT_SILVER,
            True,
        )

        add_face(
            faces,
            materials,
            smooth,
            back[0],
            back[i + 1],
            back[i],
            MAT_SILVER,
            True,
        )

    # Outer rim.
    for i in range(6):
        j = (
            i + 1
        ) % 6

        add_quad(
            faces,
            materials,
            smooth,
            front[i],
            back[i],
            back[j],
            front[j],
            MAT_SILVER,
            True,
        )


# ============================================================
# GRIP
#
# Separate smooth-shaded 8-sided component.
# ============================================================

def build_grip(
    vertices,
    faces,
    materials,
    smooth,
    profile,
):
    top_t = 0.81
    bottom_t = 0.92

    top_center, top_width, top_depth = profile(
        top_t
    )

    bottom_center, bottom_width, bottom_depth = profile(
        bottom_t
    )

    # Keep the grip mostly straight.
    center = (
        top_center + bottom_center
    ) * 0.5

    radius_x = max(
        min(top_width, bottom_width)
        * 0.30,
        0.035,
    )

    radius_z = max(
        min(top_depth, bottom_depth)
        * 0.40,
        0.030,
    )

    y_top = (
        1.0 - 2.0 * top_t
    )

    y_bottom = (
        1.0 - 2.0 * bottom_t
    )

    top_ring = []
    bottom_ring = []

    for i in range(8):
        angle = (
            2.0 * np.pi
            * i / 8.0
        )

        x = (
            center
            + np.cos(angle)
            * radius_x
        )

        z = (
            np.sin(angle)
            * radius_z
        )

        top_ring.append(
            add_vertex(
                vertices,
                x,
                y_top,
                z,
            )
        )

        # Tiny taper toward pommel.
        bottom_ring.append(
            add_vertex(
                vertices,
                center
                + np.cos(angle)
                * radius_x
                * 0.90,
                y_bottom,
                np.sin(angle)
                * radius_z
                * 0.90,
            )
        )

    for i in range(8):
        j = (
            i + 1
        ) % 8

        add_quad(
            faces,
            materials,
            smooth,
            top_ring[i],
            top_ring[j],
            bottom_ring[j],
            bottom_ring[i],
            MAT_GRIP,
            True,
        )


# ============================================================
# POMMEL
#
# Small faceted gem, deliberately flat shaded.
# ============================================================

def build_pommel(
    vertices,
    faces,
    materials,
    smooth,
    profile,
):
    center_x, _, _ = profile(
        0.95
    )

    y_top = -0.83
    y_ring = -0.91
    y_bottom = -1.0

    radius_x = 0.075
    radius_z = 0.055

    top = add_vertex(
        vertices,
        center_x,
        y_top,
        0.0,
    )

    left = add_vertex(
        vertices,
        center_x - radius_x,
        y_ring,
        0.0,
    )

    front = add_vertex(
        vertices,
        center_x,
        y_ring,
        radius_z,
    )

    right = add_vertex(
        vertices,
        center_x + radius_x,
        y_ring,
        0.0,
    )

    back = add_vertex(
        vertices,
        center_x,
        y_ring,
        -radius_z,
    )

    bottom = add_vertex(
        vertices,
        center_x,
        y_bottom,
        0.0,
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

        add_face(
            faces,
            materials,
            smooth,
            top,
            ring[i],
            ring[j],
            MAT_GEM,
            False,
        )

        add_face(
            faces,
            materials,
            smooth,
            bottom,
            ring[j],
            ring[i],
            MAT_GEM,
            False,
        )


# ============================================================
# COMPLETE MESH
# ============================================================

def build_mesh(
    side_mask,
    side_region,
    front_mask,
    front_region,
):
    side_y0, side_y1 = (
        side_region
    )

    pixel_height = float(
        max(
            1,
            side_y1 - side_y0,
        )
    )

    # Same proven reference strategy as v1:
    # grip defines neutral axis.
    handle_centers = []

    for t in np.linspace(
        0.84,
        0.93,
        7,
    ):
        xmin, xmax, _ = sample_extent(
            side_mask,
            *side_region,
            float(t),
        )

        handle_centers.append(
            (
                xmin + xmax
            ) * 0.5
        )

    neutral_center = float(
        np.median(
            handle_centers
        )
    )

    def profile(t):
        return robust_profile(
            t=t,
            side_mask=side_mask,
            side_region=side_region,
            front_mask=front_mask,
            front_region=front_region,
            neutral_center=neutral_center,
            pixel_height=pixel_height,
        )

    vertices = []
    faces = []
    face_materials = []
    face_smooth = []

    build_blade(
        vertices,
        faces,
        face_materials,
        face_smooth,
        profile,
    )

    build_guard(
        vertices,
        faces,
        face_materials,
        face_smooth,
        profile,
    )

    build_grip(
        vertices,
        faces,
        face_materials,
        face_smooth,
        profile,
    )

    build_pommel(
        vertices,
        faces,
        face_materials,
        face_smooth,
        profile,
    )

    return (
        np.asarray(
            vertices,
            dtype=np.float64,
        ),
        np.asarray(
            faces,
            dtype=np.int32,
        ),
        np.asarray(
            face_materials,
            dtype=np.int32,
        ),
        np.asarray(
            face_smooth,
            dtype=bool,
        ),
    )


# ============================================================
# OUTPUT
# ============================================================

def write_mtl(path):
    with path.open(
        "w",
        encoding="utf-8",
    ) as f:
        for name, rgb in MATERIALS:
            r, g, b = (
                np.asarray(
                    rgb,
                    dtype=np.float64,
                )
                / 255.0
            )

            f.write(
                f"newmtl {name}\n"
            )

            f.write(
                f"Kd {r:.6f} "
                f"{g:.6f} "
                f"{b:.6f}\n"
            )

            f.write(
                "Ka 0 0 0\n"
            )

            f.write(
                "Ks 0 0 0\n"
            )

            f.write(
                "illum 1\n\n"
            )


def write_obj(
    path,
    mtl_name,
    vertices,
    faces,
    face_materials,
    face_smooth,
):
    material_names = [
        name
        for name, _ in MATERIALS
    ]

    with path.open(
        "w",
        encoding="utf-8",
    ) as f:
        f.write(
            f"mtllib {mtl_name}\n"
        )

        f.write(
            "o Moonstone_Scimitar_V3\n"
        )

        for x, y, z in vertices:
            f.write(
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
            faces,
            face_materials,
            face_smooth,
        ):
            material_name = (
                material_names[
                    int(material_index)
                ]
            )

            if (
                material_name
                != current_material
            ):
                f.write(
                    f"usemtl "
                    f"{material_name}\n"
                )

                current_material = (
                    material_name
                )

            # This now maps directly through our updated
            # DebugObjModelLoader:
            #
            #   s off -> renderType 1
            #   s 1   -> renderType 0
            if (
                bool(is_smooth)
                != current_smooth
            ):
                f.write(
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

            f.write(
                f"f {a} {b} {c}\n"
            )


def save_preview(
    path,
    vertices,
    faces,
    face_materials,
):
    triangles = np.asarray([
        vertices[face]
        for face in faces
    ])

    palette = np.asarray([
        rgb
        for _, rgb in MATERIALS
    ], dtype=np.float64) / 255.0

    colors = palette[
        face_materials
    ]

    figure = plt.figure(
        figsize=(8, 8),
    )

    ax = figure.add_subplot(
        111,
        projection="3d",
    )

    poly = Poly3DCollection(
        triangles,
        linewidths=0.45,
    )

    poly.set_facecolor(
        colors
    )

    poly.set_edgecolor(
        (0, 0, 0, 0.30)
    )

    ax.add_collection3d(
        poly
    )

    mins = vertices.min(
        axis=0
    )

    maxs = vertices.max(
        axis=0
    )

    spans = (
        maxs - mins
    )

    center = (
        mins + maxs
    ) * 0.5

    radius = max(
        float(spans.max()) * 0.60,
        0.1,
    )

    ax.set_xlim(
        center[0] - radius,
        center[0] + radius,
    )

    ax.set_ylim(
        center[1] - radius,
        center[1] + radius,
    )

    ax.set_zlim(
        center[2] - radius,
        center[2] + radius,
    )

    ax.set_box_aspect(
        (1, 1, 1)
    )

    ax.view_init(
        elev=18,
        azim=-60,
    )

    ax.set_axis_off()

    figure.tight_layout()

    figure.savefig(
        path,
        dpi=220,
        bbox_inches="tight",
    )

    plt.close(
        figure
    )


# ============================================================
# MAIN
# ============================================================

def main():
    args = parse_args()

    args.output_dir.mkdir(
        parents=True,
        exist_ok=True,
    )

    image = Image.open(
        args.image
    ).convert("RGB")

    pixels = np.asarray(
        image
    )

    height, width, _ = (
        pixels.shape
    )

    # Proven Moonstone concept-sheet crops.
    side_box = (
        int(width * 0.270),
        int(height * 0.100),
        int(width * 0.470),
        int(height * 0.880),
    )

    front_box = (
        int(width * 0.500),
        int(height * 0.100),
        int(width * 0.600),
        int(height * 0.880),
    )

    side_crop = pixels[
        side_box[1]:side_box[3],
        side_box[0]:side_box[2],
    ]

    front_crop = pixels[
        front_box[1]:front_box[3],
        front_box[0]:front_box[2],
    ]

    side_mask = make_mask(
        side_crop
    )

    front_mask = make_mask(
        front_crop
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

    # Debug extraction outputs.
    Image.fromarray(
        side_crop
    ).save(
        args.output_dir
        / "side_crop.png"
    )

    Image.fromarray(
        front_crop
    ).save(
        args.output_dir
        / "front_crop.png"
    )

    Image.fromarray(
        side_mask.astype(
            np.uint8
        ) * 255
    ).save(
        args.output_dir
        / "side_mask.png"
    )

    Image.fromarray(
        front_mask.astype(
            np.uint8
        ) * 255
    ).save(
        args.output_dir
        / "front_mask.png"
    )

    (
        vertices,
        faces,
        face_materials,
        face_smooth,
    ) = build_mesh(
        side_mask,
        side_region,
        front_mask,
        front_region,
    )

    obj_path = (
        args.output_dir
        / "moonstone_scimitar_v3.obj"
    )

    mtl_path = (
        args.output_dir
        / "moonstone_scimitar_v3.mtl"
    )

    preview_path = (
        args.output_dir
        / "moonstone_scimitar_v3_preview.png"
    )

    write_mtl(
        mtl_path
    )

    write_obj(
        obj_path,
        mtl_path.name,
        vertices,
        faces,
        face_materials,
        face_smooth,
    )

    save_preview(
        preview_path,
        vertices,
        faces,
        face_materials,
    )

    counts = Counter(
        int(x)
        for x in face_materials
    )

    smooth_count = int(
        np.count_nonzero(
            face_smooth
        )
    )

    flat_count = (
        len(face_smooth)
        - smooth_count
    )

    print()
    print(
        "=== MOONSTONE SCIMITAR V3 ==="
    )

    print(
        "Vertices :",
        len(vertices),
    )

    print(
        "Triangles:",
        len(faces),
    )

    print()

    print(
        "Shading:"
    )

    print(
        "  flat   :",
        flat_count,
    )

    print(
        "  smooth :",
        smooth_count,
    )

    print()

    print(
        "Materials:"
    )

    for index, (
        name,
        _,
    ) in enumerate(
        MATERIALS
    ):
        print(
            f"  {name:20s}"
            f"{counts[index]}"
        )

    print()
    print(
        "OBJ    :",
        obj_path,
    )

    print(
        "MTL    :",
        mtl_path,
    )

    print(
        "Preview:",
        preview_path,
    )


if __name__ == "__main__":
    main()
