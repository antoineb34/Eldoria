#!/usr/bin/env python3

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import torch

from train_autoencoder import Autoencoder


def load_model(checkpoint_path):
    checkpoint = torch.load(
        checkpoint_path,
        map_location="cpu",
        weights_only=False,
    )

    point_count = checkpoint.get(
        "point_count",
        256,
    )

    latent_dim = checkpoint.get(
        "latent_dim",
        128,
    )

    model = Autoencoder(
        point_count=point_count,
        latent_dim=latent_dim,
    )

    if "model_state_dict" in checkpoint:
        state = checkpoint[
            "model_state_dict"
        ]

    elif "model_state" in checkpoint:
        state = checkpoint[
            "model_state"
        ]

    elif "state_dict" in checkpoint:
        state = checkpoint[
            "state_dict"
        ]

    else:
        # In case checkpoint itself is
        # just the state dict.
        state = checkpoint

    model.load_state_dict(state)

    model.eval()

    return (
        model,
        point_count,
        latent_dim,
    )


def get_cloud(
    target_model_id,
    model_ids,
    clouds,
    point_count,
):
    matches = np.where(
        model_ids == target_model_id
    )[0]

    if len(matches) == 0:
        raise ValueError(
            f"Model {target_model_id} "
            f"not found"
        )

    row = int(matches[0])

    cloud = clouds[row]

    rng = np.random.default_rng(
        target_model_id
    )

    indices = rng.choice(
        len(cloud),
        size=point_count,
        replace=False,
    )

    return cloud[
        indices
    ].astype(np.float32)


def encode(model, cloud):
    tensor = torch.from_numpy(
        cloud
    ).unsqueeze(0)

    # Encoder expects [B, N, 3].
    # It performs the transpose to
    # [B, 3, N] internally.
    with torch.no_grad():
        latent = model.encoder(
            tensor
        )

    return latent


def decode(
    model,
    latent,
    point_count,
):
    with torch.no_grad():
        result = model.decoder(
            latent
        )

    result = result.reshape(
        -1,
        point_count,
        3,
    )

    return (
        result[0]
        .cpu()
        .numpy()
    )


def save_xyz(path, cloud):
    np.savetxt(
        path,
        cloud,
        fmt="%.7f",
    )


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "model_a",
        type=int,
    )

    parser.add_argument(
        "model_b",
        type=int,
    )

    parser.add_argument(
        "--checkpoint",
        type=Path,
        default=Path(
            "dataset/ml/"
            "autoencoder_256_grouped/"
            "best.pt"
        ),
    )

    parser.add_argument(
        "--data",
        type=Path,
        default=Path(
            "dataset/ml"
        ),
    )

    parser.add_argument(
        "--steps",
        type=int,
        default=5,
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path(
            "dataset/ml/interpolations"
        ),
    )

    args = parser.parse_args()

    if args.steps < 3:
        raise ValueError(
            "--steps must be at least 3"
        )

    model_ids = np.load(
        args.data / "model_ids.npy"
    )

    clouds = np.load(
        args.data / "pointclouds.npy",
        mmap_mode="r",
    )

    (
        model,
        point_count,
        latent_dim,
    ) = load_model(
        args.checkpoint
    )

    cloud_a = get_cloud(
        args.model_a,
        model_ids,
        clouds,
        point_count,
    )

    cloud_b = get_cloud(
        args.model_b,
        model_ids,
        clouds,
        point_count,
    )

    z_a = encode(
        model,
        cloud_a,
    )

    z_b = encode(
        model,
        cloud_b,
    )

    print(
        "Checkpoint:",
        args.checkpoint,
    )

    print(
        "Model A:",
        args.model_a,
    )

    print(
        "Model B:",
        args.model_b,
    )

    print(
        "Points:",
        point_count,
    )

    print(
        "Latent dimensions:",
        latent_dim,
    )

    latent_distance = torch.linalg.vector_norm(
        z_b - z_a
    ).item()

    print(
        "Latent distance:",
        f"{latent_distance:.6f}",
    )

    alphas = np.linspace(
        0.0,
        1.0,
        args.steps,
    )

    generated = []

    args.output.mkdir(
        parents=True,
        exist_ok=True,
    )

    run_dir = (
        args.output
        / f"{args.model_a}_to_{args.model_b}"
    )

    run_dir.mkdir(
        parents=True,
        exist_ok=True,
    )

    for index, alpha in enumerate(
        alphas
    ):
        latent = (
            (1.0 - alpha) * z_a
            + alpha * z_b
        )

        cloud = decode(
            model,
            latent,
            point_count,
        )

        generated.append(
            cloud
        )

        np.save(
            run_dir
            / f"step_{index:02d}.npy",
            cloud,
        )

        save_xyz(
            run_dir
            / f"step_{index:02d}.xyz",
            cloud,
        )

        print(
            f"step {index}: "
            f"alpha={alpha:.3f}"
        )

    # --------------------------------
    # Display the decoded journey.
    # --------------------------------

    figure = plt.figure(
        figsize=(
            4 * args.steps,
            4,
        )
    )

    for index, (
        alpha,
        cloud,
    ) in enumerate(
        zip(
            alphas,
            generated,
        ),
        start=1,
    ):
        ax = figure.add_subplot(
            1,
            args.steps,
            index,
            projection="3d",
        )

        ax.scatter(
            cloud[:, 0],
            cloud[:, 1],
            cloud[:, 2],
            s=7,
        )

        if alpha == 0.0:
            title = (
                f"A: {args.model_a}\n"
                f"α={alpha:.2f}"
            )

        elif alpha == 1.0:
            title = (
                f"B: {args.model_b}\n"
                f"α={alpha:.2f}"
            )

        else:
            title = (
                "GENERATED\n"
                f"α={alpha:.2f}"
            )

        ax.set_title(
            title
        )

        ax.set_xlim(
            -1.05,
            1.05,
        )

        ax.set_ylim(
            -1.05,
            1.05,
        )

        ax.set_zlim(
            -1.05,
            1.05,
        )

        ax.set_box_aspect(
            (1, 1, 1)
        )

        ax.set_xticks([])
        ax.set_yticks([])
        ax.set_zticks([])

    figure.suptitle(
        f"Latent interpolation: "
        f"{args.model_a} → "
        f"{args.model_b}"
    )

    figure.tight_layout()

    image_path = (
        run_dir
        / "interpolation.png"
    )

    figure.savefig(
        image_path,
        dpi=180,
    )

    print()
    print(
        "Saved generation to:",
        run_dir,
    )

    print(
        "Image:",
        image_path,
    )

    plt.show()


if __name__ == "__main__":
    main()
