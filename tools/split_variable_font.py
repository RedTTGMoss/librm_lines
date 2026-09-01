import sys
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont
from pathlib import Path

input_file = Path(sys.argv[1])
output_prefix = Path(sys.argv[2])

weights = [300, 400, 500, 700, 800]

font = TTFont(input_file)
for weight in weights:
    instance = instantiateVariableFont(
        font,
        {"wght": weight},
        inplace=False
    )

    output = output_prefix.parent / f"{output_prefix.name}_{weight}.ttf"
    instance.save(output)
