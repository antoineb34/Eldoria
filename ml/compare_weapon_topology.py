#!/usr/bin/env python3

import csv
import math
from collections import Counter, defaultdict, deque
from pathlib import Path

import numpy as np


DATASET = Path("dataset")
MODELS = [490, 6031]


def load_model(model_id):
    vertices = {}

    with (DATASET / "vertices.csv").open(
        newline="",
        encoding="utf-8",
    ) as f:
        for row in csv.DictReader(f):
            if int(row["model_id"]) != model_id:
                continue

            index = int(row["vertex_index"])

            vertices[index] = np.array([
                float(row["x"]),
                float(row["y"]),
                float(row["z"]),
            ])

    faces = []

    with (DATASET / "faces.csv").open(
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
                "render_type": int(row["render_type"]),
                "priority": int(row["priority"]),
                "alpha": int(row["alpha"]),
                "skin": row["skin"],
            })

    points = np.asarray([
        vertices[i]
        for i in sorted(vertices)
    ])

    return points, faces


def normal(a, b, c):
    n = np.cross(
        b - a,
        c - a,
    )

    length = np.linalg.norm(n)

    if length < 1e-9:
        return np.zeros(3)

    return n / length


def triangle_area(a, b, c):
    return (
        np.linalg.norm(
            np.cross(
                b - a,
                c - a,
            )
        )
        * 0.5
    )


def build_edges(faces):
    edges = defaultdict(list)

    for fi, face in enumerate(faces):
        indices = [
            face["a"],
            face["b"],
            face["c"],
        ]

        for i in range(3):
            a = indices[i]
            b = indices[(i + 1) % 3]

            edge = tuple(sorted((a, b)))

            edges[edge].append(fi)

    return edges


def connected_components(vertex_count, edges):
    graph = defaultdict(set)

    for a, b in edges:
        graph[a].add(b)
        graph[b].add(a)

    visited = set()
    components = []

    for start in range(vertex_count):
        if start in visited:
            continue

        queue = deque([start])
        visited.add(start)

        component = []

        while queue:
            v = queue.popleft()
            component.append(v)

            for other in graph[v]:
                if other not in visited:
                    visited.add(other)
                    queue.append(other)

        components.append(component)

    return components


def analyze(model_id):
    points, faces = load_model(model_id)

    mins = points.min(axis=0)
    maxs = points.max(axis=0)
    spans = maxs - mins

    major_axis = int(np.argmax(spans))
    major_name = "XYZ"[major_axis]

    edges = build_edges(faces)

    normals = []

    areas = []

    for face in faces:
        a = points[face["a"]]
        b = points[face["b"]]
        c = points[face["c"]]

        normals.append(
            normal(a, b, c)
        )

        areas.append(
            triangle_area(a, b, c)
        )

    normals = np.asarray(normals)
    areas = np.asarray(areas)

    boundary_edges = []
    manifold_edges = []
    nonmanifold_edges = []

    dihedrals = []

    for edge, adjacent in edges.items():
        if len(adjacent) == 1:
            boundary_edges.append(edge)

        elif len(adjacent) == 2:
            manifold_edges.append(edge)

            n1 = normals[adjacent[0]]
            n2 = normals[adjacent[1]]

            dot = float(
                np.clip(
                    abs(np.dot(n1, n2)),
                    0.0,
                    1.0,
                )
            )

            angle = math.degrees(
                math.acos(dot)
            )

            dihedrals.append(angle)

        else:
            nonmanifold_edges.append(edge)

    neighbors = defaultdict(set)

    for a, b in edges:
        neighbors[a].add(b)
        neighbors[b].add(a)

    valences = [
        len(neighbors[i])
        for i in range(len(points))
    ]

    components = connected_components(
        len(points),
        edges,
    )

    # Exact coordinate stations along the longest axis.
    station_values = defaultdict(list)

    for i, point in enumerate(points):
        station_values[
            float(point[major_axis])
        ].append(i)

    stations = sorted(
        station_values.items()
    )

    return {
        "id": model_id,
        "points": points,
        "faces": faces,
        "mins": mins,
        "maxs": maxs,
        "spans": spans,
        "major_axis": major_axis,
        "major_name": major_name,
        "edges": edges,
        "boundary_edges": boundary_edges,
        "manifold_edges": manifold_edges,
        "nonmanifold_edges": nonmanifold_edges,
        "dihedrals": np.asarray(dihedrals),
        "valences": valences,
        "components": components,
        "stations": stations,
        "areas": areas,
    }


