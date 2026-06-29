#!/
import re
import sys
from pathlib import Path

src_dir = Path(sys.argv[1])

# ONLY match EXPORT functions (your macro already expands extern "C")
pattern = re.compile(
    r'EXPORT\s+[^\s]+\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\('
)

exports = set()

for file in src_dir.rglob("*"):
    if file.suffix in [".cpp", ".h", ".hpp"]:
        text = file.read_text(errors="ignore")
        exports.update(pattern.findall(text))

exports = sorted(exports)

# debug file
Path("exports.txt").write_text("\n".join(exports))

# wasm export format (_ prefix required)
cmake_list = ",".join([f"'_{e}'" for e in exports])
Path("exports.cmake").write_text(cmake_list)

print(f"Found EXPORT functions: {len(exports)}")
