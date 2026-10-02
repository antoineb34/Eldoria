#!/usr/bin/env bash

stdbuf -oL -eL ./build/bin/elclient 2>&1 |
awk '
    # spammy renderer stats
    /^Eldoria \|/ { next }

    # temporary raw model debug
    /^model raw / { skip_tail=1; next }
    skip_tail && /^  tail:/ { skip_tail=0; next }

    # world build summary
    /^=== WORLD SCENE BUILD ===$/ { world=1; next }
    world && /^world regions/ { next }
    world && /^rendered regions/ { next }
    world && /^=+$/ { world=0; next }

    { print; fflush() }
'
