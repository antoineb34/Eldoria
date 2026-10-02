#!/usr/bin/env python3

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--input",
        type=Path,
        default=Path(
            "dataset/ml/semantic_eval"
        ),
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path(
            "dataset/ml/semantic_eval/"
            "semantic_confusion.png"
        ),
    )

    args = parser.parse_args()

    precision_path = (
        args.input
        / "semantic_precision.csv"
    )

    confusion_path = (
        args.input
        / "semantic_confusion.csv"
    )

    # ---------------------------------
    # Load query counts + infer k.
    # ---------------------------------

    query_counts = {}

    with precision_path.open(
        newline="",
        encoding="utf-8",
    ) as file:
        reader = csv.DictReader(file)

        precision_column = next(
            column
            for column in reader.fieldnames
            if column.startswith(
                "precision_at_"
            )
        )

        k = int(
            precision_column.split("_")[-1]
        )

        for row in reader:
            query_counts[
                row["kind"]
            ] = int(
                row["query_count"]
            )

    # ---------------------------------
    # Load raw confusion counts.
    # ---------------------------------

    with confusion_path.open(
        newline="",
        encoding="utf-8",
    ) as file:
        reader = csv.DictReader(file)

        neighbor_kinds = [
            column
            for column in reader.fieldnames
            if column != "query_kind"
        ]

        query_kinds = []
        counts = []

        for row in reader:
            query_kind = row[
                "query_kind"
            ]

            query_kinds.append(
                query_kind
            )

            counts.append(
                [
                    int(
                        row[
                            neighbor_kind
                        ]
                    )
                    for neighbor_kind
                    in neighbor_kinds
                ]
            )

    counts = np.asarray(
        counts,
        dtype=np.float64,
    )

    # ---------------------------------
    # Convert counts into:
    #
    # percent of neighbor slots
    # carrying each semantic label.
    #
    # Because labels are multi-label,
    # rows can total >100%.
    # ---------------------------------

    percentages = np.zeros_like(
        counts
    )

    for row_index, query_kind in enumerate(
        query_kinds
    ):
        denominator = (
            query_counts[
                query_kind
            ]
            * k
        )

        if denominator > 0:
            percentages[
                row_index
            ] = (
                counts[
                    row_index
                ]
                / denominator
                * 100.0
            )

    # ---------------------------------
    # Terminal table.
    # ---------------------------------

    print()
    print(
        f"Semantic neighbor matrix "
        f"(top {k})"
    )

    print(
        "Values = % of neighbor slots "
        "containing that label."
    )

    print(
        "Rows may exceed 100% because "
        "models can have multiple labels."
    )

    print()

    header = (
        f"{'query':18s}"
        + "".join(
            f"{kind[:10]:>11s}"
            for kind in neighbor_kinds
        )
    )

    print(header)
    print(
        "-" * len(header)
    )

    for row_index, query_kind in enumerate(
        query_kinds
    ):
        values = "".join(
            f"{value:10.1f}%"
            for value in percentages[
                row_index
            ]
        )

        print(
            f"{query_kind:18s}"
            + values
        )

    # ---------------------------------
    # Strongest cross-category
    # associations.
    # ---------------------------------

    print()
    print(
        "Strongest non-self "
        "associations:"
    )
    print()

    associations = []

    for row_index, query_kind in enumerate(
        query_kinds
    ):
        for column_index, neighbor_kind in enumerate(
            neighbor_kinds
        ):
            if neighbor_kind in (
                query_kind,
                "(unlabeled)",
            ):
                continue

            associations.append(
                (
                    percentages[
                        row_index,
                        column_index,
                    ],
                    query_kind,
                    neighbor_kind,
                )
            )

    associations.sort(
        reverse=True
    )

    for (
        value,
        query_kind,
        neighbor_kind,
    ) in associations[:15]:
        print(
            f"{query_kind:18s}"
            f" -> "
            f"{neighbor_kind:18s}"
            f" {value:6.2f}%"
        )

    # ---------------------------------
    # Heatmap.
    # ---------------------------------

    fig, ax = plt.subplots(
        figsize=(11, 7)
    )

    image = ax.imshow(
        percentages,
        aspect="auto",
    )

    ax.set_xticks(
        np.arange(
            len(neighbor_kinds)
        )
    )

    ax.set_xticklabels(
        neighbor_kinds,
        rotation=40,
        ha="right",
    )

    ax.set_yticks(
        np.arange(
            len(query_kinds)
        )
    )

    ax.set_yticklabels(
        query_kinds
    )

    ax.set_xlabel(
        "Neighbor semantic kind"
    )

    ax.set_ylabel(
        "Query semantic kind"
    )

    ax.set_title(
        f"Latent-space semantic "
        f"neighbors @ {k}"
    )

    for row in range(
        len(query_kinds)
    ):
        for column in range(
            len(neighbor_kinds)
        ):
            ax.text(
                column,
                row,
                f"{percentages[row, column]:.1f}",
                ha="center",
                va="center",
                fontsize=8,
            )

    fig.colorbar(
        image,
        ax=ax,
        label="% of neighbor slots",
    )

    fig.tight_layout()

    fig.savefig(
        args.output,
        dpi=180,
    )

    print()
    print(
        "Saved:",
        args.output,
    )


if __name__ == "__main__":
    main()
