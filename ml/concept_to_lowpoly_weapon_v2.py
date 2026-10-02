#!/usr/bin/env python3

import argparse
from collections import Counter
from pathlib import Path

import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection


# ------------------------------------------------------------
# Utility
# ------------------------------------------------------------

def parse_args():
    parser = argparse.ArgumentParser(
        description=(
            "Concept sheet -> cleaner low-poly weapon generator v2 "
            "(smoothed silhouette + intentional material regions)."
        )
    )

    parser.add_argument(
        "image",
        type=Path,
        help="Concept sheet image",
    )

    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("dataset/ml/generated_weapon_v2"),
    )

    parser.add_argument(
        "--name",
        type=str,
        default="generated_weapon_v2",
        help="Base output filename (without extension)",
    )

    parser.add_argument(
        "--rings",
        type=int,
        default=18,
        help="Cross-sections from tip to pommel",
    )

    parser.add_argument(
        "--smooth-window",
        type=int,
        default=5,
        help="Moving-average smoothing window",
    )

    return parser.parse_args()


def smooth1d(values, window):
    values = np.asarray(values, dtype=np.float64)

    if window <= 1 or len(values) < 3:
        return values.copy()

    if window % 2 == 0:
        window += 1

    pad = window // 2

    padded = np.pad(
        values,
        (pad, pad),
        mode="edge",
    )

    kernel = np.ones(window, dtype=np.float64) / window

    return np.convolve(
        padded,
        kernel,
        mode="valid",
    )


def luminance(rgb):
    rgb = np.asarray(rgb, dtype=np.float64)
    return (
        0.2126 * rgb[0]
        + 0.7152 * rgb[1]
        + 0.0722 * rgb[2]
    )


def saturation_score(rgb):
    rgb = np.asarray(rgb, dtype=np.float64)
    return float(rgb.max() - rgb.min())


def color_distance(a, b):
    a = np.asarray(a, dtype=np.float64)
    b = np.asarray(b, dtype=np.float64)
    return float(np.linalg.norm(a - b))


def clamp_rgb(rgb):
    rgb = np.asarray(rgb, dtype=np.float64)
    rgb = np.clip(rgb, 0.0, 255.0)
    return rgb


# ------------------------------------------------------------
# Concept-sheet parsing
# ------------------------------------------------------------

