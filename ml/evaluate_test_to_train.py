#!/usr/bin/env python3

import argparse
import csv
from collections import defaultdict
from pathlib import Path

import numpy as np


def load_semantics(path):
    semantics = defaultdict(set)

    with path.open(
        newline="",
        encoding="utf-8",
    ) as file:
        reader = csv.DictReader(file)

        for row in reader:
            try:
                model_id = int(row["model_id"])
            except (KeyError, ValueError, TypeError):
                continue

            kind = (
                row.get("kind", "")
                or ""
            ).strip()

            if kind:
                semantics[model_id].add(kind)

    return semantics


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--latents",
        type=Path,
        default=Path(
            "dataset/ml/latents_256.npz"
        ),
    )

    parser.add_argument(
        "--splits",
        type=Path,
        default=Path(
            "dataset/ml/splits"
        ),
    )

    parser.add_argument(
        "--references",
        type=Path,
        default=Path(
            "dataset/references.csv"
        ),
    )

    parser.add_argument(
        "--k",
        type=int,
        default=10,
    )

    args = parser.parse_args()

    data = np.load(args.latents)

    model_ids = data[
        "model_ids"
    ].astype(np.int64)

    latents = data[
        "latents"
    ].astype(np.float32)

    semantics = load_semantics(
        args.references
    )

    train_ids = set(
        np.load(
            args.splits / "train_ids.npy"
        ).astype(np.int64)
    )

    test_ids = set(
        np.load(
            args.splits / "test_ids.npy"
        ).astype(np.int64)
    )

    train_indices = np.array(
        [
            i
            for i, model_id
            in enumerate(model_ids)
            if model_id in train_ids
        ],
        dtype=np.int64,
    )

    test_indices = np.array(
        [
            i
            for i, model_id
            in enumerate(model_ids)
            if (
                model_id in test_ids
                and int(model_id)
                in semantics
            )
        ],
        dtype=np.int64,
    )

    train_vectors = latents[
        train_indices
    ]

    test_vectors = latents[
        test_indices
    ]

    # IMPORTANT:
    # normalization statistics come only
    # from the training set.
    mean = train_vectors.mean(
        axis=0,
        keepdims=True,
    )

    std = train_vectors.std(
        axis=0,
        keepdims=True,
    )

    std[std < 1e-6] = 1.0

    train_vectors = (
        train_vectors - mean
    ) / std

    test_vectors = (
        test_vectors - mean
    ) / std

    train_model_ids = model_ids[
        train_indices
    ]

    test_model_ids = model_ids[
        test_indices
    ]

    kinds = sorted(
        {
            kind
            for values
            in semantics.values()
            for kind in values
        }
    )

    baseline = {}

    for kind in kinds:
        count = sum(
            kind in semantics.get(
                int(model_id),
                set(),
            )
            for model_id
            in train_model_ids
        )

        baseline[kind] = (
            count
            / len(train_model_ids)
        )

    kind_hits = defaultdict(int)
    kind_total = defaultdict(int)

    overall_hits = 0
    overall_total = 0

    print(
        "Train candidates:",
        len(train_model_ids),
    )

    print(
        "Labeled test queries:",
        len(test_model_ids),
    )

    print(
        "k:",
        args.k,
    )

    train_norms = np.sum(
        train_vectors
        * train_vectors,
        axis=1,
    )

    batch_size = 128

    for start in range(
        0,
        len(test_vectors),
        batch_size,
    ):
        queries = test_vectors[
            start:
            start + batch_size
        ]

        distances = (
            np.sum(
                queries * queries,
                axis=1,
                keepdims=True,
            )
            + train_norms[None, :]
            - 2.0
            * (
                queries
                @ train_vectors.T
            )
        )

        distances = np.maximum(
            distances,
            0.0,
        )

        nearest = np.argpartition(
            distances,
            kth=args.k - 1,
            axis=1,
        )[:, :args.k]

        for local_index in range(
            len(queries)
        ):
            query_id = int(
                test_model_ids[
                    start + local_index
                ]
            )

            query_kinds = semantics[
                query_id
            ]

            candidate_indices = nearest[
                local_index
            ]

            candidate_distances = distances[
                local_index,
                candidate_indices,
            ]

            order = np.argsort(
                candidate_distances
            )

            candidate_indices = (
                candidate_indices[
                    order
                ]
            )

            for candidate_index in (
                candidate_indices
            ):
                neighbor_id = int(
                    train_model_ids[
                        candidate_index
                    ]
                )

                neighbor_kinds = (
                    semantics.get(
                        neighbor_id,
                        set(),
                    )
                )

                if (
                    query_kinds
                    & neighbor_kinds
                ):
                    overall_hits += 1

                overall_total += 1

                for kind in query_kinds:
                    kind_total[kind] += 1

                    if kind in neighbor_kinds:
                        kind_hits[kind] += 1

        done = min(
            start + batch_size,
            len(test_vectors),
        )

        print(
            f"\rEvaluated "
            f"{done}/"
            f"{len(test_vectors)}",
            end="",
            flush=True,
        )

    print("\n")

    overall = (
        overall_hits
        / overall_total
    )

    print(
        f"TEST → TRAIN semantic "
        f"overlap@{args.k}: "
        f"{overall * 100:.2f}%"
    )

    print()

    print(
        "Per-kind:"
    )

    print()

    for kind in kinds:
        if kind_total[kind] == 0:
            continue

        observed = (
            kind_hits[kind]
            / kind_total[kind]
        )

        random = baseline[kind]

        lift = (
            observed / random
            if random > 0
            else 0.0
        )

        query_count = sum(
            kind in semantics[
                int(model_id)
            ]
            for model_id
            in test_model_ids
        )

        print(
            f"{kind:20s} "
            f"queries={query_count:4d}  "
            f"observed="
            f"{observed * 100:6.2f}%  "
            f"random="
            f"{random * 100:6.2f}%  "
            f"lift={lift:6.2f}x"
        )


if __name__ == "__main__":
    main()
