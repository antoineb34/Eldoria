#!/usr/bin/env python3

import colorsys
import csv
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d.art3d import Poly3DCollection


VERTICES = Path("dataset/vertices.csv")
FACES = Path("dataset/faces.csv")

SWORDS = [
    (2602, "Bronze longsword"),
    (2604, "Bronze sword"),
    (2608, "Rusty sword"),
    (2754, "Bronze 2h sword"),
    (2834, "Dragon longsword"),
    (2503, "Blurite sword"),
]


def rs_color_to_rgb(value):
    hue = ((value >> 10) & 0x3F) / 64.0
    saturation = ((value >> 7) & 0x07) / 8.0
    lightness = (value & 0x7F) / 127.0

    return colorsys.hls_to_rgb(
        hue,
        lightness,
        saturation,
    )


def load_vertices(model_ids):
    wanted = set(model_ids)
    out = {model_id: {} for model_id in wanted}

    with VERTICES.open(
        newline="",
        encoding="utf-8",
    ) as f:
        reader = csv.DictReader(f)

        for row in reader:
            model_id = int(row["model_id"])

            if model_id not in wanted:
                continue

            index = int(row["vertex_index"])

            out[model_id][index] = (
                float(row["x"]),
                float(row["y"]),
                float(row["z"]),
            )

    result = {}

    for model_id, values in out.items():
        if not values:
            continue

        count = max(values) + 1

        vertices = np.zeros(
            (count, 3),
            dtype=np.float64,
        )

        for index, xyz in values.items():
            vertices[index] = xyz

        result[model_id] = vertices

    return result


def load_faces(model_ids):
    wanted = set(model_ids)

    faces = {
        model_id: []
        for model_id in wanted
    }

    colors = {
        model_id: []
        for model_id in wanted
    }

    with FACES.open(
        newline="",
        encoding="utf-8",
    ) as f:
        reader = csv.DictReader(f)

        for row in reader:
            model_id = int(row["model_id"])

            if model_id not in wanted:
                continue

            faces[model_id].append(
                (
                    int(row["a"]),
                    int(row["b"]),
                    int(row["c"]),
                )
            )

            colors[model_id].append(
                rs_color_to_rgb(
                    int(row["color"])
                )
            )

    return faces, colors


def normalize(vertices):
    center = vertices.mean(
        axis=0,
        keepdims=True,
    )

    vertices = vertices - center

    radius = np.linalg.norm(
        vertices,
        axis=1,
    ).max()

    if radius > 0:
        vertices /= radius

    return vertices


def render_mesh(
    ax,
    vertices,
    faces,
    colors,
    title,
    azim=-55,
):
    triangles = vertices[
        np.asarray(
            faces,
            dtype=np.int32,
        )
    ]

    collection = Poly3DCollection(
        triangles,
        linewidths=0.25,
    )

    collection.set_facecolor(
        colors
    )

    collection.set_edgecolor(
        (0, 0, 0, 0.25)
    )

    ax.add_collection3d(
        collection
    )

    ax.set_xlim(-1.05, 1.05)
    ax.set_ylim(-1.05, 1.05)
    ax.set_zlim(-1.05, 1.05)

    ax.set_box_aspect(
        (1, 1, 1)
    )

    ax.view_init(
        elev=18,
        azim=azim,
    )

    ax.set_title(
        title,
        fontsize=10,
    )

    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_zticks([])

    ax.set_axis_off()


def main():
    ids = [
        model_id
        for model_id, _ in SWORDS
    ]

    vertices = load_vertices(ids)
    faces, colors = load_faces(ids)

    fig = plt.figure(
        figsize=(16, 10)
    )

    for i, (
        model_id,
        name,
    ) in enumerate(
        SWORDS,
        start=1,
    ):
        ax = fig.add_subplot(
            2,
            3,
            i,
            projection="3d",
        )

        verts = normalize(
            vertices[model_id].copy()
        )

        render_mesh(
            ax,
            verts,
            faces[model_id],
            colors[model_id],
            f"{name}\nmodel {model_id}",
        )

    fig.suptitle(
        "Eldoria / RuneScape Sword Style References",
        fontsize=16,
    )

    fig.tight_layout()

    output = Path(
        "dataset/ml/sword_style_sheet.png"
    )

    fig.savefig(
        output,
        dpi=220,
        bbox_inches="tight",
    )

    print(
        "Saved:",
        output,
    )

    plt.show()


if __name__ == "__main__":
    main()
