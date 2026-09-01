#!/
import re
import sys
from pathlib import Path

src_dir = Path(sys.argv[1])

# Match EXPORT function declarations/definitions, including multi-token return
# types such as `const char *` and signatures that span multiple lines.
pattern = re.compile(
    r'\bEXPORT\b(?:(?![;{}]).)*?\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(',
    re.DOTALL,
)

exports = set()

for file in src_dir.rglob("*"):
    if file.suffix in [".cpp", ".h", ".hpp"]:
        text = file.read_text(errors="ignore")
        # Ignore preprocessor directives so `#define EXPORT ...` is never
        # mistaken for an exported function declaration.
        filtered_text = "\n".join(
            line for line in text.splitlines() if not line.lstrip().startswith("#")
        )
        exports.update(pattern.findall(filtered_text))

exports = sorted(exports)

# debug file
Path("exports.txt").write_text("\n".join(exports))

# wasm export format (_ prefix required)
flags = ["_malloc", "_free", *[f"_{e}" for e in exports]]

Path("exported_functions.txt").write_text(
    f"-s EXPORTED_FUNCTIONS=[{','.join(flags)}]\n"
)

print(f"Found EXPORT functions: {len(exports)}")
