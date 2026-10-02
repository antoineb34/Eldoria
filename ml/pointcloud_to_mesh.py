#!/usr/bin/env python3

import argparse
from pathlib import Path

import numpy as np
import open3d as o3d


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "input",
        type=Path,
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=None,
    )

    args = parser.parse_args()

    points = np.load(
        args.input
    ).astype(np.float64)

    print(
        "Loaded:",
        args.input,
    )

    print(
        "Points:",
        len(points),
    )

    cloud = o3d.geometry.PointCloud()

    cloud.points = o3d.utility.Vector3dVector(
        points
    )

    # --------------------------------
    # Estimate surface normals.
    # --------------------------------

    cloud.estimate_normals(
        search_param=
        o3d.geometry.KDTreeSearchParamHybrid(
            radius=0.20,
            max_nn=30,
        )
    )

    cloud.orient_normals_consistent_tangent_plane(
        20
    )

    # --------------------------------
    # Estimate typical point spacing.
    # --------------------------------

    distances = np.asarray(
        cloud.compute_nearest_neighbor_distance()
    )

    spacing = float(
        np.median(distances)
    )

    print(
        "Median point spacing:",
        spacing,
    )

    # Ball Pivoting generally benefits
    # from several radius choices.
    radii = o3d.utility.DoubleVector(
        [
            spacing * 1.5,
            spacing * 2.0,
            spacing * 3.0,
        ]
    )

    # --------------------------------
    # Reconstruct triangles.
    # --------------------------------

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
    mesh.remove_non_manifold_edges()

    mesh.compute_vertex_normals()

    vertices = np.asarray(
        mesh.vertices
    )

    triangles = np.asarray(
        mesh.triangles
    )

    print(
        "Mesh vertices:",
        len(vertices),
    )

    print(
        "Triangles:",
        len(triangles),
    )

    if args.output is None:
        output = (
            args.input.parent
            / (
                args.input.stem
                + "_mesh.ply"
            )
        )
    else:
        output = args.output

    ok = o3d.io.write_triangle_mesh(
        str(output),
        mesh,
    )

    if not ok:
        raise RuntimeError(
            "Failed to save mesh"
        )

    print(
        "Saved:",
        output,
    )

    # --------------------------------
    # Display solid mesh.
    # --------------------------------

    o3d.visualization.draw_geometries(
        [mesh],
        window_name="Generated Eldoria Mesh",
        mesh_show_back_face=True,
    )


if __name__ == "__main__":
    main()
