#!/usr/bin/env python3
import argparse
from collections import Counter
from pathlib import Path

import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection


PALETTE = np.array([
    [226, 241, 250],  # moon white
    [165, 205, 234],  # pale blue
    [105, 158, 205],  # medium blue
    [70, 112, 158],   # deep blue
    [111, 221, 235],  # cyan accent
    [188, 199, 212],  # silver
    [75, 84, 103],    # dark grip
], dtype=np.float64)

PALETTE_NAMES = [
    "moon_white",
    "pale_blue",
    "medium_blue",
    "deep_blue",
    "cyan_accent",
    "silver",
    "dark_grip",
]


def parse_args():
    parser = argparse.ArgumentParser(
        description=(
            "Concept-sheet -> low-poly Moonstone Scimitar reconstruction."
        )
    )
    parser.add_argument(
        "image",
        type=Path,
        help="Concept sheet PNG",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("dataset/ml/generated_moonstone_scimitar_v2"),
    )
    parser.add_argument(
        "--rings",
        type=int,
        default=36,
        help="Number of cross-sections from tip to pommel",
    )
    return parser.parse_args()


def make_mask(crop):
    crop = crop.astype(np.uint8)

    height, width, _ = crop.shape

    border = np.concatenate([
        crop[:15].reshape(-1, 3),
        crop[-15:].reshape(-1, 3),
        crop[:, :15].reshape(-1, 3),
        crop[:, -15:].reshape(-1, 3),
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

    # The concept uses a nearly uniform gray background.
    # Keep pixels sufficiently different from it.
    return difference > 35.0


def longest_vertical_region(mask):
    counts = mask.sum(axis=1)
    valid = np.where(counts >= 2)[0]

    if len(valid) == 0:
        raise RuntimeError("No sword pixels detected in crop.")

    groups = []
    start = previous = int(valid[0])

    for value in valid[1:]:
        value = int(value)

        if value - previous <= 3:
            previous = value
        else:
            groups.append((start, previous))
            start = previous = value

    groups.append((start, previous))

    return max(
        groups,
        key=lambda pair: pair[1] - pair[0],
    )


def sample_extent(mask, y0, y1, t):
    y = int(round(
        y0 + t * (y1 - y0)
    ))

    ya = max(y0, y - 2)
    yb = min(y1, y + 2)

    xs = np.where(
        mask[ya:yb + 1].any(axis=0)
    )[0]

    if len(xs) == 0:
        return 0, 0, 2, y

    xmin = int(xs.min())
    xmax = int(xs.max())

    return (
        xmin,
        xmax,
        xmax - xmin + 1,
        y,
    )


def sample_rgb(crop, mask, y0, y1, t, fraction):
    xmin, xmax, _, y = sample_extent(
        mask,
        y0,
        y1,
        t,
    )

    x = int(round(
        xmin + fraction * (xmax - xmin)
    ))

    pixels = []

    for yy in range(
        max(0, y - 2),
        min(crop.shape[0], y + 3),
    ):
        for xx in range(
            max(0, x - 2),
            min(crop.shape[1], x + 3),
        ):
            if mask[yy, xx]:
                pixels.append(
                    crop[yy, xx]
                )

    if not pixels:
        return PALETTE[0].astype(np.uint8)

    return np.median(
        np.asarray(pixels),
        axis=0,
    ).astype(np.uint8)


def nearest_palette(rgb):
    rgb = np.asarray(
        rgb,
        dtype=np.float64,
    )

    distances = np.sum(
        (PALETTE - rgb) ** 2,
        axis=1,
    )

    return int(np.argmin(distances))


def build_mesh(
    front_crop,
    front_mask,
    front_region,
    side_crop,
    side_mask,
    side_region,
    ring_count,
):
    side_y0, side_y1 = side_region
    front_y0, front_y1 = front_region

    sword_pixel_height = max(
        1,
        side_y1 - side_y0,
    )

    ts = np.linspace(
        0.0,
        1.0,
        ring_count,
    )

    # --------------------------------------------------------
    # Same Moonstone geometry that already worked.
    # --------------------------------------------------------

    handle_centers = []

    for handle_t in np.linspace(
        0.84,
        0.93,
        7,
    ):
        xmin, xmax, _, _ = sample_extent(
            side_mask,
            side_y0,
            side_y1,
            float(handle_t),
        )

        handle_centers.append(
            (xmin + xmax) * 0.5
        )

    neutral_center = float(
        np.median(handle_centers)
    )


    # --------------------------------------------------------
    # Sample raw geometry profile first.
    # --------------------------------------------------------

    widths = []
    depths = []
    centers = []

    for t in ts:
        (
            side_xmin,
            side_xmax,
            side_width,
            _,
        ) = sample_extent(
            side_mask,
            side_y0,
            side_y1,
            t,
        )

        (
            _,
            _,
            front_width,
            _,
        ) = sample_extent(
            front_mask,
            front_y0,
            front_y1,
            t,
        )

        side_center = (
            side_xmin + side_xmax
        ) * 0.5

        widths.append(
            max(side_width, 2)
            / sword_pixel_height
            * 2.0
        )

        depths.append(
            max(
                max(front_width, 2)
                / sword_pixel_height
                * 2.0,
                0.02,
            )
        )

        centers.append(
            (
                side_center
                - neutral_center
            )
            / sword_pixel_height
            * 2.0
        )


    widths = np.asarray(
        widths,
        dtype=np.float64,
    )

    depths = np.asarray(
        depths,
        dtype=np.float64,
    )

    centers = np.asarray(
        centers,
        dtype=np.float64,
    )


    # --------------------------------------------------------
    # LIGHT smoothing.
    #
    # This keeps the concept silhouette, but removes single-
    # row mask jitter.
    # --------------------------------------------------------

    def smooth(values):
        if len(values) < 5:
            return values

        result = values.copy()

        for i in range(
            2,
            len(values) - 2,
        ):
            result[i] = (
                values[i - 2] * 1.0
                + values[i - 1] * 2.0
                + values[i] * 3.0
                + values[i + 1] * 2.0
                + values[i + 2] * 1.0
            ) / 9.0

        return result


    widths = smooth(widths)
    depths = smooth(depths)
    centers = smooth(centers)


    # --------------------------------------------------------
    # Build SAME ring geometry.
    # --------------------------------------------------------

    vertices = []
    rings = []

    for i, t in enumerate(ts):
        width = widths[i]
        depth = depths[i]
        center_x = centers[i]

        # Force a genuinely pointed blade tip instead of
        # trusting a few blurry concept-sheet pixels.
        if t < 0.10:
            tip_factor = max(
                0.05,
                t / 0.10,
            )

            width *= tip_factor
            depth *= max(
                0.18,
                tip_factor,
            )

        y = 1.0 - 2.0 * t


        # --------------------------------------------
        # SHARP blade cross-section.
        #
        # Positive X is our cutting-edge side.
        #
        # 0 = spine/front
        # 1 = ridge/front
        # 2 = SHARED cutting edge, z = 0
        # 3 = ridge/back
        # 4 = spine/back
        #
        # Because vertex 2 is shared by both sides,
        # the blade has mathematically zero thickness
        # at the cutting edge.
        # --------------------------------------------

        spine_x = (
            center_x
            - width * 0.50
        )

        ridge_x = (
            center_x
            - width * 0.04
        )

        edge_x = (
            center_x
            + width * 0.50
        )


        spine_front = len(vertices)
        vertices.append((
            spine_x,
            y,
            depth * 0.32,
        ))

        ridge_front = len(vertices)
        vertices.append((
            ridge_x,
            y,
            depth * 0.50,
        ))

        cutting_edge = len(vertices)
        vertices.append((
            edge_x,
            y,
            0.0,
        ))

        ridge_back = len(vertices)
        vertices.append((
            ridge_x,
            y,
            -depth * 0.50,
        ))

        spine_back = len(vertices)
        vertices.append((
            spine_x,
            y,
            -depth * 0.32,
        ))


        rings.append((
            spine_front,
            ridge_front,
            cutting_edge,
            ridge_back,
            spine_back,
        ))


    # --------------------------------------------------------
    # MATERIAL SYSTEM
    #
    # No pixel-by-pixel triangle roulette anymore.
    #
    # Front/back strip meaning:
    #
    #   0 = inner/spine facet
    #   1 = central moonstone facet
    #   2 = cutting-edge facet
    #
    # Along length:
    #
    #   < .72 = blade
    #   .72-.82 = guard
    #   .82-.95 = grip
    #   > .95 = pommel
    # --------------------------------------------------------

    MAT_WHITE = 0
    MAT_PALE = 1
    MAT_MID = 2
    MAT_DEEP = 3
    MAT_CYAN = 4
    MAT_SILVER = 5
    MAT_GRIP = 6


    def surface_material(
        t,
        strip,
        back=False,
    ):
        # Blade
        if t < 0.72:
            if strip == 0:
                return (
                    MAT_DEEP
                    if not back
                    else MAT_MID
                )

            if strip == 1:
                # Only a few intentional cyan zones.
                if (
                    0.18 < t < 0.30
                    or
                    0.48 < t < 0.60
                ):
                    return MAT_CYAN

                return MAT_PALE

            # Cutting edge
            return MAT_WHITE


        # Guard / transition
        if t < 0.82:
            if strip == 1:
                return MAT_CYAN

            return MAT_SILVER


        # Grip
        if t < 0.95:
            return MAT_GRIP


        # Pommel
        if strip == 1:
            return MAT_CYAN

        return MAT_SILVER


    faces = []
    materials = []


    for i in range(
        ring_count - 1
    ):
        t_mid = (
            ts[i] + ts[i + 1]
        ) * 0.5

        a = rings[i]
        b = rings[i + 1]

        (
            a_spine_front,
            a_ridge_front,
            a_edge,
            a_ridge_back,
            a_spine_back,
        ) = a

        (
            b_spine_front,
            b_ridge_front,
            b_edge,
            b_ridge_back,
            b_spine_back,
        ) = b


        # ============================================
        # FRONT FACE
        # ============================================

        # Dark inner/spine facet.
        faces.extend([
            (
                a_spine_front,
                a_ridge_front,
                b_ridge_front,
            ),
            (
                a_spine_front,
                b_ridge_front,
                b_spine_front,
            ),
        ])

        if t_mid < 0.72:
            front_inner_material = MAT_DEEP
        elif t_mid < 0.82:
            front_inner_material = MAT_SILVER
        elif t_mid < 0.95:
            front_inner_material = MAT_GRIP
        else:
            front_inner_material = MAT_SILVER

        materials.extend([
            front_inner_material,
            front_inner_material,
        ])


        # Main bevel running directly into the sharp edge.
        faces.extend([
            (
                a_ridge_front,
                a_edge,
                b_edge,
            ),
            (
                a_ridge_front,
                b_edge,
                b_ridge_front,
            ),
        ])

        if t_mid < 0.72:
            # Large coherent moonstone facets.
            if (
                0.22 < t_mid < 0.34
                or
                0.50 < t_mid < 0.62
            ):
                front_edge_material = MAT_CYAN
            else:
                front_edge_material = MAT_PALE

        elif t_mid < 0.82:
            front_edge_material = MAT_CYAN

        elif t_mid < 0.95:
            front_edge_material = MAT_GRIP

        else:
            front_edge_material = MAT_SILVER

        materials.extend([
            front_edge_material,
            front_edge_material,
        ])


        # ============================================
        # BACK FACE
        # ============================================

        faces.extend([
            (
                a_edge,
                a_ridge_back,
                b_ridge_back,
            ),
            (
                a_edge,
                b_ridge_back,
                b_edge,
            ),
        ])

        if t_mid < 0.72:
            back_edge_material = MAT_MID
        elif t_mid < 0.82:
            back_edge_material = MAT_SILVER
        elif t_mid < 0.95:
            back_edge_material = MAT_GRIP
        else:
            back_edge_material = MAT_SILVER

        materials.extend([
            back_edge_material,
            back_edge_material,
        ])


        faces.extend([
            (
                a_ridge_back,
                a_spine_back,
                b_spine_back,
            ),
            (
                a_ridge_back,
                b_spine_back,
                b_ridge_back,
            ),
        ])

        if t_mid < 0.72:
            back_inner_material = MAT_DEEP
        elif t_mid < 0.82:
            back_inner_material = MAT_SILVER
        elif t_mid < 0.95:
            back_inner_material = MAT_GRIP
        else:
            back_inner_material = MAT_SILVER

        materials.extend([
            back_inner_material,
            back_inner_material,
        ])


        # ============================================
        # SPINE
        #
        # The blunt BACK of the sword still has
        # thickness. The cutting edge does not.
        # ============================================

        faces.extend([
            (
                a_spine_back,
                a_spine_front,
                b_spine_front,
            ),
            (
                a_spine_back,
                b_spine_front,
                b_spine_back,
            ),
        ])

        if t_mid < 0.72:
            spine_material = MAT_DEEP
        elif t_mid < 0.82:
            spine_material = MAT_SILVER
        elif t_mid < 0.95:
            spine_material = MAT_GRIP
        else:
            spine_material = MAT_SILVER

        materials.extend([
            spine_material,
            spine_material,
        ])


    # ========================================================
    # TIP CAP
    #
    # Rings now contain 5 vertices:
    #
    #   0 spine_front
    #   1 ridge_front
    #   2 cutting_edge
    #   3 ridge_back
    #   4 spine_back
    #
    # So the old 8-vertex cap is invalid.
    # ========================================================

    tip_ring = rings[0]

    tip_center = len(vertices)

    vertices.append((
        sum(vertices[i][0] for i in tip_ring) / 5.0,
        sum(vertices[i][1] for i in tip_ring) / 5.0,
        sum(vertices[i][2] for i in tip_ring) / 5.0,
    ))

    tip_loop = [
        tip_ring[0],
        tip_ring[1],
        tip_ring[2],
        tip_ring[3],
        tip_ring[4],
    ]

    for i in range(len(tip_loop)):
        a = tip_loop[i]
        b = tip_loop[(i + 1) % len(tip_loop)]

        faces.append((
            tip_center,
            a,
            b,
        ))

        materials.append(
            MAT_WHITE
        )


    # ========================================================
    # GRIP END + MOONSTONE POMMEL
    #
    # Build the pommel as a separate low-poly crystal instead
    # of pretending the last blade ring is the pommel.
    # ========================================================

    end_ring = rings[-1]

    end_center_x = (
        sum(vertices[i][0] for i in end_ring)
        / len(end_ring)
    )

    end_center_y = (
        sum(vertices[i][1] for i in end_ring)
        / len(end_ring)
    )

    end_center_z = (
        sum(vertices[i][2] for i in end_ring)
        / len(end_ring)
    )


    # --------------------------------------------------------
    # Close the end of the grip.
    # --------------------------------------------------------

    grip_cap = len(vertices)

    vertices.append((
        end_center_x,
        end_center_y,
        end_center_z,
    ))

    end_loop = [
        end_ring[0],
        end_ring[1],
        end_ring[2],
        end_ring[3],
        end_ring[4],
    ]

    for i in range(len(end_loop)):
        a = end_loop[i]
        b = end_loop[(i + 1) % len(end_loop)]

        faces.append((
            a,
            grip_cap,
            b,
        ))

        materials.append(
            MAT_GRIP
        )


    # --------------------------------------------------------
    # Small silver neck.
    # --------------------------------------------------------

    neck_y = (
        end_center_y
        - 0.055
    )

    neck_half_x = 0.045
    neck_half_z = 0.035

    neck_front = len(vertices)
    vertices.append((
        end_center_x,
        neck_y,
        end_center_z + neck_half_z,
    ))

    neck_right = len(vertices)
    vertices.append((
        end_center_x + neck_half_x,
        neck_y,
        end_center_z,
    ))

    neck_back = len(vertices)
    vertices.append((
        end_center_x,
        neck_y,
        end_center_z - neck_half_z,
    ))

    neck_left = len(vertices)
    vertices.append((
        end_center_x - neck_half_x,
        neck_y,
        end_center_z,
    ))


    # Connect grip center to neck ring.
    for a, b in (
        (neck_front, neck_right),
        (neck_right, neck_back),
        (neck_back, neck_left),
        (neck_left, neck_front),
    ):
        faces.append((
            grip_cap,
            a,
            b,
        ))

        materials.append(
            MAT_SILVER
        )


    # --------------------------------------------------------
    # Faceted Moonstone pommel.
    #
    # Octahedron-ish crystal:
    #
    #          top
    #           *
    #        / / \ \
    #       L F   R B
    #        \ \ / /
    #         bottom
    # --------------------------------------------------------

    pommel_top = len(vertices)

    vertices.append((
        end_center_x,
        end_center_y - 0.080,
        end_center_z,
    ))

    pommel_y = (
        end_center_y
        - 0.145
    )

    pommel_half_x = 0.095
    pommel_half_z = 0.070

    pommel_left = len(vertices)

    vertices.append((
        end_center_x - pommel_half_x,
        pommel_y,
        end_center_z,
    ))

    pommel_front = len(vertices)

    vertices.append((
        end_center_x,
        pommel_y,
        end_center_z + pommel_half_z,
    ))

    pommel_right = len(vertices)

    vertices.append((
        end_center_x + pommel_half_x,
        pommel_y,
        end_center_z,
    ))

    pommel_back = len(vertices)

    vertices.append((
        end_center_x,
        pommel_y,
        end_center_z - pommel_half_z,
    ))

    pommel_bottom = len(vertices)

    vertices.append((
        end_center_x,
        end_center_y - 0.245,
        end_center_z,
    ))


    # Upper crystal facets.
    pommel_upper = [
        (pommel_top, pommel_left,  pommel_front, MAT_CYAN),
        (pommel_top, pommel_front, pommel_right, MAT_PALE),
        (pommel_top, pommel_right, pommel_back,  MAT_CYAN),
        (pommel_top, pommel_back,  pommel_left,  MAT_MID),
    ]

    for a, b, c, mat in pommel_upper:
        faces.append((a, b, c))
        materials.append(mat)


    # Lower crystal facets.
    pommel_lower = [
        (pommel_left,  pommel_bottom, pommel_front, MAT_PALE),
        (pommel_front, pommel_bottom, pommel_right, MAT_WHITE),
        (pommel_right, pommel_bottom, pommel_back,  MAT_CYAN),
        (pommel_back,  pommel_bottom, pommel_left,  MAT_MID),
    ]

    for a, b, c, mat in pommel_lower:
        faces.append((a, b, c))
        materials.append(mat)


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
            materials,
            dtype=np.int32,
        ),
    )


