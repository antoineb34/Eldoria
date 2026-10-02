#!/usr/bin/env python3

import argparse
import csv
import hashlib
from collections import defaultdict
from pathlib import Path

import numpy as np


def geometry_hash(points):
    points = np.asarray(
        points,
        dtype=np.float64,
    )

    center = points.mean(
        axis=0,
        keepdims=True,
    )

    points = points - center

    radius = np.linalg.norm(
        points,
        axis=1,
    ).max()

    if radius > 0:
        points = points / radius

    # Make tiny floating differences irrelevant.
    points = np.round(
        points,
        decimals=6,
    )

    # Vertex ordering shouldn't matter.
    order = np.lexsort(
        (
            points[:, 2],
            points[:, 1],
            points[:, 0],
        )
    )

    points = points[order]

    return hashlib.sha256(
        points.tobytes()
    ).hexdigest()


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--vertices",
        type=Path,
        default=Path(
            "dataset/vertices.csv"
        ),
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path(
            "dataset/ml/"
            "geometry_hashes.csv"
        ),
    )

    args = parser.parse_args()

    vertices = defaultdict(list)

    print("Reading vertices...")

    with args.vertices.open(
        newline="",
        encoding="utf-8",
    ) as file:
        reader = csv.DictReader(file)

        for row in reader:
            model_id = int(
                row["model_id"]
            )

            vertices[model_id].append(
                (
                    float(row["x"]),
                    float(row["y"]),
                    float(row["z"]),
                )
            )

    hashes = {}

    groups = defaultdict(list)

    for model_id, points in vertices.items():
        fingerprint = geometry_hash(
            points
        )

        hashes[model_id] = fingerprint

        groups[fingerprint].append(
            model_id
        )

    args.output.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    with args.output.open(
        "w",
        newline="",
    ) as file:
        writer = csv.writer(file)

        writer.writerow(
            [
                "model_id",
                "geometry_hash",
            ]
        )

        for model_id in sorted(
            hashes
        ):
            writer.writerow(
                [
                    model_id,
                    hashes[model_id],
                ]
            )

    duplicate_groups = [
        ids
        for ids in groups.values()
        if len(ids) > 1
    ]

    duplicate_models = sum(
        len(ids)
        for ids in duplicate_groups
    )

    print()
    print(
        "Models hashed:",
        len(hashes),
    )

    print(
        "Duplicate groups:",
        len(duplicate_groups),
    )

    print(
        "Models in duplicate groups:",
        duplicate_models,
    )

    print()
    print(
        "Largest duplicate groups:"
    )

    duplicate_groups.sort(
        key=len,
        reverse=True,
    )

    for ids in duplicate_groups[:20]:
        print(
            f"{len(ids):4d} models:",
            ids[:20],
            "..." if len(ids) > 20 else "",
        )

    print()
    print(
        "Saved:",
        args.output,
    )


if __name__ == "__main__":
    main()
