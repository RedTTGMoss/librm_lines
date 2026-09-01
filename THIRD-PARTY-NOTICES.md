# Third-Party Notices

This file lists third-party software currently used by `librm_lines` and
provides a simple policy for future third-party additions.

If a dependency is added, removed, vendored, or upgraded in `CMakeLists.txt`
or elsewhere in the code base, update this file so the shipped notices stay in
sync with what is actually distributed.

## Current third-party software

---

### `nlohmann/json`

- Upstream: https://github.com/nlohmann/json
- Version used by CMake: `v3.12.0`
- License: MIT
- Notes: preserve the upstream copyright and license notice.

---

### `cppcodec`

- Upstream: https://github.com/tplgy/cppcodec
- Version used by CMake: `master` branch snapshot
- License: MIT
- Notes: preserve the upstream copyright and license notice.

---

### `utfcpp`

- Upstream: https://github.com/nemtrif/utfcpp
- Version used by CMake: `v4.0.6`
- License: Boost Software License 1.0
- Notes: preserve the full copyright notice and license statement.

---

### `simdutf`

- Upstream: https://github.com/simdutf/simdutf
- Version used by CMake: `v8.2.0`
- License: Apache License 2.0 or MIT
- Notes: this dependency is dual-licensed upstream; preserve the applicable
  upstream notices.

---

### `FreeType`

- Upstream: https://gitlab.freedesktop.org/freetype/freetype
- Version used by CMake: `VER-2-14-3`
- License: FreeType Project License
- Notes: the upstream license asks for credit/acknowledgement in distributed
  products.

---

### `HarfBuzz`

- Upstream: https://github.com/harfbuzz/harfbuzz
- Version used by CMake: `11.5.0`
- License: Old MIT license
- Notes: preserve upstream copyright and license notices.

---

### `stb_image` and `stb_perlin`

- Location in this repository:
    - `rm_lines/headers/stb/stb_image.h`
    - `rm_lines/headers/stb/stb_perlin.h`
    - `rm_lines/src/stb/stb_image.cpp`
    - `rm_lines/src/stb/stb_perlin.cpp`
- License: public domain / no warranty
- Notes: keep the upstream file headers intact when updating these copies.

---

## Future third-party projects

When a new third-party project is introduced, add a new section here with at
least the following information:

- upstream project name and URL
- version, tag, commit, or release source used
- license name(s)
- where it is used in this repository
- any special attribution or redistribution requirements

For vendored code, keep the original license header and upstream copyright
notice in the source files whenever possible.

For CMake-fetched dependencies, prefer recording the exact version or tag used
so the notices remain reproducible over time.



