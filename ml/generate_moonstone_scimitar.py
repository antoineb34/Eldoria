#!/usr/bin/env python3

from pathlib import Path
import math

OUT = Path("dataset/ml/generated_moonstone_scimitar")
OUT.mkdir(parents=True, exist_ok=True)

OBJ = OUT / "moonstone_scimitar.obj"
MTL = OUT / "moonstone_scimitar.mtl"


# ============================================================
# Basic mesh builder
# ============================================================

vertices = []
faces = []


def vertex(x, y, z):
    vertices.append((x, y, z))
    return len(vertices)  # OBJ = 1 based


def tri(a, b, c, material):
    faces.append((a, b, c, material))


def quad(a, b, c, d, material):
    tri(a, b, c, material)
    tri(a, c, d, material)


# ============================================================
# Materials
# ============================================================

materials = {
    "moonstone_white": (0.90, 0.96, 1.00),
    "moonstone_blue":  (0.48, 0.72, 0.96),
    "moonstone_cyan":  (0.42, 0.88, 0.95),

    "silver":          (0.62, 0.67, 0.74),
    "silver_dark":     (0.36, 0.40, 0.48),

    "grip":            (0.10, 0.14, 0.24),
    "grip_light":      (0.16, 0.21, 0.33),

    "gem":             (0.30, 0.82, 1.00),
}


# ============================================================
# BLADE
#
# Source-space convention:
#
#     Y = length
#     X = visible width / scimitar curve
#     Z = thickness
#
# Our existing fitToWornReference() then maps:
#
#     source Z -> RuneScape X
#     source X -> RuneScape Y
#     source Y -> RuneScape Z
#
# That's exactly what we want for model 490.
# ============================================================

sections = 6

blade = []

for i in range(sections):
    t = i / (sections - 1)

    y = -0.02 + t * 1.02

    # Scimitar bends more aggressively near the upper half.
    center_x = (
        0.035 * t +
        0.34 * (t ** 1.65)
    )

    # Broad belly, then taper sharply to the point.
    half_width = (
        0.115 +
        0.075 * math.sin(math.pi * t)
    )

    half_width *= (
        1.0 - 0.78 * (t ** 5)
    )

    if i == sections - 1:
        half_width = 0.006

    thickness = (
        0.030 -
        0.010 * t
    )

    inner_x = center_x - half_width
    outer_x = center_x + half_width

    blade.append({
        "lf": vertex(inner_x, y, +thickness),
        "rf": vertex(outer_x, y, +thickness),
        "lb": vertex(inner_x, y, -thickness),
        "rb": vertex(outer_x, y, -thickness),
    })


# Connect blade sections.

for i in range(sections - 1):
    a = blade[i]
    b = blade[i + 1]

    # Broad front face.
    material = (
        "moonstone_white"
        if i % 2 == 0
        else "moonstone_blue"
    )

    quad(
        a["lf"],
        a["rf"],
        b["rf"],
        b["lf"],
        material,
    )

    # Back face.
    quad(
        a["rb"],
        a["lb"],
        b["lb"],
        b["rb"],
        "moonstone_blue",
    )

    # Cutting edge.
    quad(
        a["rf"],
        a["rb"],
        b["rb"],
        b["rf"],
        "moonstone_white",
    )

    # Inner curved edge.
    quad(
        a["lb"],
        a["lf"],
        b["lf"],
        b["lb"],
        "moonstone_cyan",
    )


# ============================================================
# GUARD
# ============================================================

guard_y = -0.10

# Simple winged low-poly guard.
g_left = vertex(-0.22, guard_y, 0.0)
g_center_top = vertex(0.0, guard_y + 0.065, 0.045)
g_right = vertex(0.22, guard_y, 0.0)
g_center_bottom = vertex(0.0, guard_y - 0.065, 0.045)

g_center_back = vertex(
    0.0,
    guard_y,
    -0.045,
)

tri(
    g_left,
    g_center_top,
    g_center_bottom,
    "silver",
)

tri(
    g_center_top,
    g_right,
    g_center_bottom,
    "silver_dark",
)

tri(
    g_left,
    g_center_back,
    g_center_top,
    "silver_dark",
)

tri(
    g_right,
    g_center_top,
    g_center_back,
    "silver",
)

tri(
    g_left,
    g_center_bottom,
    g_center_back,
    "silver",
)

tri(
    g_right,
    g_center_back,
    g_center_bottom,
    "silver_dark",
)