def estimate_background_rgb(crop):
    h, w, _ = crop.shape

    s = max(8, min(h, w) // 10)

    corners = np.concatenate(
        [
            crop[:s, :s].reshape(-1, 3),
            crop[:s, -s:].reshape(-1, 3),
            crop[-s:, :s].reshape(-1, 3),
            crop[-s:, -s:].reshape(-1, 3),
        ],
        axis=0,
    )

    return np.median(
        corners.astype(np.float64),
        axis=0,
    )


def make_mask(crop):
    crop_f = crop.astype(np.float64)

    bg = estimate_background_rgb(crop)

    distance = np.linalg.norm(
        crop_f - bg[None, None, :],
        axis=2,
    )

    mx = crop_f.max(axis=2)
    mn = crop_f.min(axis=2)

    saturation = (mx - mn) / (mx + 1e-6)
    brightness = crop_f.mean(axis=2)

    bg_brightness = float(bg.mean())

    mask = (
        (distance > 22.0)
        | (saturation > 0.10)
        | (brightness < bg_brightness - 12.0)
    )

    return mask


def longest_vertical_region(mask):
    counts = mask.sum(axis=1)
    valid = np.where(counts >= 2)[0]

    if len(valid) == 0:
        raise RuntimeError("No weapon pixels found in crop.")

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
        return 0, 0, 2, 1, y

    xmin = int(xs.min())
    xmax = int(xs.max())
    width = xmax - xmin + 1
    center = 0.5 * (xmin + xmax)

    return xmin, xmax, width, center, y


def sample_profile(mask, region, ts):
    y0, y1 = region

    widths = []
    centers = []

    for t in ts:
        _, _, width, center, _ = sample_extent(
            mask,
            y0,
            y1,
            t,
        )

        widths.append(width)
        centers.append(center)

    return (
        np.asarray(widths, dtype=np.float64),
        np.asarray(centers, dtype=np.float64),
    )


# ------------------------------------------------------------
# Palette extraction
# ------------------------------------------------------------

DEFAULT_BLADE_COLORS = [
    np.array([88, 110, 145], dtype=np.float64),   # dark blue
    np.array([115, 150, 185], dtype=np.float64),  # mid blue
    np.array([168, 205, 234], dtype=np.float64),  # light blue
    np.array([115, 220, 240], dtype=np.float64),  # cyan accent
    np.array([230, 242, 252], dtype=np.float64),  # bright edge
]

DEFAULT_HANDLE_COLORS = [
    np.array([55, 62, 78], dtype=np.float64),     # grip dark
    np.array([85, 95, 115], dtype=np.float64),    # grip light
    np.array([138, 148, 162], dtype=np.float64),  # metal dark
    np.array([188, 198, 215], dtype=np.float64),  # metal light
]


def choose_distinct_colors(unique_colors, counts, wanted, min_distance=28.0):
    order = np.argsort(counts)[::-1]

    chosen = []

    for index in order:
        rgb = unique_colors[index].astype(np.float64)

        if not chosen:
            chosen.append(rgb)
        else:
            if all(
                color_distance(rgb, other) >= min_distance
                for other in chosen
            ):
                chosen.append(rgb)

        if len(chosen) >= wanted:
            break

    return chosen


def extract_region_colors(crop, mask, region, t0, t1, wanted, fallback):
    y0, y1 = region

    ya = int(round(y0 + t0 * (y1 - y0)))
    yb = int(round(y0 + t1 * (y1 - y0)))

    ya = max(0, min(crop.shape[0] - 1, ya))
    yb = max(0, min(crop.shape[0] - 1, yb))

    if yb < ya:
        ya, yb = yb, ya

    region_mask = mask[ya:yb + 1]
    region_pixels = crop[ya:yb + 1][region_mask]

    if len(region_pixels) < 20:
        return [c.copy() for c in fallback[:wanted]]

    # Quantize so tiny pixel noise doesn't dominate.
    quantized = (
        (region_pixels.astype(np.int32) // 16) * 16 + 8
    )

    unique_colors, counts = np.unique(
        quantized,
        axis=0,
        return_counts=True,
    )

    selected = choose_distinct_colors(
        unique_colors=unique_colors,
        counts=counts,
        wanted=wanted,
        min_distance=26.0,
    )

    # Fill from fallback if needed.
    for fallback_color in fallback:
        if len(selected) >= wanted:
            break

        if all(
            color_distance(fallback_color, other) >= 18.0
            for other in selected
        ):
            selected.append(
                fallback_color.astype(np.float64)
            )

    while len(selected) < wanted:
        selected.append(
            fallback[min(len(selected), len(fallback) - 1)].astype(np.float64)
        )

    return selected[:wanted]


def build_material_palette(front_crop, front_mask, front_region):
    blade_candidates = extract_region_colors(
        crop=front_crop,
        mask=front_mask,
        region=front_region,
        t0=0.00,
        t1=0.72,
        wanted=5,
        fallback=DEFAULT_BLADE_COLORS,
    )

    handle_candidates = extract_region_colors(
        crop=front_crop,
        mask=front_mask,
        region=front_region,
        t0=0.72,
        t1=1.00,
        wanted=4,
        fallback=DEFAULT_HANDLE_COLORS,
    )

    # Sort blade colors from dark -> bright.
    blade_candidates = sorted(
        blade_candidates,
        key=luminance,
    )

    blade_dark = blade_candidates[0]
    blade_mid = blade_candidates[1]
    blade_light = blade_candidates[2]
    bright_candidate = blade_candidates[-1]

    accent_cyan = max(
        blade_candidates,
        key=saturation_score,
    )

    # Force a nice bright edge by blending toward white.
    edge_highlight = clamp_rgb(
        bright_candidate * 0.72
        + np.array([255, 255, 255], dtype=np.float64) * 0.28
    )

    # Sort handle colors dark -> bright.
    handle_candidates = sorted(
        handle_candidates,
        key=luminance,
    )

    grip_dark = handle_candidates[0]
    grip_light = handle_candidates[1]
    metal_dark = clamp_rgb(
        handle_candidates[1] * 0.55
        + handle_candidates[2] * 0.45
    )
    metal_light = clamp_rgb(
        handle_candidates[2] * 0.50
        + handle_candidates[3] * 0.50
    )

    materials = [
        ("blade_dark", blade_dark),
        ("blade_mid", blade_mid),
        ("blade_light", blade_light),
        ("accent_cyan", accent_cyan),
        ("edge_highlight", edge_highlight),
        ("grip_dark", grip_dark),
        ("grip_light", grip_light),
        ("metal_dark", metal_dark),
        ("metal_light", metal_light),
    ]

    return materials


# ------------------------------------------------------------
# Mesh generation
# ------------------------------------------------------------

def build_mesh(
    front_crop,
    front_mask,
    front_region,
    side_crop,
    side_mask,
    side_region,
    materials,
    ring_count,
    smooth_window,
):
    material_names = [name for name, _ in materials]
    material_lookup = {
        name: index
        for index, name in enumerate(material_names)
    }

    ts = np.linspace(
        0.0,
        1.0,
        ring_count,
    )

    front_widths, front_centers = sample_profile(
        front_mask,
        front_region,
        ts,
    )

    side_widths, _ = sample_profile(
        side_mask,
        side_region,
        ts,
    )

    front_widths = smooth1d(
        front_widths,
        smooth_window,
    )

    side_widths = smooth1d(
        side_widths,
        smooth_window,
    )

    front_centers = smooth1d(
        front_centers,
        smooth_window,
    )

    front_y0, front_y1 = front_region

    sword_pixel_height = max(
        1.0,
        float(front_y1 - front_y0),
    )

    global_center = float(
        np.median(front_centers)
    )

    # 4 samples across the front face = 3 readable strips.
    x_fractions = [
        -0.52,
        -0.16,
         0.16,
         0.52,
    ]

    vertices = []
    rings = []

    for i, t in enumerate(ts):
        width = (
            max(front_widths[i], 2.0)
            / sword_pixel_height
            * 2.0
        )

        # Slightly flatter than old version:
        depth = (
            max(side_widths[i], 2.0)
            / sword_pixel_height
            * 2.0
            * 0.55
        )

        depth = max(depth, 0.028)

        center_offset = (
            (front_centers[i] - global_center)
            / sword_pixel_height
            * 2.0
        )

        # Light extra smoothing/taper near ends.
        end_taper = 1.0

        if t < 0.06:
            end_taper = 0.55 + 7.5 * t
        elif t > 0.94:
            end_taper = 0.55 + 7.5 * (1.0 - t)

        width *= min(end_taper, 1.0)
        depth *= min(end_taper, 1.0)

        y = 1.0 - 2.0 * t

        ring = []

        for z_sign in (1.0, -1.0):
            z = z_sign * depth * 0.5

            for x_fraction in x_fractions:
                ring.append(len(vertices))

                vertices.append(
                    (
                        center_offset + x_fraction * width,
                        y,
                        z,
                    )
                )

        rings.append(ring)

    faces = []
    face_materials = []

    def add_face(a, b, c, material_name):
        faces.append((a, b, c))
        face_materials.append(
            material_lookup[material_name]
        )

    def add_quad(a, b, c, d, material_name):
        add_face(a, b, c, material_name)
        add_face(a, c, d, material_name)

    def blade_material_for_strip(strip_index, t):
        # strip 0 = inner darker facet
        # strip 1 = main/accent facet
        # strip 2 = cutting edge side
        if strip_index == 0:
            if t < 0.18:
                return "blade_mid"
            return "blade_dark"

        if strip_index == 1:
            if 0.18 <= t <= 0.68:
                return "accent_cyan"
            if t < 0.10:
                return "blade_light"
            return "blade_light"

        return "edge_highlight"

    def region_material(strip_index, t, side_wall=False, right_wall=False):
        # Blade
        if t < 0.74:
            if side_wall:
                if right_wall:
                    return "edge_highlight"
                return "blade_mid"

            return blade_material_for_strip(strip_index, t)

        # Guard / transition
        if t < 0.83:
            if side_wall:
                return "metal_dark"

            if strip_index == 1:
                return "metal_light"

            if strip_index == 2:
                return "blade_light"

            return "metal_dark"

        # Grip
        if t < 0.95:
            if side_wall:
                return "grip_dark"

            return "grip_light" if strip_index == 1 else "grip_dark"

        # Pommel
        if side_wall:
            return "metal_dark"

        if strip_index == 1:
            return "metal_light"

        return "metal_dark"

    for i in range(ring_count - 1):
        t_mid = 0.5 * (ts[i] + ts[i + 1])

        ring_a = rings[i]
        ring_b = rings[i + 1]

        # Front and back surfaces.
        for offset in (0, 4):
            for strip_index in range(3):
                a = ring_a[offset + strip_index]
                b = ring_a[offset + strip_index + 1]
                c = ring_b[offset + strip_index + 1]
                d = ring_b[offset + strip_index]

                material_name = region_material(
                    strip_index=strip_index,
                    t=t_mid,
                    side_wall=False,
                )

                add_quad(a, b, c, d, material_name)

        # Left wall.
        add_quad(
            ring_a[0],
            ring_a[4],
            ring_b[4],
            ring_b[0],
            region_material(
                strip_index=0,
                t=t_mid,
                side_wall=True,
                right_wall=False,
            ),
        )

        # Right wall / cutting edge side.
        add_quad(
            ring_a[3],
            ring_a[7],
            ring_b[7],
            ring_b[3],
            region_material(
                strip_index=2,
                t=t_mid,
                side_wall=True,
                right_wall=True,
            ),
        )

    # Tip cap.
    tip = rings[0]
    add_face(tip[0], tip[1], tip[5], "edge_highlight")
    add_face(tip[0], tip[5], tip[4], "edge_highlight")
    add_face(tip[2], tip[3], tip[7], "edge_highlight")
    add_face(tip[2], tip[7], tip[6], "edge_highlight")

    # Pommel cap.
    end = rings[-1]
    add_face(end[0], end[1], end[5], "metal_light")
    add_face(end[0], end[5], end[4], "metal_dark")
    add_face(end[2], end[3], end[7], "metal_light")
    add_face(end[2], end[7], end[6], "metal_dark")

    return (
        np.asarray(vertices, dtype=np.float64),
        np.asarray(faces, dtype=np.int32),
        np.asarray(face_materials, dtype=np.int32),
    )


# ------------------------------------------------------------
# Output
# ------------------------------------------------------------

def write_mtl(path, materials):
    with path.open("w", encoding="utf-8") as file:
        for name, rgb in materials:
            r, g, b = (
                np.asarray(rgb, dtype=np.float64) / 255.0
            )

            file.write(f"newmtl {name}\n")
            file.write(f"Kd {r:.6f} {g:.6f} {b:.6f}\n")
            file.write("Ka 0.000000 0.000000 0.000000\n")
            file.write("Ks 0.000000 0.000000 0.000000\n")
            file.write("illum 1\n\n")


def write_obj(path, mtl_name, object_name, vertices, faces, face_materials, materials):
    material_names = [name for name, _ in materials]

    with path.open("w", encoding="utf-8") as file:
        file.write(f"mtllib {mtl_name}\n")
        file.write(f"o {object_name}\n")

        for x, y, z in vertices:
            file.write(
                f"v {x:.6f} {y:.6f} {z:.6f}\n"
            )

        current_material = None

        for face, material_index in zip(faces, face_materials):
            material_name = material_names[int(material_index)]

            if material_name != current_material:
                file.write(f"usemtl {material_name}\n")
                current_material = material_name

            a, b, c = (
                int(face[0]) + 1,
                int(face[1]) + 1,
                int(face[2]) + 1,
            )

            file.write(f"f {a} {b} {c}\n")


def save_preview(path, vertices, faces, face_materials, materials):
    colors = np.asarray(
        [rgb for _, rgb in materials],
        dtype=np.float64,
    )

    figure = plt.figure(figsize=(7, 9))
    ax = figure.add_subplot(111, projection="3d")

    triangles = vertices[faces]

    mesh = Poly3DCollection(
        triangles,
        linewidths=0.22,
    )

    mesh.set_facecolor(
        colors[face_materials] / 255.0
    )

    mesh.set_edgecolor(
        (0, 0, 0, 0.28)
    )

    ax.add_collection3d(mesh)

    x_extent = max(
        0.30,
        float(np.abs(vertices[:, 0]).max()) * 1.30,
    )

    z_extent = max(
        0.15,
        float(np.abs(vertices[:, 2]).max()) * 1.30,
    )

    ax.set_xlim(-x_extent, x_extent)
    ax.set_ylim(-1.08, 1.08)
    ax.set_zlim(-z_extent, z_extent)

    ax.set_box_aspect(
        (
            x_extent * 2.0,
            2.16,
            z_extent * 2.0,
        )
    )

    ax.view_init(elev=12, azim=-55)
    ax.set_axis_off()

    figure.tight_layout()
    figure.savefig(
        path,
        dpi=220,
        bbox_inches="tight",
    )

    plt.close(figure)


# ------------------------------------------------------------
# Main
# ------------------------------------------------------------

def main():
    args = parse_args()

    args.output_dir.mkdir(
        parents=True,
        exist_ok=True,
    )

    image = Image.open(args.image).convert("RGB")
    pixels = np.asarray(image)

    height, width, _ = pixels.shape

    # Tuned for the same concept-sheet layout style:
    # center sword = side view
    # right sword  = front/main readable view
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

    side_crop = pixels[sy0:sy1, sx0:sx1]
    front_crop = pixels[fy0:fy1, fx0:fx1]

    side_mask = make_mask(side_crop)
    front_mask = make_mask(front_crop)

    side_region = longest_vertical_region(side_mask)
    front_region = longest_vertical_region(front_mask)

    materials = build_material_palette(
        front_crop=front_crop,
        front_mask=front_mask,
        front_region=front_region,
    )

    vertices, faces, face_materials = build_mesh(
        front_crop=front_crop,
        front_mask=front_mask,
        front_region=front_region,
        side_crop=side_crop,
        side_mask=side_mask,
        side_region=side_region,
        materials=materials,
        ring_count=args.rings,
        smooth_window=args.smooth_window,
    )

    obj_path = args.output_dir / f"{args.name}.obj"
    mtl_path = args.output_dir / f"{args.name}.mtl"
    preview_path = args.output_dir / f"{args.name}_preview.png"

    write_mtl(mtl_path, materials)

    write_obj(
        path=obj_path,
        mtl_name=mtl_path.name,
        object_name=args.name,
        vertices=vertices,
        faces=faces,
        face_materials=face_materials,
        materials=materials,
    )

    save_preview(
        path=preview_path,
        vertices=vertices,
        faces=faces,
        face_materials=face_materials,
        materials=materials,
    )

    print(f"Vertices : {len(vertices)}")
    print(f"Triangles: {len(faces)}")
    print()

    print("Materials:")
    for name, rgb in materials:
        print(
            f"  {name:15s} "
            f"{tuple(int(round(v)) for v in rgb)}"
        )

    print()
    counts = Counter(int(x) for x in face_materials)
    material_names = [name for name, _ in materials]

    print("Face counts:")
    for index, count in sorted(counts.items()):
        print(
            f"  {material_names[index]:15s} {count}"
        )

    print()
    print("OBJ    :", obj_path)
    print("MTL    :", mtl_path)
    print("Preview:", preview_path)


if __name__ == "__main__":
    main()
