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

via = {
    "name": kb["keyboard_name"],
    "vendorId": kb["usb"]["vid"],
    "productId": kb["usb"]["pid"],
    "matrix": kb["matrix_size"],
    "keycodes": ["qmk_lighting"],
    "menus": ["qmk_rgb_matrix"],
    "customKeycodes": [
        {"name": k["label"], "title": k["label"], "shortName": k["aliases"][0]}
        for k in kb["keycodes"]
    ],
    "layouts": {"keymap": kle},
}
print(json.dumps(via, indent=2))
