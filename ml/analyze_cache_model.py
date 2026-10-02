#!/usr/bin/env python3

import argparse
import csv
import math
from collections import Counter, defaultdict
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection


def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("--model", type=int, required=True)
    p.add_argument(
        "--dataset",
        type=Path,
        default=Path("dataset"),
    )
    p.add_argument(
        "--output-dir",
        type=Path,
        default=None,
    )
    return p.parse_args()


# ------------------------------------------------------------
# RuneScape packed-HSL decoding
# ------------------------------------------------------------

def unpack_rs_hsl(value):
    value = int(value)

    hue = (value >> 10) & 0x3F
    sat = (value >> 7) & 0x07
    light = value & 0x7F

    return hue, sat, light


def hsl_to_rgb(h, s, l):
    # Convert RuneScape-ish discrete HSL to normalized HSL.
    h = h / 64.0
    s = s / 8.0
    l = l / 128.0

    if s <= 1e-8:
        v = int(round(l * 255))
        return (v, v, v)

    def hue_to_rgb(p, q, t):
        if t < 0:
            t += 1
        if t > 1:
            t -= 1
        if t < 1 / 6:
            return p + (q - p) * 6 * t
        if t < 1 / 2:
            return q
        if t < 2 / 3:
            return p + (q - p) * (2 / 3 - t) * 6
        return p

    if l < 0.5:
        q = l * (1 + s)
    else:
        q = l + s - l * s

    p = 2 * l - q

    r = hue_to_rgb(p, q, h + 1 / 3)
    g = hue_to_rgb(p, q, h)
    b = hue_to_rgb(p, q, h - 1 / 3)

    return (
        int(round(r * 255)),
        int(round(g * 255)),
        int(round(b * 255)),
    )


def rs_color_to_rgb(value):
    return hsl_to_rgb(
        *unpack_rs_hsl(value)
    )


# ------------------------------------------------------------
# Geometry
# ------------------------------------------------------------

def face_normal(a, b, c):
    ab = b - a
    ac = c - a

    n = np.cross(ab, ac)

    length = np.linalg.norm(n)

    if length <= 1e-10:
        return np.zeros(3)

    return n / length


def triangle_area(a, b, c):
    return 0.5 * np.linalg.norm(
        np.cross(
            b - a,
            c - a,
        )
    )


def load_model(dataset, model_id):
    vertices = []

    with (dataset / "vertices.csv").open(
        newline="",
        encoding="utf-8",
    ) as f:
        for row in csv.DictReader(f):
            if int(row["model_id"]) != model_id:
                continue

            vertices.append({
                "index": len(vertices),
                "x": float(row["x"]),
                "y": float(row["y"]),
                "z": float(row["z"]),
                "skin": row["skin"],
            })

    faces = []

    with (dataset / "faces.csv").open(
        newline="",
        encoding="utf-8",
    ) as f:
        for row in csv.DictReader(f):
            if int(row["model_id"]) != model_id:
                continue

            faces.append({
                "index": int(row["face_index"]),
                "a": int(row["a"]),
                "b": int(row["b"]),
                "c": int(row["c"]),
                "color": int(row["color"]),
                "priority": int(row["priority"]),
                "alpha": int(row["alpha"]),
                "render_type": int(row["render_type"]),
                "skin": row["skin"],
                "texture_id": row["texture_id"],
                "texture_mapping_index":
                    row["texture_mapping_index"],
            })

    if not vertices:
        raise RuntimeError(
            f"Model {model_id} has no vertices."
        )

    if not faces:
        raise RuntimeError(
            f"Model {model_id} has no faces."
        )

    return vertices, faces


