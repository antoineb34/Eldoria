#!/usr/bin/env python3
import argparse
from collections import Counter
from pathlib import Path

import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection


PALETTE = np.array([
    [58, 166, 161],   # teal
    [54, 125, 120],   # dark teal
    [89, 205, 201],   # light cyan
    [86, 86, 86],     # dark gray
    [102, 87, 70],    # brown
    [255, 111, 44],   # orange
    [187, 63, 49],    # red
], dtype=np.float64)

PALETTE_NAMES = [
    "teal",
    "dark_teal",
    "light_cyan",
    "dark_gray",
    "brown",
    "orange",
    "red",
]


def parse_args():
    parser = argparse.ArgumentParser(
        description=(
            "Prototype concept-sheet -> low-poly sword reconstruction. "
            "Designed for the Crystal Enflamed Longsword sheet."
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
        default=Path("dataset/ml/generated_sword"),
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

    mx = crop.max(axis=2).astype(np.float64)
    mn = crop.min(axis=2).astype(np.float64)
    saturation = (mx - mn) / (mx + 1e-6)

    brightness = crop.mean(axis=2)

    # Colored crystal/flame pixels OR sufficiently dark hilt/guard pixels.
    return (saturation > 0.12) | (brightness < 145.0)


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
    front_y0, front_y1 = front_region
    side_y0, side_y1 = side_region

    sword_pixel_height = max(
        1,
        front_y1 - front_y0,
    )

    ts = np.linspace(
        0.0,
        1.0,
        ring_count,
    )

    vertices = []
    rings = []

    # Four X positions on front and four matching positions on back.
    # This gives us 3 front/back strips:
    # left crystal / flame-core region / right crystal.
    x_fractions = [
        -0.50,
        -0.17,
         0.17,
         0.50,
    ]

    for t in ts:
        _, _, front_width, _ = sample_extent(
            front_mask,
            front_y0,
            front_y1,
            t,
        )

        _, _, side_width, _ = sample_extent(
            side_mask,
            side_y0,
            side_y1,
            t,
        )

        # Preserve image proportions relative to a total sword height of 2.
        width = (
            max(front_width, 2)
            / sword_pixel_height
            * 2.0
        )

        depth = (
            max(side_width, 2)
            / sword_pixel_height
            * 2.0
        )

        # Avoid numerically paper-thin geometry.
        depth = max(
            depth,
            0.02,
        )

        y = 1.0 - 2.0 * t

        ring = []

        for z_sign in (1.0, -1.0):
            z = z_sign * depth * 0.5

            for x_fraction in x_fractions:
                ring.append(
                    len(vertices)
                )

                vertices.append(
                    (
                        x_fraction * width,
                        y,
                        z,
                    )
                )

        rings.append(ring)

    faces = []
    materials = []

    strip_centers = [
        0.165,
        0.500,
        0.835,
    ]

    for i in range(ring_count - 1):
        t_mid = (
            ts[i] + ts[i + 1]
        ) * 0.5

        ring_a = rings[i]
        ring_b = rings[i + 1]

        # Front and back surfaces.
        for offset in (0, 4):
            for strip_index, sample_fraction in enumerate(
                strip_centers
            ):
                a = ring_a[
                    offset + strip_index
                ]
                b = ring_a[
                    offset + strip_index + 1
                ]
                c = ring_b[
                    offset + strip_index + 1
                ]
                d = ring_b[
                    offset + strip_index
                ]

                faces.extend([
                    (a, b, c),
                    (a, c, d),
                ])

                rgb = sample_rgb(
                    front_crop,
                    front_mask,
                    front_y0,
                    front_y1,
                    t_mid,
                    sample_fraction,
                )

                material = nearest_palette(
                    rgb
                )

                materials.extend([
                    material,
                    material,
                ])

        # Left/right thickness walls.
        wall_pairs = [
            (0, 4, 0.02),
            (3, 7, 0.98),
        ]

        for front_index, back_index, sample_fraction in wall_pairs:
            a = ring_a[
                front_index
            ]
            b = ring_a[
                back_index
            ]
            c = ring_b[
                back_index
            ]
            d = ring_b[
                front_index
            ]

            faces.extend([
                (a, b, c),
                (a, c, d),
            ])

            rgb = sample_rgb(
                front_crop,
                front_mask,
                front_y0,
                front_y1,
                t_mid,
                sample_fraction,
            )

            material = nearest_palette(
                rgb
            )

            materials.extend([
                material,
                material,
            ])

    # Simple caps.
    for ring in (rings[0], rings[-1]):
        faces.extend([
            (ring[0], ring[1], ring[5]),
            (ring[0], ring[5], ring[4]),
            (ring[2], ring[3], ring[7]),
            (ring[2], ring[7], ring[6]),
        ])

        materials.extend([
            0, 0, 0, 0
        ])

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

    # Tuned for the generated 4:3 concept sheet:
    # Side view is the thin center sword.
    # Front view is the sword immediately right of it.
    #
    # Fractions make it resolution-independent.
    side_box = (
        int(width * 0.365),
        int(height * 0.083),
        int(width * 0.445),
        int(height * 0.820),
    )

    front_box = (
        int(width * 0.515),
        int(height * 0.083),
        int(width * 0.650),
        int(height * 0.820),
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
        / "crystal_enflamed_longsword.obj"
    )

    mtl_path = (
        args.output_dir
        / "crystal_enflamed_longsword.mtl"
    )

    preview_path = (
        args.output_dir
        / "crystal_enflamed_longsword_preview.png"
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
