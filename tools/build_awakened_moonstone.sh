#!/usr/bin/env bash

set -euo pipefail

ROOT="$(
    cd "$(
        dirname "${BASH_SOURCE[0]}"
    )/.."
    pwd
)"

cd "$ROOT"

echo
echo "========================================"
echo " AWAKENED MOONSTONE BUILD"
echo "========================================"

CONCEPT="dataset/ml/awakened_moonstone_scimitar_concept_sheet.png"

if [[ ! -f "$CONCEPT" ]]; then
    echo
    echo "ERROR: concept image missing:"
    echo "  $CONCEPT"
    echo
    return 1 2>/dev/null || exit 1
fi


if [[ ! -f ".venv-ml/bin/activate" ]]; then
    echo
    echo "ERROR: .venv-ml is missing."
    return 1 2>/dev/null || exit 1
fi


source .venv-ml/bin/activate


echo
echo "=== GENERATE MODEL ==="

python \
    ml/concept_to_awakened_moonstone_scimitar.py


echo
echo "=== SHOW EXTRACTED SIDE VIEW ==="

xdg-open \
    dataset/ml/generated_awakened_moonstone_scimitar/side_crop.png \
    >/dev/null 2>&1 &


echo
echo "=== SHOW SIDE MASK ==="

xdg-open \
    dataset/ml/generated_awakened_moonstone_scimitar/side_mask.png \
    >/dev/null 2>&1 &


echo
echo "=== PATCH CLIENT ==="

cp \
    src/apps/elclient/Client.cpp \
    /tmp/Client.cpp.before-awakened-moonstone


python - <<'PY'
import json
from pathlib import Path


client = Path(
    "src/apps/elclient/Client.cpp"
)

meta_path = Path(
    "dataset/ml/"
    "generated_awakened_moonstone_scimitar/"
    "awakened_moonstone_scimitar.meta.json"
)


meta = json.loads(
    meta_path.read_text()
)

grip = meta[
    "parts"
]["grip"]

grip_begin = int(
    grip["vertex_begin"]
)

grip_end = int(
    grip["vertex_end"]
)


s = client.read_text()


# ------------------------------------------------------------
# MODEL PATH
# ------------------------------------------------------------

old_path = '''"dataset/ml/"
                        "generated_moonstone_scimitar_v3/"
                        "moonstone_scimitar_v3.obj"'''

new_path = '''"dataset/ml/"
                        "generated_awakened_moonstone_scimitar/"
                        "awakened_moonstone_scimitar.obj"'''


if new_path not in s:
    if old_path not in s:
        raise SystemExit(
            "Could not locate current V3 OBJ path."
        )

    s = s.replace(
        old_path,
        new_path,
        1,
    )


# ------------------------------------------------------------
# FIND CURRENT WORKING V3 PLACEMENT
# ------------------------------------------------------------

start_marker = (
    "                // ============================================\n"
    "                // MOONSTONE V3 FINAL WORN PLACEMENT"
)

start = s.find(
    start_marker
)


# Allow reruns.
if start == -1:
    start_marker = (
        "                // ============================================\n"
        "                // AWAKENED MOONSTONE FINAL WORN PLACEMENT"
    )

    start = s.find(
        start_marker
    )


if start == -1:
    raise SystemExit(
        "Could not locate working Moonstone placement block."
    )


end_marker = (
    "                const auto generatedVertices ="
)

end = s.find(
    end_marker,
    start,
)


if end == -1:
    raise SystemExit(
        "Could not find generatedVertices marker."
    )


replacement = f'''                // ============================================
                // AWAKENED MOONSTONE FINAL WORN PLACEMENT
                //
                // Proven V3 placement grammar:
                //
                //   fit -> mirror Y profile
                //       -> exact semantic grip
                //       -> translate to native hand
                //
                // grip indices are generated from meta.json.
                // ============================================

                eld::elclient::debug::
                    fitToWornReference(
                        moonstoneScimitar,
                        *referenceSword
                    );


                if (!moonstoneScimitar.vertices.empty()) {{

                    float minY =
                        moonstoneScimitar.vertices.front().y;

                    float maxY =
                        minY;


                    for (
                        const auto& vertex :
                        moonstoneScimitar.vertices
                    ) {{
                        minY =
                            std::min(
                                minY,
                                vertex.y
                            );

                        maxY =
                            std::max(
                                maxY,
                                vertex.y
                            );
                    }}


                    const float centerY =
                        (minY + maxY)
                        * 0.5f;


                    for (
                        auto& vertex :
                        moonstoneScimitar.vertices
                    ) {{
                        vertex.y =
                            centerY
                            - (
                                vertex.y
                                - centerY
                            );
                    }}


                    constexpr std::size_t gripBegin =
                        {grip_begin};

                    constexpr std::size_t gripEnd =
                        {grip_end};


                    if (
                        moonstoneScimitar.vertices.size()
                        >= gripEnd
                    ) {{
                        float gripX = 0.0f;
                        float gripY = 0.0f;
                        float gripZ = 0.0f;


                        for (
                            std::size_t index =
                                gripBegin;
                            index < gripEnd;
                            ++index
                        ) {{
                            const auto& vertex =
                                moonstoneScimitar.vertices[
                                    index
                                ];

                            gripX += vertex.x;
                            gripY += vertex.y;
                            gripZ += vertex.z;
                        }}


                        constexpr float gripCount =
                            static_cast<float>(
                                gripEnd
                                - gripBegin
                            );


                        gripX /= gripCount;
                        gripY /= gripCount;
                        gripZ /= gripCount;


                        constexpr float nativeGripX =
                            -29.0f;

                        constexpr float nativeGripY =
                            -98.25f;

                        constexpr float nativeGripZ =
                            2.50f;


                        const float dx =
                            nativeGripX
                            - gripX;

                        const float dy =
                            nativeGripY
                            - gripY;

                        const float dz =
                            nativeGripZ
                            - gripZ;


                        for (
                            auto& vertex :
                            moonstoneScimitar.vertices
                        ) {{
                            vertex.x += dx;
                            vertex.y += dy;
                            vertex.z += dz;
                        }}


                        std::cout
                            << "Awakened Moonstone profile Y = "
                            << centerY
                            << '\\n'

                            << "Awakened exact grip = "
                            << gripX << ", "
                            << gripY << ", "
                            << gripZ << '\\n'

                            << "Awakened grip correction = "
                            << dx << ", "
                            << dy << ", "
                            << dz << '\\n'

                            << "Awakened final grip = "
                            << nativeGripX << ", "
                            << nativeGripY << ", "
                            << nativeGripZ << '\\n';
                    }}
                }}


'''


s = (
    s[:start]
    + replacement
    + s[end:]
)


client.write_text(s)


print(
    "Client switched to Awakened Moonstone."
)

print(
    "Semantic grip:",
    grip_begin,
    "..",
    grip_end - 1,
)
PY


echo
echo "=== VERIFY CLIENT ==="

rg -n -C 4 \
    'generated_awakened|AWAKENED MOONSTONE|gripBegin|Awakened exact grip' \
    src/apps/elclient/Client.cpp


echo
echo "=== BUILD CLIENT ==="

cmake --build \
    build \
    --target elclient \
    -j


echo
echo "=== RUN ==="

./run-client-clean.sh