def write_mtl(path):
    with path.open(
        "w",
        encoding="utf-8",
    ) as file:
        for index, name in enumerate(
            PALETTE_NAMES
        ):
            r, g, b = (
                PALETTE[index]
                / 255.0
            )

            file.write(
                f"newmtl {name}\n"
            )
            file.write(
                f"Kd {r:.6f} {g:.6f} {b:.6f}\n"
            )
            file.write(
                "Ka 0.000000 0.000000 0.000000\n"
            )
            file.write(
                "Ks 0.000000 0.000000 0.000000\n"
            )
            file.write(
                "illum 1\n\n"
            )


def write_obj(
    path,
    mtl_name,
    vertices,
    faces,
    materials,
):
    with path.open(
        "w",
        encoding="utf-8",
    ) as file:
        file.write(
            f"mtllib {mtl_name}\n"
        )
        file.write(
            "o Crystal_Enflamed_Longsword\n"
        )

        for x, y, z in vertices:
            file.write(
                f"v {x:.6f} {y:.6f} {z:.6f}\n"
            )

        current_material = None

        for face, material in zip(
            faces,
            materials,
        ):
            material = int(material)

            if material != current_material:
                file.write(
                    f"usemtl {PALETTE_NAMES[material]}\n"
                )
                current_material = material

            # OBJ is 1-indexed.
            a, b, c = (
                int(face[0]) + 1,
                int(face[1]) + 1,
                int(face[2]) + 1,
            )

            file.write(
                f"f {a} {b} {c}\n"
            )


