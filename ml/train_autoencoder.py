#!/usr/bin/env python3

import argparse
import csv
import random
from pathlib import Path

import numpy as np
import torch
from torch import nn
from torch.utils.data import Dataset, DataLoader


class PointCloudDataset(Dataset):
    def __init__(
        self,
        root,
        ids_file,
        points=128,
        random_subset=False,
    ):
        root = Path(root)

        self.clouds = np.load(
            root / "pointclouds.npy",
            mmap_mode="r",
        )

        model_ids = np.load(
            root / "model_ids.npy"
        )

        valid = np.load(
            root / "valid.npy"
        )

        self.ids = np.load(
            ids_file
        ).astype(np.int64)

        self.row_by_id = {
            int(model_id): row
            for row, model_id
            in enumerate(model_ids)
            if valid[row]
        }

        self.points = points
        self.random_subset = random_subset

    def __len__(self):
        return len(self.ids)

    def __getitem__(self, index):
        model_id = int(self.ids[index])

        row = self.row_by_id[
            model_id
        ]

        cloud = np.asarray(
            self.clouds[row],
            dtype=np.float32,
        )

        if self.points < len(cloud):

            if self.random_subset:
                indices = np.random.choice(
                    len(cloud),
                    self.points,
                    replace=False,
                )
            else:
                rng = np.random.default_rng(
                    model_id
                )

                indices = rng.choice(
                    len(cloud),
                    self.points,
                    replace=False,
                )

            cloud = cloud[indices]

        cloud = np.array(
            cloud,
            copy=True,
            dtype=np.float32,
        )

        return torch.from_numpy(cloud)


class PointNetEncoder(nn.Module):
    def __init__(self, latent_dim=128):
        super().__init__()

        self.network = nn.Sequential(
            nn.Conv1d(3, 64, 1),
            nn.BatchNorm1d(64),
            nn.ReLU(),

            nn.Conv1d(64, 128, 1),
            nn.BatchNorm1d(128),
            nn.ReLU(),

            nn.Conv1d(128, 256, 1),
            nn.BatchNorm1d(256),
            nn.ReLU(),

            nn.Conv1d(
                256,
                latent_dim,
                1,
            ),
        )

    def forward(self, points):
        # [B, N, 3] -> [B, 3, N]
        x = points.transpose(1, 2)

        x = self.network(x)

        # Collapse all points into one latent vector.
        return torch.max(
            x,
            dim=2,
        ).values


class PointCloudDecoder(nn.Module):
    def __init__(
        self,
        latent_dim=128,
        point_count=128,
    ):
        super().__init__()

        self.point_count = point_count

        self.network = nn.Sequential(
            nn.Linear(
                latent_dim,
                256,
            ),
            nn.ReLU(),

            nn.Linear(
                256,
                512,
            ),
            nn.ReLU(),

            nn.Linear(
                512,
                point_count * 3,
            ),

            nn.Tanh(),
        )

    def forward(self, latent):
        x = self.network(latent)

        return x.view(
            -1,
            self.point_count,
            3,
        )


class Autoencoder(nn.Module):
    def __init__(
        self,
        latent_dim=128,
        point_count=128,
    ):
        super().__init__()

        self.encoder = PointNetEncoder(
            latent_dim
        )

        self.decoder = PointCloudDecoder(
            latent_dim,
            point_count,
        )

    def forward(self, points):
        latent = self.encoder(points)
        reconstruction = self.decoder(
            latent
        )

        return reconstruction, latent


def chamfer_distance(
    predicted,
    target,
):
    distances = torch.cdist(
        predicted,
        target,
        p=2,
    )

    distances = distances.square()

    a = (
        distances
        .min(dim=2)
        .values
        .mean()
    )

    b = (
        distances
        .min(dim=1)
        .values
        .mean()
    )

    return a + b


