#!/usr/bin/env python3

import argparse
import csv
import itertools
from pathlib import Path

import numpy as np


def read_model_ids(path: Path):
    ids = []

    with path.open(newline="") as f:
        reader = csv.DictReader(f)

        for row in reader:
            ids.append(int(row["model_id"]))

    return ids


def grouped_vertices(path: Path):
    with path.open(newline="") as f:
        reader = csv.DictReader(f)

        for model_id, rows in itertools.groupby(
            reader,
            key=lambda row: int(row["model_id"]),
        ):
            vertices = []

            for row in rows:
                vertices.append((
                    float(row["x"]),
                    float(row["y"]),
                    float(row["z"]),
                ))

            yield model_id, np.asarray(
                vertices,
                dtype=np.float32,
            )


def grouped_faces(path: Path):
    with path.open(newline="") as f:
        reader = csv.DictReader(f)

        for model_id, rows in itertools.groupby(
            reader,
            key=lambda row: int(row["model_id"]),
        ):
            faces = []

            for row in rows:
                faces.append((
                    int(row["a"]),
                    int(row["b"]),
                    int(row["c"]),
                ))

            yield model_id, np.asarray(
                faces,
                dtype=np.int32,
            )


def take_group(iterator, current, model_id):
    while current is not None and current[0] < model_id:
        current = next(iterator, None)

    if current is not None and current[0] == model_id:
        value = current[1]
        current = next(iterator, None)
        return value, current

    return None, current


def sample_surface(
    vertices: np.ndarray,
    faces: np.ndarray,
    point_count: int,
    rng: np.random.Generator,
):
    if vertices is None or faces is None:
        return None, 0.0

    if len(vertices) == 0 or len(faces) == 0:
        return None, 0.0

    valid_indices = (
        (faces >= 0).all(axis=1)
        & (faces < len(vertices)).all(axis=1)
    )

    faces = faces[valid_indices]

    if len(faces) == 0:
        return None, 0.0

    triangles = vertices[faces]

    a = triangles[:, 0]
    b = triangles[:, 1]
    c = triangles[:, 2]

    cross = np.cross(
        b - a,
        c - a,
    )

    areas = 0.5 * np.linalg.norm(
        cross,
        axis=1,
    )

    valid_area = (
        np.isfinite(areas)
        & (areas > 1e-8)
    )

    triangles = triangles[valid_area]
    areas = areas[valid_area]

    if len(triangles) == 0:
        return None, 0.0

    total_area = float(areas.sum())

    probabilities = areas / total_area

    triangle_indices = rng.choice(
        len(triangles),
        size=point_count,
        replace=True,
        p=probabilities,
    )

    chosen = triangles[triangle_indices]

    # Uniform sampling inside a triangle.
    u = rng.random(point_count)
    v = rng.random(point_count)

    sqrt_u = np.sqrt(u)

    w0 = 1.0 - sqrt_u
    w1 = sqrt_u * (1.0 - v)
    w2 = sqrt_u * v

    points = (
        chosen[:, 0] * w0[:, None]
        + chosen[:, 1] * w1[:, None]
        + chosen[:, 2] * w2[:, None]
    )

    return points.astype(np.float32), total_area


def normalize_points(points: np.ndarray):
    center = points.mean(
        axis=0,
        dtype=np.float64,
    ).astype(np.float32)

    centered = points - center

    scale = float(
        np.linalg.norm(
            centered,
            axis=1,
        ).max()
    )

    if not np.isfinite(scale) or scale <= 1e-8:
        return None

    normalized = centered / scale

    return (
        normalized.astype(np.float32),
        center,
        np.float32(scale),
    )


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--dataset",
        type=Path,
        default=Path("dataset"),
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path("dataset/ml"),
    )

    parser.add_argument(
        "--points",
        type=int,
        default=2048,
    )

    parser.add_argument(
        "--seed",
        type=int,
        default=1337,
    )

    parser.add_argument(
        "--limit",
        type=int,
        default=None,
    )

    args = parser.parse_args()

    model_ids = read_model_ids(
        args.dataset / "models.csv"
    )

    if args.limit is not None:
        model_ids = model_ids[:args.limit]

    args.output.mkdir(
        parents=True,
        exist_ok=True,
    )

    model_count = len(model_ids)

    print(
        f"Models: {model_count}\n"
        f"Points/model: {args.points}"
    )

    pointclouds = np.lib.format.open_memmap(
        args.output / "pointclouds.npy",
        mode="w+",
        dtype=np.float32,
        shape=(
            model_count,
            args.points,
            3,
        ),
    )

    ids_out = np.lib.format.open_memmap(
        args.output / "model_ids.npy",
        mode="w+",
        dtype=np.uint16,
        shape=(model_count,),
    )

    valid_out = np.lib.format.open_memmap(
        args.output / "valid.npy",
        mode="w+",
        dtype=np.bool_,
        shape=(model_count,),
    )

    centers_out = np.lib.format.open_memmap(
        args.output / "centers.npy",
        mode="w+",
        dtype=np.float32,
        shape=(model_count, 3),
    )

    scales_out = np.lib.format.open_memmap(
        args.output / "scales.npy",
        mode="w+",
        dtype=np.float32,
        shape=(model_count,),
    )

    areas_out = np.lib.format.open_memmap(
        args.output / "surface_areas.npy",
        mode="w+",
        dtype=np.float32,
        shape=(model_count,),
    )

    vertex_iterator = iter(
        grouped_vertices(
            args.dataset / "vertices.csv"
        )
    )

    face_iterator = iter(
        grouped_faces(
            args.dataset / "faces.csv"
        )
    )

    current_vertices = next(
        vertex_iterator,
        None,
    )

    current_faces = next(
        face_iterator,
        None,
    )

    valid_count = 0

    for row_index, model_id in enumerate(model_ids):
        vertices, current_vertices = take_group(
            vertex_iterator,
            current_vertices,
            model_id,
        )

        faces, current_faces = take_group(
            face_iterator,
            current_faces,
            model_id,
        )

        # Deterministic per model.
        rng = np.random.default_rng(
            args.seed + model_id
        )

        points, surface_area = sample_surface(
            vertices,
            faces,
            args.points,
            rng,
        )

        ids_out[row_index] = model_id

        pointclouds[row_index] = 0.0
        centers_out[row_index] = 0.0
        scales_out[row_index] = 0.0
        areas_out[row_index] = surface_area
        valid_out[row_index] = False

        if points is not None:
            normalized = normalize_points(
                points
            )

            if normalized is not None:
                (
                    normalized_points,
                    center,
                    scale,
                ) = normalized

                pointclouds[row_index] = (
                    normalized_points
                )

                centers_out[row_index] = center
                scales_out[row_index] = scale
                valid_out[row_index] = True

                valid_count += 1

        if (
            (row_index + 1) % 500 == 0
            or row_index + 1 == model_count
        ):
            print(
                f"{row_index + 1:5d}/"
                f"{model_count}  "
                f"valid={valid_count}"
            )

    pointclouds.flush()
    ids_out.flush()
    valid_out.flush()
    centers_out.flush()
    scales_out.flush()
    areas_out.flush()

    print()
    print("Done.")
    print(f"Valid models:   {valid_count}")
    print(
        f"Invalid models: "
        f"{model_count - valid_count}"
    )
    print(f"Output: {args.output}")


if __name__ == "__main__":
    main()
