#!/usr/bin/env python3

import argparse
import csv
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt
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
            except (
                KeyError,
                TypeError,
                ValueError,
            ):
                continue

            kind = (
                row.get("kind", "")
                or ""
            ).strip()

            if kind:
                semantics[model_id].add(kind)

    return semantics


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
        "--latents",
        type=Path,
        default=Path(
            "dataset/ml/latents_256.npz"
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
        "--output",
        type=Path,
        default=Path(
            "dataset/ml/semantic_eval"
        ),
    )

    parser.add_argument(
        "--k",
        type=int,
        default=10,
    )

    parser.add_argument(
        "--batch-size",
        type=int,
        default=256,
    )

    parser.add_argument(
        "--raw-distance",
        action="store_true",
    )

    args = parser.parse_args()

    args.output.mkdir(
        parents=True,
        exist_ok=True,
    )

    data = np.load(
        args.latents
    )

    model_ids = data[
        "model_ids"
    ].astype(np.int64)

    latents = data[
        "latents"
    ].astype(np.float32)

    semantics = load_semantics(
        args.references
    )

    if args.raw_distance:
        vectors = latents
        mode = "raw"
    else:
        vectors = normalize_latents(
            latents
        )
        mode = "zscore"

    # Squared vector norms for fast:
    #
    # ||a-b||² =
    # ||a||² + ||b||² - 2(a·b)
    norms = np.sum(
        vectors * vectors,
        axis=1,
    )

    id_to_index = {
        int(model_id): index
        for index, model_id
        in enumerate(model_ids)
    }

    labeled_indices = np.array(
        [
            index
            for index, model_id
            in enumerate(model_ids)
            if int(model_id) in semantics
        ],
        dtype=np.int64,
    )

    all_kinds = sorted(
        {
            kind
            for values in semantics.values()
            for kind in values
        }
    )

    print(
        "Models in latent database:",
        len(model_ids),
    )

    print(
        "Models with semantic labels:",
        len(labeled_indices),
    )

    print(
        "Kinds:",
        ", ".join(all_kinds),
    )

    print(
        "k:",
        args.k,
    )

    print(
        "Distance mode:",
        mode,
    )

    print()

    kind_population = {
        kind: sum(
            1
            for model_id in model_ids
            if kind in semantics.get(
                int(model_id),
                set(),
            )
        )
        for kind in all_kinds
    }

    random_baseline = {
        kind: (
            kind_population[kind]
            / len(model_ids)
        )
        for kind in all_kinds
    }

    kind_hits = defaultdict(int)
    kind_total = defaultdict(int)

    confusion = {
        query_kind: defaultdict(int)
        for query_kind in all_kinds
    }

    overall_hits = 0
    overall_total = 0

    labeled_neighbor_count = 0
    neighbor_total = 0

    query_results = []

    total_queries = len(
        labeled_indices
    )

    for batch_start in range(
        0,
        total_queries,
        args.batch_size,
    ):
        batch_indices = labeled_indices[
            batch_start:
            batch_start + args.batch_size
        ]

        queries = vectors[
            batch_indices
        ]

        distances = (
            np.sum(
                queries * queries,
                axis=1,
                keepdims=True,
            )
            + norms[None, :]
            - 2.0 * (
                queries
                @ vectors.T
            )
        )

        # Numerical precision can produce
        # tiny negatives.
        distances = np.maximum(
            distances,
            0.0,
        )

        for local_index, query_index in enumerate(
            batch_indices
        ):
            distances[
                local_index,
                query_index,
            ] = np.inf

        # We only need k closest,
        # not a complete sort.
        nearest_unsorted = np.argpartition(
            distances,
            kth=args.k - 1,
            axis=1,
        )[:, :args.k]

        for local_index, query_index in enumerate(
            batch_indices
        ):
            candidate_indices = (
                nearest_unsorted[
                    local_index
                ]
            )

            candidate_distances = (
                distances[
                    local_index,
                    candidate_indices,
                ]
            )

            order = np.argsort(
                candidate_distances
            )

            nearest = candidate_indices[
                order
            ]

            query_id = int(
                model_ids[
                    query_index
                ]
            )

            query_kinds = semantics[
                query_id
            ]

            query_overlap_hits = 0

            for neighbor_index in nearest:
                neighbor_id = int(
                    model_ids[
                        neighbor_index
                    ]
                )

                neighbor_kinds = semantics.get(
                    neighbor_id,
                    set(),
                )

                neighbor_total += 1

                if neighbor_kinds:
                    labeled_neighbor_count += 1

                overlap = (
                    query_kinds
                    & neighbor_kinds
                )

                if overlap:
                    overall_hits += 1
                    query_overlap_hits += 1

                overall_total += 1

                for query_kind in query_kinds:
                    kind_total[
                        query_kind
                    ] += 1

                    if (
                        query_kind
                        in neighbor_kinds
                    ):
                        kind_hits[
                            query_kind
                        ] += 1

                    if neighbor_kinds:
                        for neighbor_kind in (
                            neighbor_kinds
                        ):
                            confusion[
                                query_kind
                            ][
                                neighbor_kind
                            ] += 1
                    else:
                        confusion[
                            query_kind
                        ][
                            "(unlabeled)"
                        ] += 1

            query_results.append(
                (
                    query_id,
                    "|".join(
                        sorted(
                            query_kinds
                        )
                    ),
                    query_overlap_hits
                    / args.k,
                )
            )

        done = min(
            batch_start
            + args.batch_size,
            total_queries,
        )

        print(
            f"\rEvaluated "
            f"{done}/{total_queries}",
            end="",
            flush=True,
        )

    print()
    print()

    overall_precision = (
        overall_hits
        / overall_total
    )

    label_coverage = (
        labeled_neighbor_count
        / neighbor_total
    )

    print(
        "Overall semantic overlap "
        f"precision@{args.k}: "
        f"{overall_precision:.4f} "
        f"({overall_precision * 100:.2f}%)"
    )

    print(
        "Neighbor semantic coverage: "
        f"{label_coverage:.4f} "
        f"({label_coverage * 100:.2f}%)"
    )

    print()
    print(
        "Per-kind same-kind "
        f"precision@{args.k}:"
    )
    print()

    rows = []

    for kind in all_kinds:
        observed = (
            kind_hits[kind]
            / kind_total[kind]
            if kind_total[kind]
            else 0.0
        )

        baseline = random_baseline[
            kind
        ]

        lift = (
            observed / baseline
            if baseline > 0
            else 0.0
        )

        query_count = sum(
            1
            for index in labeled_indices
            if kind in semantics[
                int(model_ids[index])
            ]
        )

        rows.append(
            (
                kind,
                query_count,
                observed,
                baseline,
                lift,
            )
        )

    rows.sort(
        key=lambda row: row[4],
        reverse=True,
    )

    for (
        kind,
        query_count,
        observed,
        baseline,
        lift,
    ) in rows:
        print(
            f"{kind:20s} "
            f"queries={query_count:5d}  "
            f"observed={observed * 100:6.2f}%  "
            f"random={baseline * 100:6.2f}%  "
            f"lift={lift:6.2f}x"
        )

    summary_path = (
        args.output
        / "semantic_precision.csv"
    )

    with summary_path.open(
        "w",
        newline="",
    ) as file:
        writer = csv.writer(file)

        writer.writerow(
            [
                "kind",
                "query_count",
                f"precision_at_{args.k}",
                "random_baseline",
                "lift",
            ]
        )

        for row in rows:
            writer.writerow(row)

    query_path = (
        args.output
        / "query_precision.csv"
    )

    with query_path.open(
        "w",
        newline="",
    ) as file:
        writer = csv.writer(file)

        writer.writerow(
            [
                "model_id",
                "kinds",
                f"semantic_overlap_at_{args.k}",
            ]
        )

        writer.writerows(
            query_results
        )

    confusion_kinds = (
        all_kinds
        + ["(unlabeled)"]
    )

    confusion_path = (
        args.output
        / "semantic_confusion.csv"
    )

    with confusion_path.open(
        "w",
        newline="",
    ) as file:
        writer = csv.writer(file)

        writer.writerow(
            ["query_kind"]
            + confusion_kinds
        )

        for query_kind in all_kinds:
            writer.writerow(
                [query_kind]
                + [
                    confusion[
                        query_kind
                    ][neighbor_kind]
                    for neighbor_kind
                    in confusion_kinds
                ]
            )

    # Plot observed vs random baseline.
    kinds = [
        row[0]
        for row in rows
    ]

    observed_values = np.array(
        [
            row[2]
            for row in rows
        ]
    )

    baseline_values = np.array(
        [
            row[3]
            for row in rows
        ]
    )

    positions = np.arange(
        len(kinds)
    )

    width = 0.38

    fig, ax = plt.subplots(
        figsize=(12, 6)
    )

    ax.bar(
        positions - width / 2,
        observed_values,
        width,
        label="Latent neighbors",
    )

    ax.bar(
        positions + width / 2,
        baseline_values,
        width,
        label="Random baseline",
    )

    ax.set_ylabel(
        f"Same-kind precision@{args.k}"
    )

    ax.set_title(
        "Semantic structure in "
        "geometry-only latent space"
    )

    ax.set_xticks(
        positions
    )

    ax.set_xticklabels(
        kinds,
        rotation=35,
        ha="right",
    )

    ax.legend()

    fig.tight_layout()

    plot_path = (
        args.output
        / "semantic_precision.png"
    )

    fig.savefig(
        plot_path,
        dpi=160,
    )

    print()
    print(
        "Saved:",
        summary_path,
    )

    print(
        "Saved:",
        query_path,
    )

    print(
        "Saved:",
        confusion_path,
    )

    print(
        "Saved:",
        plot_path,
    )


if __name__ == "__main__":
    main()
