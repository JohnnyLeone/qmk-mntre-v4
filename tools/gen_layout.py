"""Emit LAYOUT and rgb_matrix.layout JSON for the Reform Keyboard 4.

Geometry follows upstream QMK's mntre_v3 (same physical layout). LEDs are
chained row-major, left to right, top to bottom: 14,14,14,13,14,11 = 80.
"""
import json, sys

def row(r, spec):
    # spec: list of (col, width); x accumulates
    x, out = 0.0, []
    for col, w in spec:
        out.append((r, col, x, w))
        x += w
    return out

ones = lambda cols: [(c, 1) for c in cols]
keys = []
keys += row(0, ones(range(13)) + [(13, 1.5)])
keys += row(1, ones(range(13)) + [(13, 1.5)])
keys += row(2, [(0, 1.5)] + ones(range(1, 14)))
keys += row(3, [(0, 1.75)] + ones(range(1, 12)) + [(12, 1.75)])
keys += row(4, [(0, 1.25)] + ones(range(1, 13)) + [(13, 1.25)])
keys += row(5, [(0, 1.25), (1, 1.5), (2, 1.5), (3, 1.5), (4, 2), (6, 1.5)]
               + ones(range(7, 11)) + [(11, 1.25)])
assert len(keys) == 80, len(keys)

# Keys lit as "modifier" (LED_FLAG_MODIFIER) rather than "keylight".
mods = {(0, 0), (0, 13), (1, 13), (2, 0), (3, 0), (3, 12), (4, 0), (4, 13),
        (5, 0), (5, 1), (5, 2), (5, 7), (5, 8)}

def num(v):
    return int(v) if float(v).is_integer() else v

layout, leds = [], []
for r, c, x, w in keys:
    k = {"matrix": [r, c], "x": num(x), "y": r}
    if w != 1:
        k["w"] = num(w)
    layout.append(k)
    leds.append({"matrix": [r, c],
                 "x": round((x + w / 2) * 224 / 14.5),
                 "y": round((r + 0.5) * 64 / 6),
                 "flags": 1 if (r, c) in mods else 4})

def dump(items):
    return ",\n".join("                " + json.dumps(i) for i in items)

which = sys.argv[1]
print(dump(layout if which == "layout" else leds))
