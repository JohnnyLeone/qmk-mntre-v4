# Submitting to qmk_firmware

Checklist for a pull request adding `keyboards/mntre_v4` to
[qmk/qmk_firmware](https://github.com/qmk/qmk_firmware), against the
[PR checklist](https://docs.qmk.fm/pr_checklist).

## Blockers

- [ ] **Font license.** `reform4_font.c` is MNT's font, GPL-3.0-or-later.
  qmk_firmware only accepts GPL-2.0-or-later compatible code. Either:
  - ask MNT Research to also license the font (and the battery reply
    format handling, which mirrors their `remote.c`) under
    GPL-2.0-or-later, or
  - drop it: use QMK's default `glcdfont.c` and draw new battery and logo
    glyphs.
- [ ] **USB ID.** `1209:6D02` is MNT's pid.codes allocation. Get MNT's OK
  to use it in QMK, ideally with an MNT co-maintainer.

## Required

- [ ] Photo of the keyboard in `keyboards/mntre_v4/readme.md` (hosted on
  imgur or similar; QMK does not take images in the repo).
- [x] `maintainer` is a GitHub username (`JohnnyLeone`).
- [x] Name follows upstream's `mntre` / `mntre_v3`.
- [x] Configuration in `keyboard.json` where it has a data-driven option;
  `qmk lint -kb mntre_v4 --strict` passes (checked in CI).
- [x] Keyboard-level hooks call their `_user` counterparts
  (`process_record_kb`, `oled_task_kb`, `housekeeping_task_kb`, ...).
- [x] No VIA keymap in the keyboard folder (`keymaps/via` lives outside;
  submit it to [the-via/qmk_userspace_via](https://github.com/the-via/qmk_userspace_via)
  and `via/mntre_v4.json` to [the-via/keyboards](https://github.com/the-via/keyboards)).
- [x] C code follows QMK's clang-format style; keymap tables are wrapped in
  `// clang-format off/on`.

## Worth discussing in the PR

- The custom matrix: QMK's stock matrix only scans active low, the board
  is active high with pull-downs.
- The folded duplicate matrix positions (`[3,13]`, `[5,5]`).
- Default keymap: the Circle layer reproduces MNT's Circle menu, the Hyper
  layer MNT's Fn layer plus system controller keys. The checklist asks for
  a "clean slate" default keymap, so reviewers may want it closer to the
  printed legends.

## Steps

1. Fork qmk/qmk_firmware and create a feature branch (not your fork's
   `master`). New keyboards target `master`; only keyboard moves and
   refactors go through `develop`.
2. Copy `keyboards/mntre_v4` into the fork.
3. Run `qmk lint -kb mntre_v4 --strict`, `qmk format-c` on the new files,
   and `qmk compile -kb mntre_v4 -km default`.
4. Open the PR against `master`, filling in the template.