def main():
    args = parse_args()

    out = (
        args.output_dir
        or Path(f"dataset/ml/model_analysis/{args.model}")
    )

    out.mkdir(
        parents=True,
        exist_ok=True,
    )

    vertices, faces = load_model(
        args.dataset,
        args.model,
    )

    points = np.asarray([
        [v["x"], v["y"], v["z"]]
        for v in vertices
    ], dtype=np.float64)

    mins = points.min(axis=0)
    maxs = points.max(axis=0)
    center = (mins + maxs) * 0.5
    spans = maxs - mins

    print(f"=== MODEL {args.model} ===")
    print("vertices:", len(vertices))
    print("faces   :", len(faces))
    print()

    for axis, i in zip("XYZ", range(3)):
        print(
            f"{axis}: "
            f"min={mins[i]:8.3f} "
            f"max={maxs[i]:8.3f} "
            f"span={spans[i]:8.3f} "
            f"center={center[i]:8.3f}"
        )

    print()

    print(
        "vertex skins:",
        Counter(v["skin"] for v in vertices),
    )

    print(
        "face skins:",
        Counter(f["skin"] for f in faces),
    )

    print(
        "render types:",
        Counter(f["render_type"] for f in faces),
    )

    print(
        "priorities:",
        Counter(f["priority"] for f in faces),
    )

    print(
        "alphas:",
        Counter(f["alpha"] for f in faces),
    )

    print(
        "textures:",
        Counter(f["texture_id"] for f in faces),
    )

    print()


    # --------------------------------------------------------
    # Detailed per-face calculations
    # --------------------------------------------------------

    detailed = []

    by_color = defaultdict(list)

    for f in faces:
        a = points[f["a"]]
        b = points[f["b"]]
        c = points[f["c"]]

        centroid = (
            a + b + c
        ) / 3.0

        normal = face_normal(
            a, b, c
        )

        area = triangle_area(
            a, b, c
        )

        hue, sat, light = unpack_rs_hsl(
            f["color"]
        )

        rgb = rs_color_to_rgb(
            f["color"]
        )

        record = {
            **f,
            "centroid_x": centroid[0],
            "centroid_y": centroid[1],
            "centroid_z": centroid[2],
            "normal_x": normal[0],
            "normal_y": normal[1],
            "normal_z": normal[2],
            "area": area,
            "hue": hue,
            "saturation": sat,
            "lightness": light,
            "rgb_r": rgb[0],
            "rgb_g": rgb[1],
            "rgb_b": rgb[2],
        }

        detailed.append(record)

        by_color[f["color"]].append(
            record
        )


    # --------------------------------------------------------
    # Color-group report
    # --------------------------------------------------------

    print("=== COLOR GROUPS ===")
    print()

    for color, group in sorted(
        by_color.items(),
        key=lambda item: -len(item[1]),
    ):
        hue, sat, light = unpack_rs_hsl(
            color
        )

        rgb = rs_color_to_rgb(
            color
        )

        centroids = np.asarray([
            [
                f["centroid_x"],
                f["centroid_y"],
                f["centroid_z"],
            ]
            for f in group
        ])

        cmin = centroids.min(axis=0)
        cmax = centroids.max(axis=0)
        ccenter = centroids.mean(axis=0)

        total_area = sum(
            f["area"]
            for f in group
        )

        print(
            f"color={color:5d} "
            f"faces={len(group):3d} "
            f"HSL=({hue:2d},{sat},{light:3d}) "
            f"RGB={rgb} "
            f"area={total_area:8.2f}"
        )

        print(
            "    centroid = "
            f"({ccenter[0]:7.2f}, "
            f"{ccenter[1]:7.2f}, "
            f"{ccenter[2]:7.2f})"
        )

        print(
            "    range X  = "
            f"{cmin[0]:7.2f} .. {cmax[0]:7.2f}"
        )

        print(
            "    range Y  = "
            f"{cmin[1]:7.2f} .. {cmax[1]:7.2f}"
        )

        print(
            "    range Z  = "
            f"{cmin[2]:7.2f} .. {cmax[2]:7.2f}"
        )

        normals = np.asarray([
            [
                f["normal_x"],
                f["normal_y"],
                f["normal_z"],
            ]
            for f in group
        ])

        avg_normal = normals.mean(axis=0)

        print(
            "    avg normal = "
            f"({avg_normal[0]: .3f}, "
            f"{avg_normal[1]: .3f}, "
            f"{avg_normal[2]: .3f})"
        )

        print()


    # --------------------------------------------------------
    # Save vertex table
    # --------------------------------------------------------

    with (
        out / "vertices_detailed.csv"
    ).open(
        "w",
        newline="",
        encoding="utf-8",
    ) as f:

        writer = csv.DictWriter(
            f,
            fieldnames=[
                "index",
                "x",
                "y",
                "z",
                "skin",
            ],
        )

        writer.writeheader()
        writer.writerows(vertices)


    # --------------------------------------------------------
    # Save face table
    # --------------------------------------------------------

    face_fields = list(
        detailed[0].keys()
    )

    with (
        out / "faces_detailed.csv"
    ).open(
        "w",
        newline="",
        encoding="utf-8",
    ) as f:

        writer = csv.DictWriter(
            f,
            fieldnames=face_fields,
        )

        writer.writeheader()
        writer.writerows(detailed)


    # --------------------------------------------------------
    # Native-color OBJ
    # --------------------------------------------------------

    colors = sorted(
        by_color.keys()
    )

    material_names = {
        color: f"rs_{color}"
        for color in colors
    }

    mtl_path = out / "native.mtl"
    obj_path = out / "native.obj"

    with mtl_path.open(
        "w",
        encoding="utf-8",
    ) as f:

        for color in colors:
            rgb = np.asarray(
                rs_color_to_rgb(color),
                dtype=np.float64,
            ) / 255.0

            f.write(
                f"newmtl {material_names[color]}\n"
            )

            f.write(
                f"Kd {rgb[0]:.6f} "
                f"{rgb[1]:.6f} "
                f"{rgb[2]:.6f}\n"
            )

            f.write("Ka 0 0 0\n")
            f.write("Ks 0 0 0\n")
            f.write("illum 1\n\n")


    with obj_path.open(
        "w",
        encoding="utf-8",
    ) as f:

        f.write(
            f"mtllib {mtl_path.name}\n"
        )

        f.write(
            f"o Cache_Model_{args.model}\n"
        )

        for p in points:
            f.write(
                f"v {p[0]:.6f} "
                f"{p[1]:.6f} "
                f"{p[2]:.6f}\n"
            )

        current = None

        for face in faces:
            color = face["color"]

            if color != current:
                f.write(
                    f"usemtl {material_names[color]}\n"
                )
                current = color

            f.write(
                f"f "
                f"{face['a'] + 1} "
                f"{face['b'] + 1} "
                f"{face['c'] + 1}\n"
            )


    # --------------------------------------------------------
    # Preview
    # --------------------------------------------------------

    triangles = np.asarray([
        [
            points[f["a"]],
            points[f["b"]],
            points[f["c"]],
        ]
        for f in faces
    ])

    face_colors = np.asarray([
        rs_color_to_rgb(f["color"])
        for f in faces
    ], dtype=np.float64) / 255.0

    fig = plt.figure(
        figsize=(8, 8)
    )

    ax = fig.add_subplot(
        111,
        projection="3d",
    )

    collection = Poly3DCollection(
        triangles,
        linewidths=0.45,
    )

    collection.set_facecolor(
        face_colors
    )

    collection.set_edgecolor(
        (0.0, 0.0, 0.0, 0.35)
    )

    ax.add_collection3d(
        collection
    )

    pad = np.maximum(
        spans * 0.15,
        2.0,
    )

    ax.set_xlim(
        mins[0] - pad[0],
        maxs[0] + pad[0],
    )

    ax.set_ylim(
        mins[1] - pad[1],
        maxs[1] + pad[1],
    )

    ax.set_zlim(
        mins[2] - pad[2],
        maxs[2] + pad[2],
    )

    ax.set_box_aspect(
        np.maximum(
            spans,
            1.0,
        )
    )

    ax.view_init(
        elev=18,
        azim=-55,
    )

    ax.set_axis_off()

    fig.tight_layout()

    preview = out / "native_preview.png"

    fig.savefig(
        preview,
        dpi=220,
        bbox_inches="tight",
    )

    plt.close(fig)


    print("=== OUTPUT ===")
    print(out)
    print(" ", obj_path)
    print(" ", mtl_path)
    print(" ", preview)
    print(" ", out / "vertices_detailed.csv")
    print(" ", out / "faces_detailed.csv")


if __name__ == "__main__":
    main()
