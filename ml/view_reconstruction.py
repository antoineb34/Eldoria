#!/usr/bin/env python3

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import torch

from train_autoencoder import Autoencoder


def equal_axes(ax, points):
    mins = points.min(axis=0)
    maxs = points.max(axis=0)

    center = (mins + maxs) / 2.0
    radius = (maxs - mins).max() / 2.0

    if radius <= 0:
        radius = 1.0

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


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--data",
        type=Path,
        default=Path("dataset/ml"),
    )

    parser.add_argument(
        "--checkpoint",
        type=Path,
        default=Path(
            "dataset/ml/autoencoder/best.pt"
        ),
    )

    parser.add_argument(
        "--model-id",
        type=int,
        default=None,
    )

    args = parser.parse_args()

    device = torch.device("cpu")

    checkpoint = torch.load(
        args.checkpoint,
        map_location=device,
        weights_only=False,
    )

    point_count = checkpoint[
        "point_count"
    ]

    latent_dim = checkpoint[
        "latent_dim"
    ]

    model = Autoencoder(
        latent_dim=latent_dim,
        point_count=point_count,
    )

    model.load_state_dict(
        checkpoint["model_state"]
    )

    model.eval()

    clouds = np.load(
        args.data / "pointclouds.npy",
        mmap_mode="r",
    )

    model_ids = np.load(
        args.data / "model_ids.npy"
    )

    valid = np.load(
        args.data / "valid.npy"
    )

    test_ids = np.load(
        args.data
        / "splits"
        / "test_ids.npy"
    )

    if args.model_id is None:
        rng = np.random.default_rng()

        model_id = int(
            rng.choice(test_ids)
        )
    else:
        model_id = args.model_id

    matches = np.where(
        model_ids == model_id
    )[0]

    if len(matches) == 0:
        raise RuntimeError(
            f"Unknown model ID {model_id}"
        )

    row = int(matches[0])

    if not valid[row]:
        raise RuntimeError(
            f"Model {model_id} is not valid"
        )

    cloud = np.asarray(
        clouds[row],
        dtype=np.float32,
    )

    # Match the deterministic test-set
    # sampling used during training.
    rng = np.random.default_rng(
        model_id
    )

    indices = rng.choice(
        len(cloud),
        point_count,
        replace=False,
    )

    original = np.array(
        cloud[indices],
        copy=True,
    )

    tensor = torch.from_numpy(
        original
    ).unsqueeze(0)

    with torch.no_grad():
        reconstructed, latent = model(
            tensor
        )

    reconstructed = (
        reconstructed[0]
        .cpu()
        .numpy()
    )

    latent = (
        latent[0]
        .cpu()
        .numpy()
    )

    print("Model ID:", model_id)
    print("Points:", point_count)
    print("Latent dimensions:", len(latent))

    print()
    print(
        "Latent first 10:",
        latent[:10],
    )

    combined = np.concatenate(
        [
            original,
            reconstructed,
        ],
        axis=0,
    )

    fig = plt.figure(
        figsize=(12, 6)
    )

    original_ax = fig.add_subplot(
        1,
        2,
        1,
        projection="3d",
    )

    reconstructed_ax = (
        fig.add_subplot(
            1,
            2,
            2,
            projection="3d",
        )
    )

    original_ax.scatter(
        original[:, 0],
        original[:, 1],
        original[:, 2],
        s=8,
    )

    reconstructed_ax.scatter(
        reconstructed[:, 0],
        reconstructed[:, 1],
        reconstructed[:, 2],
        s=8,
    )

    original_ax.set_title(
        f"Original - model {model_id}"
    )

    reconstructed_ax.set_title(
        "AI reconstruction"
    )

    equal_axes(
        original_ax,
        combined,
    )

    equal_axes(
        reconstructed_ax,
        combined,
    )

    original_ax.set_xlabel("X")
    original_ax.set_ylabel("Y")
    original_ax.set_zlabel("Z")

    reconstructed_ax.set_xlabel("X")
    reconstructed_ax.set_ylabel("Y")
    reconstructed_ax.set_zlabel("Z")

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