def save_preview(
    path,
    vertices,
    faces,
    materials,
):
    figure = plt.figure(
        figsize=(7, 9)
    )

    ax = figure.add_subplot(
        111,
        projection="3d",
    )

    triangles = vertices[
        faces
    ]

    mesh = Poly3DCollection(
        triangles,
        linewidths=0.18,
    )

    mesh.set_facecolor(
        PALETTE[materials]
        / 255.0
    )

    mesh.set_edgecolor(
        (0, 0, 0, 0.28)
    )

    ax.add_collection3d(
        mesh
    )

    x_extent = max(
        0.30,
        float(
            np.abs(
                vertices[:, 0]
            ).max()
        ) * 1.30,
    )

    z_extent = max(
        0.15,
        float(
            np.abs(
                vertices[:, 2]
            ).max()
        ) * 1.30,
    )

    ax.set_xlim(
        -x_extent,
        x_extent,
    )
    ax.set_ylim(
        -1.08,
        1.08,
    )
    ax.set_zlim(
        -z_extent,
        z_extent,
    )

    ax.set_box_aspect(
        (
            x_extent * 2,
            2.16,
            z_extent * 2,
        )
    )

    ax.view_init(
        elev=12,
        azim=-55,
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


def main():
    args = parse_args()

    args.output_dir.mkdir(
        parents=True,
        exist_ok=True,
    )

    image = Image.open(
        args.image
    ).convert(
        "RGB"
    )

    pixels = np.asarray(
        image
    )

    height, width, _ = pixels.shape

    # Moonstone Scimitar concept sheet.
    #
    # SIDE VIEW:
    #     curved broad silhouette in the middle.
    #
    # FRONT VIEW:
    #     narrow straight view immediately right.
    #
    # Fractions keep this independent of resolution.
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

    sx0, sy0, sx1, sy1 = side_box
    fx0, fy0, fx1, fy1 = front_box

    side_crop = pixels[
        sy0:sy1,
        sx0:sx1,
    ]

    front_crop = pixels[
        fy0:fy1,
        fx0:fx1,
    ]

    side_mask = make_mask(
        side_crop
    )

    front_mask = make_mask(
        front_crop
    )

    # Save exactly what the generator is seeing.
    Image.fromarray(
        side_crop.astype(np.uint8)
    ).save(
        args.output_dir
        / "moonstone_side_crop.png"
    )

    Image.fromarray(
        front_crop.astype(np.uint8)
    ).save(
        args.output_dir
        / "moonstone_front_crop.png"
    )

    Image.fromarray(
        (
            side_mask.astype(np.uint8)
            * 255
        )
    ).save(
        args.output_dir
        / "moonstone_side_mask.png"
    )

    Image.fromarray(
        (
            front_mask.astype(np.uint8)
            * 255
        )
    ).save(
        args.output_dir
        / "moonstone_front_mask.png"
    )

    side_region = longest_vertical_region(
        side_mask
    )

    front_region = longest_vertical_region(
        front_mask
    )

    vertices, faces, materials = build_mesh(
        front_crop,
        front_mask,
        front_region,
        side_crop,
        side_mask,
        side_region,
        args.rings,
    )

    obj_path = (
        args.output_dir
        / "moonstone_scimitar_v2.obj"
    )

    mtl_path = (
        args.output_dir
        / "moonstone_scimitar_v2.mtl"
    )

    preview_path = (
        args.output_dir
        / "moonstone_scimitar_v2_preview.png"
    )

    write_mtl(
        mtl_path
    )

    write_obj(
        obj_path,
        mtl_path.name,
        vertices,
        faces,
        materials,
    )

    save_preview(
        preview_path,
        vertices,
        faces,
        materials,
    )

    print(
        f"Vertices: {len(vertices)}"
    )
    print(
        f"Triangles: {len(faces)}"
    )

    counts = Counter(
        int(value)
        for value in materials
    )

    print(
        "Materials:"
    )

    for index, count in sorted(
        counts.items()
    ):
        print(
            f"  {PALETTE_NAMES[index]:12s} {count}"
        )

    print()
    print(
        "OBJ:",
        obj_path,
    )
    print(
        "MTL:",
        mtl_path,
    )
    print(
        "Preview:",
        preview_path,
    )


if __name__ == "__main__":
    main()
