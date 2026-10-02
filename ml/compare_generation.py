#!/usr/bin/env python3

import argparse
import colorsys
import csv
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import open3d as o3d

from mpl_toolkits.mplot3d.art3d import Poly3DCollection


def rs_color_to_rgb(value):
    # RuneScape packed HSL-ish format:
    # HHHHHH SSS LLLLLLL
    hue = ((value >> 10) & 0x3F) / 64.0
    saturation = ((value >> 7) & 0x07) / 8.0
    lightness = (value & 0x7F) / 127.0

    r, g, b = colorsys.hls_to_rgb(
        hue,
        lightness,
        saturation,
    )

    return (r, g, b)


def load_raw_mesh(
    model_id,
    vertices_path,
    faces_path,
):
    vertices = {}

    with vertices_path.open(
        newline="",
        encoding="utf-8",
    ) as file:
        reader = csv.DictReader(file)

        for row in reader:
            if int(row["model_id"]) != model_id:
                continue

            index = int(
                row["vertex_index"]
            )

            vertices[index] = (
                float(row["x"]),
                float(row["y"]),
                float(row["z"]),
            )

    if not vertices:
        raise ValueError(
            f"No vertices for model {model_id}"
        )

    max_index = max(vertices)

    ordered = np.zeros(
        (max_index + 1, 3),
        dtype=np.float64,
    )

    for index, xyz in vertices.items():
        ordered[index] = xyz

    faces = []
    colors = []

    with faces_path.open(
        newline="",
        encoding="utf-8",
    ) as file:
        reader = csv.DictReader(file)

        for row in reader:
            if int(row["model_id"]) != model_id:
                continue

            faces.append(
                (
                    int(row["a"]),
                    int(row["b"]),
                    int(row["c"]),
                )
            )

            colors.append(
                rs_color_to_rgb(
                    int(row["color"])
                )
            )

    return (
        ordered,
        np.asarray(
            faces,
            dtype=np.int32,
        ),
        colors,
    )


def normalize_raw_mesh(
    vertices,
    model_id,
    model_ids,
    centers,
    scales,
):
    matches = np.where(
        model_ids == model_id
    )[0]

    if len(matches) != 1:
        raise ValueError(
            f"Could not locate model {model_id}"
        )

    row = int(matches[0])

    center = centers[row]
    scale = float(scales[row])

    return (
        vertices - center
    ) / scale


def reconstruct_surface(points):
    cloud = o3d.geometry.PointCloud()

    cloud.points = (
        o3d.utility.Vector3dVector(
            points.astype(np.float64)
        )
    )

    cloud.estimate_normals(
        search_param=
        o3d.geometry.KDTreeSearchParamHybrid(
            radius=0.20,
            max_nn=30,
        )
    )

    try:
        cloud.orient_normals_consistent_tangent_plane(
            20
        )
    except RuntimeError:
        pass

    distances = np.asarray(
        cloud.compute_nearest_neighbor_distance()
    )

    spacing = float(
        np.median(distances)
    )

    radii = o3d.utility.DoubleVector(
        [
            spacing * 1.5,
            spacing * 2.0,
            spacing * 3.0,
        ]
    )

    mesh = (
        o3d.geometry.TriangleMesh
        .create_from_point_cloud_ball_pivoting(
            cloud,
            radii,
        )
    )

    mesh.remove_degenerate_triangles()
    mesh.remove_duplicated_triangles()
    mesh.remove_duplicated_vertices()

    return (
        np.asarray(mesh.vertices),
        np.asarray(mesh.triangles),
    )


