#!/usr/bin/env python3

from pathlib import Path
import numpy as np

root = Path("dataset/ml")
out = root / "splits"
out.mkdir(parents=True, exist_ok=True)

model_ids = np.load(root / "model_ids.npy")
valid = np.load(root / "valid.npy")

ids = model_ids[valid].copy()

rng = np.random.default_rng(1337)
rng.shuffle(ids)

n = len(ids)

train_end = int(n * 0.80)
val_end = train_end + int(n * 0.10)

train = ids[:train_end]
val = ids[train_end:val_end]
test = ids[val_end:]

np.save(out / "train_ids.npy", train)
np.save(out / "val_ids.npy", val)
np.save(out / "test_ids.npy", test)

print("Valid:", len(ids))
print("Train:", len(train))
print("Validation:", len(val))
print("Test:", len(test))
