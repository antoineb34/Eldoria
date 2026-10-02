#!/usr/bin/env python3

import argparse
import csv
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def load_references(path):
    refs = defaultdict(list)

    if not path.exists():
        return refs

    with path.open(
        newline="",
        encoding="utf-8",
    ) as file:

        reader = csv.DictReader(file)

        for row in reader:

            try:
                model_id = int(
                    row["model_id"]
                )
            except (
                KeyError,
                TypeError,
                ValueError,
            ):
                continue

            kind = (
                row.get("kind", "")
                or ""
            )

            name = (
                row.get("name", "")
                or ""
            )

            usage = (
                row.get("usage", "")
                or ""
            )

            definition_id = (
                row.get(
                    "definition_id",
                    "",
                )
                or ""
            )

            pieces = []

            if kind:
                pieces.append(kind)

            if definition_id:
                pieces.append(
                    f"#{definition_id}"
                )

            if name and name.lower() != "null":
                pieces.append(
                    f'"{name}"'
                )

            if usage:
                pieces.append(
                    f"[{usage}]"
                )

            if pieces:
                refs[model_id].append(
                    " ".join(pieces)
                )

    return refs


def describe_model(
    model_id,
    refs,
    max_refs=3,
):
    entries = refs.get(
        int(model_id),
        [],
    )

    if not entries:
        return "(no semantic reference)"

    shown = entries[:max_refs]

    result = "; ".join(shown)

    remaining = len(entries) - len(shown)

    if remaining > 0:
        result += (
            f"; +{remaining} more"
        )

    return result


def normalize_latents(latents):
    mean = latents.mean(
        axis=0,
        keepdims=True,
    )

    std = latents.std(
        axis=0,
        keepdims=True,
    )

    std[std < 1e-6] = 1.0

    return (
        latents - mean
    ) / std


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "model_id",
        type=int,
    )

    parser.add_argument(
        "--latents",
        type=Path,
        default=Path(
            "dataset/ml/latents_256.npz"
        ),
    )

    parser.add_argument(
        "--data",
        type=Path,
        default=Path("dataset/ml"),
    )

    parser.add_argument(
        "--references",
        type=Path,
        default=Path(
            "dataset/references.csv"
        ),
    )

    parser.add_argument(
        "--count",
        type=int,
        default=8,
    )

    parser.add_argument(
        "--raw-distance",
        action="store_true",
        help=(
            "Use raw latent Euclidean distance "
            "instead of z-score-normalized latent dimensions."
        ),
    )

    parser.add_argument(
        "--no-plot",
        action="store_true",
    )

    args = parser.parse_args()

    data = np.load(
        args.latents
    )

    model_ids = data[
        "model_ids"
    ].astype(np.int64)

    latents = data[
        "latents"
    ].astype(np.float32)

    matches = np.where(
        model_ids == args.model_id
    )[0]

    if len(matches) == 0:
        raise RuntimeError(
            f"Model {args.model_id} "
            f"is not in latent database."
        )

    query_index = int(
        matches[0]
    )

    if args.raw_distance:
        search_latents = latents
    else:
        search_latents = normalize_latents(
            latents
        )

    query = search_latents[
        query_index
    ]

    difference = (
        search_latents - query
    )

    distances = np.sqrt(
        np.sum(
            difference * difference,
            axis=1,
        )
    )

    # Don't return the query itself.
    distances[query_index] = np.inf

    nearest_indices = np.argsort(
        distances
    )[:args.count]

    refs = load_references(
        args.references
    )

    print()
    print(
        f"Query model {args.model_id}"
    )

    print(
        " ",
        describe_model(
            args.model_id,
            refs,
        ),
    )

    print()
    print(
        "Nearest latent neighbors:"
    )
    print()

    for rank, index in enumerate(
        nearest_indices,
        start=1,
    ):
        neighbor_id = int(
            model_ids[index]
        )

        print(
            f"{rank:2d}. "
            f"model {neighbor_id:5d}  "
            f"distance="
            f"{distances[index]:.4f}"
        )

        print(
            "    ",
            describe_model(
                neighbor_id,
                refs,
            ),
        )

    if args.no_plot:
        return

    clouds = np.load(
        args.data / "pointclouds.npy",
        mmap_mode="r",
    )

    all_model_ids = np.load(
        args.data / "model_ids.npy"
    )

    row_by_id = {
        int(model_id): row
        for row, model_id
        in enumerate(all_model_ids)
    }

    display_ids = [
        args.model_id,
        *[
            int(model_ids[index])
            for index in nearest_indices
        ],
    ]

    columns = 3

    rows = int(
        np.ceil(
            len(display_ids)
            / columns
        )
    )

    fig = plt.figure(
        figsize=(
            5 * columns,
            4 * rows,
        )
    )

    for plot_index, model_id in enumerate(
        display_ids,
        start=1,
    ):
        row = row_by_id[
            model_id
        ]

        cloud = np.asarray(
            clouds[row],
            dtype=np.float32,
        )

        # Only plot 512 so matplotlib
        # stays responsive.
        if len(cloud) > 512:
            rng = np.random.default_rng(
                model_id
            )

            indices = rng.choice(
                len(cloud),
                512,
                replace=False,
            )

            cloud = cloud[indices]

        ax = fig.add_subplot(
            rows,
            columns,
            plot_index,
            projection="3d",
        )

        ax.scatter(
            cloud[:, 0],
            cloud[:, 1],
            cloud[:, 2],
            s=4,
        )

        if plot_index == 1:
            title = (
                f"QUERY\nmodel {model_id}"
            )
        else:
            distance = distances[
                nearest_indices[
                    plot_index - 2
                ]
            ]

            title = (
                f"#{plot_index - 1} "
                f"model {model_id}\n"
                f"d={distance:.3f}"
            )

        ax.set_title(title)

        mins = cloud.min(axis=0)
        maxs = cloud.max(axis=0)

        center = (
            mins + maxs
        ) / 2.0

        radius = (
            maxs - mins
        ).max() / 2.0

        radius = max(
            float(radius),
            0.001,
        )

        ax.set_xlim(
            center[0] - radius,
            center[0] + radius,
        )

        ax.set_ylim(
            center[1] - radius,
            center[1] + radius,
        )

        ax.set_zlim(
            center[2] - radius,
            center[2] + radius,
        )

        ax.set_xticks([])
        ax.set_yticks([])
        ax.set_zticks([])

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