def draw_mesh(
    ax,
    vertices,
    faces,
    title,
    face_colors=None,
):
    triangles = vertices[
        faces
    ]

    collection = Poly3DCollection(
        triangles,
        linewidths=0.1,
    )

    if face_colors is not None:
        collection.set_facecolor(
            face_colors
        )
    else:
        # Let matplotlib use its
        # default solid face appearance.
        collection.set_alpha(
            0.9
        )

    collection.set_edgecolor(
        (0, 0, 0, 0.12)
    )

    ax.add_collection3d(
        collection
    )

    ax.set_title(
        title
    )

    ax.set_xlim(
        -1.05,
        1.05,
    )

    ax.set_ylim(
        -1.05,
        1.05,
    )

    ax.set_zlim(
        -1.05,
        1.05,
    )

    ax.set_box_aspect(
        (1, 1, 1)
    )

    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_zticks([])

    # Same camera everywhere.
    ax.view_init(
        elev=20,
        azim=-60,
    )


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "model_a",
        type=int,
    )

    parser.add_argument(
        "model_b",
        type=int,
    )

    parser.add_argument(
        "--data",
        type=Path,
        default=Path("dataset"),
    )

    parser.add_argument(
        "--ml-data",
        type=Path,
        default=Path("dataset/ml"),
    )

    args = parser.parse_args()

    run_dir = (
        args.ml_data
        / "interpolations"
        / f"{args.model_a}_to_{args.model_b}"
    )

    step_a = np.load(
        run_dir / "step_00.npy"
    )

    step_mid = np.load(
        run_dir / "step_02.npy"
    )

    step_b = np.load(
        run_dir / "step_04.npy"
    )

    model_ids = np.load(
        args.ml_data
        / "model_ids.npy"
    )

    centers = np.load(
        args.ml_data
        / "centers.npy"
    )

    scales = np.load(
        args.ml_data
        / "scales.npy"
    )

    raw_a, faces_a, colors_a = (
        load_raw_mesh(
            args.model_a,
            args.data / "vertices.csv",
            args.data / "faces.csv",
        )
    )

    raw_b, faces_b, colors_b = (
        load_raw_mesh(
            args.model_b,
            args.data / "vertices.csv",
            args.data / "faces.csv",
        )
    )

    raw_a = normalize_raw_mesh(
        raw_a,
        args.model_a,
        model_ids,
        centers,
        scales,
    )

    raw_b = normalize_raw_mesh(
        raw_b,
        args.model_b,
        model_ids,
        centers,
        scales,
    )

    rec_a_v, rec_a_f = (
        reconstruct_surface(
            step_a
        )
    )

    mid_v, mid_f = (
        reconstruct_surface(
            step_mid
        )
    )

    rec_b_v, rec_b_f = (
        reconstruct_surface(
            step_b
        )
    )

    print(
        f"Original {args.model_a}: "
        f"{len(raw_a)} vertices, "
        f"{len(faces_a)} faces"
    )

    print(
        f"Reconstructed {args.model_a}: "
        f"{len(rec_a_v)} vertices, "
        f"{len(rec_a_f)} faces"
    )

    print(
        f"Generated midpoint: "
        f"{len(mid_v)} vertices, "
        f"{len(mid_f)} faces"
    )

    print(
        f"Reconstructed {args.model_b}: "
        f"{len(rec_b_v)} vertices, "
        f"{len(rec_b_f)} faces"
    )

    print(
        f"Original {args.model_b}: "
        f"{len(raw_b)} vertices, "
        f"{len(faces_b)} faces"
    )

    figure = plt.figure(
        figsize=(22, 5)
    )

    panels = [
        (
            raw_a,
            faces_a,
            f"ORIGINAL\n{args.model_a}",
            colors_a,
        ),
        (
            rec_a_v,
            rec_a_f,
            f"AE RECONSTRUCTION\n{args.model_a}",
            None,
        ),
        (
            mid_v,
            mid_f,
            "GENERATED\n50 / 50",
            None,
        ),
        (
            rec_b_v,
            rec_b_f,
            f"AE RECONSTRUCTION\n{args.model_b}",
            None,
        ),
        (
            raw_b,
            faces_b,
            f"ORIGINAL\n{args.model_b}",
            colors_b,
        ),
    ]

    for index, panel in enumerate(
        panels,
        start=1,
    ):
        ax = figure.add_subplot(
            1,
            5,
            index,
            projection="3d",
        )

        draw_mesh(
            ax,
            *panel,
        )

    figure.suptitle(
        "Where is the failure?"
    )

    figure.tight_layout()

    output = (
        run_dir
        / "diagnostic_comparison.png"
    )

    figure.savefig(
        output,
        dpi=180,
    )

    print()
    print(
        "Saved:",
        output,
    )

    plt.show()


if __name__ == "__main__":
    main()
