"""Build a VIA v3 definition from keyboard.json (keyboard.json path as argv[1])."""
import json, sys

kb = json.load(open(sys.argv[1]))
rows = {}
for k in kb["layouts"]["LAYOUT"]["layout"]:
    rows.setdefault(k["y"], []).append(k)

kle = []
for y in sorted(rows):
    out, x = [], 0.0
    for k in sorted(rows[y], key=lambda k: k["x"]):
        props = {}
        if k["x"] != x:
            props["x"] = k["x"] - x
        if k.get("w", 1) != 1:
            props["w"] = k["w"]
        if props:
            out.append(props)
        out.append("%d,%d" % tuple(k["matrix"]))
        x = k["x"] + k.get("w", 1)
    kle.append(out)

# VIA's stock "qmk_rgb_matrix" menu lists every QMK effect by number; this
# keyboard only builds the static ones (see README, Backlight), numbered in
# rgb_matrix_effects.inc order.
EFFECTS = [["Off", 0], ["Solid Color", 1], ["Alphas Mods", 2],
           ["Gradient Up Down", 3], ["Gradient Left Right", 4]]
ON = "{id_qmk_rgb_matrix_effect} != 0"
LIGHTING_MENU = {
    "label": "Lighting",
    "content": [{
        "label": "Backlight",
        "content": [
            {"label": "Brightness", "type": "range", "options": [0, 255],
             "content": ["id_qmk_rgb_matrix_brightness", 3, 1]},
            {"label": "Effect", "type": "dropdown", "options": EFFECTS,
             "content": ["id_qmk_rgb_matrix_effect", 3, 2]},
            {"showIf": ON, "label": "Gradient / Modifier Shift", "type": "range",
             "options": [0, 255], "content": ["id_qmk_rgb_matrix_effect_speed", 3, 3]},
            {"showIf": ON, "label": "Color", "type": "color",
             "content": ["id_qmk_rgb_matrix_color", 3, 4]},
        ],
    }],
}

via = {
    "name": kb["keyboard_name"],
    "vendorId": kb["usb"]["vid"],
    "productId": kb["usb"]["pid"],
    "matrix": kb["matrix_size"],
    "keycodes": ["qmk_lighting"],
    "menus": [LIGHTING_MENU],
    "customKeycodes": [
        {"name": k["label"], "title": k["label"], "shortName": k["aliases"][0]}
        for k in kb["keycodes"]
    ],
    "layouts": {"keymap": kle},
}
print(json.dumps(via, indent=2))