# Moonstone in guard.
gem = vertex(
    0.0,
    guard_y,
    0.070,
)

tri(
    g_center_top,
    g_right,
    gem,
    "gem",
)

tri(
    g_right,
    g_center_bottom,
    gem,
    "moonstone_cyan",
)

tri(
    g_center_bottom,
    g_left,
    gem,
    "gem",
)

tri(
    g_left,
    g_center_top,
    gem,
    "moonstone_white",
)


# ============================================================
# GRIP
# ============================================================

grip_top = -0.15
grip_bottom = -0.49

gx = 0.045
gz = 0.035

a = vertex(-gx, grip_top, +gz)
b = vertex(+gx, grip_top, +gz)
c = vertex(+gx, grip_bottom, +gz)
d = vertex(-gx, grip_bottom, +gz)

e = vertex(-gx, grip_top, -gz)
f = vertex(+gx, grip_top, -gz)
g = vertex(+gx, grip_bottom, -gz)
h = vertex(-gx, grip_bottom, -gz)

quad(a, b, c, d, "grip")
quad(f, e, h, g, "grip_light")

quad(a, e, f, b, "grip_light")
quad(b, f, g, c, "grip")
quad(c, g, h, d, "grip_light")
quad(d, h, e, a, "grip")


# ============================================================
# POMMEL
# ============================================================

pommel_y = -0.58

p_top = vertex(
    0.0,
    pommel_y + 0.085,
    0.0,
)

p_right = vertex(
    0.070,
    pommel_y,
    0.0,
)

p_bottom = vertex(
    0.0,
    pommel_y - 0.105,
    0.0,
)

p_left = vertex(
    -0.070,
    pommel_y,
    0.0,
)

p_front = vertex(
    0.0,
    pommel_y - 0.005,
    0.060,
)

p_back = vertex(
    0.0,
    pommel_y - 0.005,
    -0.060,
)

tri(p_top, p_right, p_front, "silver")
tri(p_right, p_bottom, p_front, "silver_dark")
tri(p_bottom, p_left, p_front, "silver")
tri(p_left, p_top, p_front, "silver_dark")

tri(p_right, p_top, p_back, "silver_dark")
tri(p_bottom, p_right, p_back, "silver")
tri(p_left, p_bottom, p_back, "silver_dark")
tri(p_top, p_left, p_back, "silver")


# Small glowing moonstone inset.
gem_front = vertex(
    0.0,
    pommel_y - 0.005,
    0.075,
)

tri(
    p_top,
    p_right,
    gem_front,
    "gem",
)

tri(
    p_right,
    p_bottom,
    gem_front,
    "moonstone_cyan",
)

tri(
    p_bottom,
    p_left,
    gem_front,
    "gem",
)

tri(
    p_left,
    p_top,
    gem_front,
    "moonstone_white",
)


# ============================================================
# Write MTL
# ============================================================

with MTL.open("w") as f:
    for name, (r, g, b) in materials.items():
        f.write(f"newmtl {name}\n")
        f.write(f"Kd {r:.4f} {g:.4f} {b:.4f}\n")
        f.write("Ka 0 0 0\n")
        f.write("Ks 0 0 0\n\n")


# ============================================================
# Write OBJ
# ============================================================

with OBJ.open("w") as f:
    f.write("# Eldoria Moonstone Scimitar\n")
    f.write(f"mtllib {MTL.name}\n\n")

    for x, y, z in vertices:
        f.write(
            f"v {x:.6f} "
            f"{y:.6f} "
            f"{z:.6f}\n"
        )

    current_material = None

    for a, b, c, material in faces:
        if material != current_material:
            f.write(
                f"\nusemtl {material}\n"
            )
            current_material = material

        f.write(
            f"f {a} {b} {c}\n"
        )


# ============================================================
# Stats
# ============================================================

xs = [p[0] for p in vertices]
ys = [p[1] for p in vertices]
zs = [p[2] for p in vertices]

print("=== MOONSTONE SCIMITAR ===")
print("vertices :", len(vertices))
print("triangles:", len(faces))

print()
print(
    "X:",
    min(xs),
    max(xs),
    "span =", max(xs) - min(xs),
)

print(
    "Y:",
    min(ys),
    max(ys),
    "span =", max(ys) - min(ys),
)

print(
    "Z:",
    min(zs),
    max(zs),
    "span =", max(zs) - min(zs),
)

print()
print("OBJ:", OBJ)
print("MTL:", MTL)