def run_epoch(
    model,
    loader,
    device,
    optimizer=None,
):
    training = optimizer is not None

    if training:
        model.train()
    else:
        model.eval()

    total = 0.0
    count = 0

    context = (
        torch.enable_grad()
        if training
        else torch.no_grad()
    )

    with context:

        for points in loader:
            points = points.to(device)

            if training:
                optimizer.zero_grad(
                    set_to_none=True
                )

            reconstructed, _ = model(
                points
            )

            loss = chamfer_distance(
                reconstructed,
                points,
            )

            if training:
                loss.backward()
                optimizer.step()

            batch = points.shape[0]

            total += (
                loss.item() * batch
            )

            count += batch

    return total / count


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--data",
        type=Path,
        default=Path("dataset/ml"),
    )

    parser.add_argument(
        "--splits",
        type=Path,
        default=Path(
            "dataset/ml/splits"
        ),
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path(
            "dataset/ml/autoencoder"
        ),
    )

    parser.add_argument(
        "--points",
        type=int,
        default=128,
    )

    parser.add_argument(
        "--latent",
        type=int,
        default=128,
    )

    parser.add_argument(
        "--batch-size",
        type=int,
        default=8,
    )

    parser.add_argument(
        "--epochs",
        type=int,
        default=3,
    )

    parser.add_argument(
        "--learning-rate",
        type=float,
        default=1e-3,
    )

    parser.add_argument(
        "--seed",
        type=int,
        default=1337,
    )

    args = parser.parse_args()

    random.seed(args.seed)
    np.random.seed(args.seed)
    torch.manual_seed(args.seed)

    device = torch.device("cpu")

    train_dataset = PointCloudDataset(
        args.data,
        args.splits / "train_ids.npy",
        points=args.points,
        random_subset=True,
    )

    val_dataset = PointCloudDataset(
        args.data,
        args.splits / "val_ids.npy",
        points=args.points,
        random_subset=False,
    )

    test_dataset = PointCloudDataset(
        args.data,
        args.splits / "test_ids.npy",
        points=args.points,
        random_subset=False,
    )

    print("Device:", device)
    print("Points/model:", args.points)
    print("Latent:", args.latent)
    print("Train:", len(train_dataset))
    print("Validation:", len(val_dataset))
    print("Test:", len(test_dataset))

    train_loader = DataLoader(
        train_dataset,
        batch_size=args.batch_size,
        shuffle=True,
        num_workers=0,
    )

    val_loader = DataLoader(
        val_dataset,
        batch_size=args.batch_size,
        shuffle=False,
        num_workers=0,
    )

    test_loader = DataLoader(
        test_dataset,
        batch_size=args.batch_size,
        shuffle=False,
        num_workers=0,
    )

    model = Autoencoder(
        latent_dim=args.latent,
        point_count=args.points,
    ).to(device)

    optimizer = torch.optim.Adam(
        model.parameters(),
        lr=args.learning_rate,
    )

    args.output.mkdir(
        parents=True,
        exist_ok=True,
    )

    best_path = (
        args.output / "best.pt"
    )

    log_path = (
        args.output / "training.csv"
    )

    best_val = float("inf")

    with log_path.open(
        "w",
        newline="",
    ) as file:

        writer = csv.writer(file)

        writer.writerow([
            "epoch",
            "train_loss",
            "val_loss",
        ])

        for epoch in range(
            1,
            args.epochs + 1,
        ):
            train_loss = run_epoch(
                model,
                train_loader,
                device,
                optimizer,
            )

            val_loss = run_epoch(
                model,
                val_loader,
                device,
            )

            writer.writerow([
                epoch,
                train_loss,
                val_loss,
            ])

            file.flush()

            print(
                f"Epoch "
                f"{epoch:03d}/"
                f"{args.epochs:03d}  "
                f"train={train_loss:.6f}  "
                f"val={val_loss:.6f}"
            )

            if val_loss < best_val:
                best_val = val_loss

                torch.save(
                    {
                        "model_state":
                            model.state_dict(),

                        "epoch": epoch,
                        "val_loss":
                            val_loss,

                        "latent_dim":
                            args.latent,

                        "point_count":
                            args.points,
                    },
                    best_path,
                )

                print(
                    "  saved best model"
                )

    checkpoint = torch.load(
        best_path,
        map_location=device,
        weights_only=False,
    )

    model.load_state_dict(
        checkpoint[
            "model_state"
        ]
    )

    test_loss = run_epoch(
        model,
        test_loader,
        device,
    )

    print()
    print(
        "Best epoch:",
        checkpoint["epoch"],
    )

    print(
        "Best val:",
        checkpoint["val_loss"],
    )

    print(
        "Test loss:",
        test_loss,
    )

    print(
        "Saved:",
        best_path,
    )


if __name__ == "__main__":
    main()
