#!/usr/bin/env python3

import argparse
from pathlib import Path

import numpy as np
import torch

from train_autoencoder import Autoencoder


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
            "dataset/ml/autoencoder_256/best.pt"
        ),
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path(
            "dataset/ml/latents_256.npz"
        ),
    )

    parser.add_argument(
        "--batch-size",
        type=int,
        default=64,
    )

    args = parser.parse_args()

    device = torch.device("cpu")

    checkpoint = torch.load(
        args.checkpoint,
        map_location=device,
        weights_only=False,
    )

    point_count = checkpoint["point_count"]
    latent_dim = checkpoint["latent_dim"]

    print("Checkpoint:", args.checkpoint)
    print("Point count:", point_count)
    print("Latent dimensions:", latent_dim)

    model = Autoencoder(
        latent_dim=latent_dim,
        point_count=point_count,
    )

    model.load_state_dict(
        checkpoint["model_state"]
    )

    model.to(device)
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

    valid_rows = np.where(valid)[0]

    valid_ids = model_ids[
        valid_rows
    ].astype(np.uint16)

    latents = np.empty(
        (
            len(valid_rows),
            latent_dim,
        ),
        dtype=np.float32,
    )

    print("Encoding models:", len(valid_rows))

    with torch.no_grad():

        for start in range(
            0,
            len(valid_rows),
            args.batch_size,
        ):
            end = min(
                start + args.batch_size,
                len(valid_rows),
            )

            batch_points = []

            for row in valid_rows[start:end]:

                model_id = int(
                    model_ids[row]
                )

                cloud = np.asarray(
                    clouds[row],
                    dtype=np.float32,
                )

                # Deterministic subset so every run
                # produces the same latent vector.
                rng = np.random.default_rng(
                    model_id
                )

                indices = rng.choice(
                    len(cloud),
                    point_count,
                    replace=False,
                )

                points = np.array(
                    cloud[indices],
                    copy=True,
                    dtype=np.float32,
                )

                batch_points.append(points)

            tensor = torch.from_numpy(
                np.stack(batch_points)
            ).to(device)

            encoded = model.encoder(
                tensor
            )

            latents[start:end] = (
                encoded
                .cpu()
                .numpy()
            )

            print(
                f"\rEncoded "
                f"{end}/{len(valid_rows)}",
                end="",
                flush=True,
            )

    print()

    args.output.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    np.savez(
        args.output,
        model_ids=valid_ids,
        latents=latents,
    )

    print()
    print("Saved:", args.output)
    print("Model IDs:", valid_ids.shape)
    print("Latents:", latents.shape)


if __name__ == "__main__":
    main()