def print_model(result):
    print()
    print("=" * 72)
    print(f"MODEL {result['id']}")
    print("=" * 72)

    print(
        f"vertices     : {len(result['points'])}"
    )

    print(
        f"triangles    : {len(result['faces'])}"
    )

    print(
        f"unique edges : {len(result['edges'])}"
    )

    print(
        f"components   : {len(result['components'])}"
    )

    print()

    print(
        "bbox spans   : "
        f"X={result['spans'][0]:.1f} "
        f"Y={result['spans'][1]:.1f} "
        f"Z={result['spans'][2]:.1f}"
    )

    print(
        "major axis   : "
        f"{result['major_name']}"
    )

    print()

    print(
        f"boundary edges    : "
        f"{len(result['boundary_edges'])}"
    )

    print(
        f"manifold edges    : "
        f"{len(result['manifold_edges'])}"
    )

    print(
        f"nonmanifold edges : "
        f"{len(result['nonmanifold_edges'])}"
    )

    print()

    valences = np.asarray(
        result["valences"]
    )

    print(
        "vertex valence:"
    )

    for value, count in sorted(
        Counter(valences).items()
    ):
        print(
            f"  {int(value):2d} neighbors : "
            f"{count:3d} vertices"
        )

    print()

    angles = result["dihedrals"]

    if len(angles):
        print("dihedral / crease angles:")

        print(
            f"  mean       : "
            f"{angles.mean():6.2f}°"
        )

        print(
            f"  median     : "
            f"{np.median(angles):6.2f}°"
        )

        print(
            f"  max        : "
            f"{angles.max():6.2f}°"
        )

        for threshold in (
            10,
            20,
            30,
            45,
            60,
        ):
            count = int(
                np.sum(
                    angles >= threshold
                )
            )

            print(
                f"  >= {threshold:2d}°     : "
                f"{count:3d}"
            )

    print()

    print("colors:")

    colors = Counter(
        face["color"]
        for face in result["faces"]
    )

    for color, count in colors.most_common():
        print(
            f"  {color:5d} : "
            f"{count:3d} faces"
        )

    print()

    print(
        "render types:",
        Counter(
            f["render_type"]
            for f in result["faces"]
        ),
    )

    print(
        "priorities  :",
        Counter(
            f["priority"]
            for f in result["faces"]
        ),
    )

    print()

    print(
        f"long-axis stations "
        f"({result['major_name']}):"
    )

    for coordinate, indices in result["stations"]:
        print(
            f"  {coordinate:8.2f} : "
            f"{len(indices):2d} vertices"
        )


def compare(a, b):
    print()
    print()
    print("=" * 72)
    print("490 vs 6031")
    print("=" * 72)

    print(
        f"vertex multiplier : "
        f"{len(b['points']) / len(a['points']):.2f}x"
    )

    print(
        f"face multiplier   : "
        f"{len(b['faces']) / len(a['faces']):.2f}x"
    )

    print(
        f"color count       : "
        f"{len(set(f['color'] for f in a['faces']))}"
        f" -> "
        f"{len(set(f['color'] for f in b['faces']))}"
    )

    print(
        f"stations          : "
        f"{len(a['stations'])}"
        f" -> "
        f"{len(b['stations'])}"
    )

    print(
        f"components        : "
        f"{len(a['components'])}"
        f" -> "
        f"{len(b['components'])}"
    )

    print()

    if (
        len(a["dihedrals"])
        and len(b["dihedrals"])
    ):
        print(
            "median crease angle: "
            f"{np.median(a['dihedrals']):.2f}°"
            " -> "
            f"{np.median(b['dihedrals']):.2f}°"
        )

        print(
            "strong creases >=45°: "
            f"{np.sum(a['dihedrals'] >= 45)}"
            " -> "
            f"{np.sum(b['dihedrals'] >= 45)}"
        )


def main():
    results = {
        model_id: analyze(model_id)
        for model_id in MODELS
    }

    for model_id in MODELS:
        print_model(
            results[model_id]
        )

    compare(
        results[490],
        results[6031],
    )


if __name__ == "__main__":
    main()
