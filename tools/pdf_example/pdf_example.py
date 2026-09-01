from enum import Enum
from pathlib import Path
import pygameextra as pe
from numpy.testing import print_coercion_tables

script_dir = Path(__file__).parent
out_dir = script_dir / "out"

out_dir.mkdir(exist_ok=True)

pe.init((1, 1))

DPI = 227.54
PDF_SCALE = DPI / 72
RM2Width = 1404
RM2Height = 1872


class Mode(Enum):
    Width = 0
    Height = 1
    Custom = 2


def handle_file(name, mode: Mode = Mode.Width, custom=None):
    pdf_path = script_dir / f'{name}_pdf.png'
    lines_path = script_dir / f'{name}_lines.png'
    pdf = pe.Sprite(str(pdf_path))
    lines = pe.Sprite(str(lines_path))

    pdf.scale = (PDF_SCALE, PDF_SCALE)

    rect = pe.Rect(0, 0, *pdf.size)
    surface = pe.Surface(lines.size)
    with surface:
        rect.center = lines.width / 2, lines.height / 2

        pdf.display(rect.topleft)
        lines.display()
    surface.save_to_file(out_dir / f"{name}.png")


handle_file("1000x500", Mode.Width)
handle_file("500x1000", Mode.Width)
