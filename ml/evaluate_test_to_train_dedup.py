#!/usr/bin/env python3

import argparse
import csv
from collections import defaultdict
from pathlib import Path

import numpy as np


def load_semantics(path):
    semantics = defaultdict(set)

    with path.open(newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)

        for row in reader:
            try:
                model_id = int(row["model_id"])
            except (KeyError, ValueError, TypeError):
                continue

            kind = (row.get("kind", "") or "").strip()

            if kind:
                semantics[model_id].add(kind)

    return semantics


def load_hashes(path):
    hashes = {}

    with path.open(newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)

        for row in reader:
            try:
                model_id = int(row["model_id"])
            except (KeyError, ValueError, TypeError):
                continue

            hashes[model_id] = row["geometry_hash"]

    return hashes


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--latents",
        type=Path,
        default=Path("dataset/ml/latents_256.npz"),
    )

    parser.add_argument(
        "--splits",
        type=Path,
        default=Path("dataset/ml/splits"),
    )

    parser.add_argument(
        "--references",
        type=Path,
        default=Path("dataset/references.csv"),
    )

    parser.add_argument(
        "--hashes",
        type=Path,
        default=Path("dataset/ml/geometry_hashes.csv"),
    )

    parser.add_argument(
        "--k",
        type=int,
        default=10,
    )

    args = parser.parse_args()

    data = np.load(args.latents)

    model_ids = data["model_ids"].astype(np.int64)
    latents = data["latents"].astype(np.float32)

    semantics = load_semantics(args.references)
    geometry_hashes = load_hashes(args.hashes)

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
            index
            for index, model_id in enumerate(model_ids)
            if model_id in train_ids
        ],
        dtype=np.int64,
    )

    test_indices = np.array(
        [
            index
            for index, model_id in enumerate(model_ids)
            if (
                model_id in test_ids
                and int(model_id) in semantics
            )
        ],
        dtype=np.int64,
    )

    train_model_ids = model_ids[train_indices]
    test_model_ids = model_ids[test_indices]

    train_vectors = latents[train_indices]
    test_vectors = latents[test_indices]

    # Normalize using TRAIN statistics only.
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

    train_norms = np.sum(
        train_vectors * train_vectors,
        axis=1,
    )

    kinds = sorted(
        {
            kind
            for values in semantics.values()
            for kind in values
        }
    )

    kind_hits = defaultdict(int)
    kind_total = defaultdict(int)

    # Instead of one global random baseline,
    # calculate the baseline after duplicate
    # candidates have been removed for each query.
    baseline_sum = defaultdict(float)
    baseline_queries = defaultdict(int)

    overall_hits = 0
    overall_total = 0

    queries_with_hash = 0
    queries_with_duplicate_train_models = 0
    total_banned_candidates = 0

    query_count_by_kind = defaultdict(int)

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

    print()

    for query_number, (
        query_vector,
        query_id,
    ) in enumerate(
        zip(
            test_vectors,
            test_model_ids,
        ),
        start=1,
    ):
        query_id = int(query_id)

        distances = (
            np.sum(
                query_vector * query_vector
            )
            + train_norms
            - 2.0
            * (
                train_vectors
                @ query_vector
            )
        )

        distances = np.maximum(
            distances,
            0.0,
        )

        allowed = np.ones(
            len(train_model_ids),
            dtype=bool,
        )

        query_hash = geometry_hashes.get(
            query_id
        )

        if query_hash is not None:
            queries_with_hash += 1

            banned = np.array(
                [
                    (
                        geometry_hashes.get(
                            int(train_id)
                        )
                        == query_hash
                    )
                    for train_id
                    in train_model_ids
                ],
                dtype=bool,
            )

            banned_count = int(
                banned.sum()
            )

            if banned_count > 0:
                queries_with_duplicate_train_models += 1
                total_banned_candidates += banned_count

                allowed[banned] = False
                distances[banned] = np.inf

        allowed_count = int(
            allowed.sum()
        )

        if allowed_count < args.k:
            raise RuntimeError(
                f"Not enough candidates for "
                f"model {query_id}"
            )

        query_kinds = semantics[
            query_id
        ]

        # Calculate random baseline using only
        # train candidates that remain allowed
        # for THIS query.
        allowed_ids = train_model_ids[
            allowed
        ]

        for kind in query_kinds:
            query_count_by_kind[kind] += 1

            same_kind_allowed = sum(
                kind in semantics.get(
                    int(candidate_id),
                    set(),
                )
                for candidate_id
                in allowed_ids
            )

            baseline_sum[kind] += (
                same_kind_allowed
                / allowed_count
            )

            baseline_queries[kind] += 1

        nearest = np.argpartition(
            distances,
            kth=args.k - 1,
        )[:args.k]

        nearest = nearest[
            np.argsort(
                distances[nearest]
            )
        ]

        for candidate_index in nearest:
            neighbor_id = int(
                train_model_ids[
                    candidate_index
                ]
            )

            neighbor_kinds = semantics.get(
                neighbor_id,
                set(),
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

        print(
            f"\rEvaluated "
            f"{query_number}/"
            f"{len(test_model_ids)}",
            end="",
            flush=True,
        )

    print("\n")

    overall = (
        overall_hits
        / overall_total
    )

    print(
        "Queries with geometry hash:",
        queries_with_hash,
    )

    print(
        "Queries where train duplicates "
        "were banned:",
        queries_with_duplicate_train_models,
    )

    print(
        "Total duplicate train candidates "
        "banned:",
        total_banned_candidates,
    )

    print()

    print(
        f"DEDUP TEST → TRAIN semantic "
        f"overlap@{args.k}: "
        f"{overall * 100:.2f}%"
    )

    print()
    print("Per-kind:")
    print()

    for kind in kinds:
        if kind_total[kind] == 0:
            continue

        observed = (
            kind_hits[kind]
            / kind_total[kind]
        )

        random = (
            baseline_sum[kind]
            / baseline_queries[kind]
        )

        lift = (
            observed / random
            if random > 0
            else 0.0
        )

        print(
            f"{kind:20s} "
            f"queries="
            f"{query_count_by_kind[kind]:4d}  "
            f"observed="
            f"{observed * 100:6.2f}%  "
            f"random="
            f"{random * 100:6.2f}%  "
            f"lift="
            f"{lift:6.2f}x"
        )


if __name__ == "__main__":
    main()
