#!/usr/bin/env python3

import csv
from collections import defaultdict
from pathlib import Path

import numpy as np


DATA = Path("dataset/ml")
HASH_FILE = DATA / "geometry_hashes.csv"
OUT = DATA / "splits_grouped"

SEED = 1337


model_ids = np.load(
    DATA / "model_ids.npy"
)

valid = np.load(
    DATA / "valid.npy"
)

valid_ids = set(
    int(x)
    for x in model_ids[valid]
)


# ----------------------------------
# Load geometry hashes.
# ----------------------------------

hash_by_id = {}

with HASH_FILE.open(
    newline="",
    encoding="utf-8",
) as file:
    reader = csv.DictReader(file)

    for row in reader:
        model_id = int(
            row["model_id"]
        )

        hash_by_id[model_id] = (
            row["geometry_hash"]
        )


# ----------------------------------
# Group models by normalized geometry.
#
# If a valid model somehow has no hash,
# give it its own unique group.
# ----------------------------------

groups = defaultdict(list)

for model_id in sorted(valid_ids):

    key = hash_by_id.get(
        model_id,
        f"unique:{model_id}",
    )

    groups[key].append(
        model_id
    )


group_list = list(
    groups.values()
)

rng = np.random.default_rng(
    SEED
)

rng.shuffle(
    group_list
)


# Largest groups first makes it easier
# to keep the final split sizes close
# to the desired ratios.
group_list.sort(
    key=len,
    reverse=True,
)


total = len(valid_ids)

targets = {
    "train": int(total * 0.80),
    "val": int(total * 0.10),
}

targets["test"] = (
    total
    - targets["train"]
    - targets["val"]
)


splits = {
    "train": [],
    "val": [],
    "test": [],
}


def remaining(name):
    return (
        targets[name]
        - len(splits[name])
    )


# ----------------------------------
# Assign entire geometry groups.
# Never split a group.
# ----------------------------------

for group in group_list:

    # Pick split furthest below target.
    choices = sorted(
        splits.keys(),
        key=lambda name: remaining(name),
        reverse=True,
    )

    destination = choices[0]

    splits[destination].extend(
        group
    )


# Shuffle IDs inside each final split.
for values in splits.values():
    rng.shuffle(values)


OUT.mkdir(
    parents=True,
    exist_ok=True,
)


for name, values in splits.items():

    values = np.asarray(
        values,
        dtype=np.uint16,
    )

    np.save(
        OUT / f"{name}_ids.npy",
        values,
    )


print(
    "Valid models:",
    total,
)

print(
    "Geometry groups:",
    len(group_list),
)

print()

for name in (
    "train",
    "val",
    "test",
):
    print(
        f"{name:5s}: "
        f"{len(splits[name])}"
        f" target={targets[name]}"
    )


# ----------------------------------
# Prove no geometry group crosses
# split boundaries.
# ----------------------------------

membership = {}

for split_name, values in (
    splits.items()
):
    for model_id in values:

        key = hash_by_id.get(
            model_id,
            f"unique:{model_id}",
        )

        old = membership.get(key)

        if (
            old is not None
            and old != split_name
        ):
            raise RuntimeError(
                f"Geometry group leaked: "
                f"{key}"
            )

        membership[key] = (
            split_name
        )


print()
print(
    "Geometry leakage: NONE"
)
