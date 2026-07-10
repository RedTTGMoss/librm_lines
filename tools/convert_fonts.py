from fontTools.ttLib import TTFont
from pathlib import Path
import shutil

BASE = Path(__file__).resolve().parent / "../rm_lines/assets/fonts"
BASE = BASE.resolve()


def unicode_map(font):
    result = {}

    for table in font["cmap"].tables:
        if table.isUnicode():
            for codepoint, glyph in table.cmap.items():
                result[codepoint] = glyph

    return result


def copy_metrics(source, target):
    source_hmtx = source["hmtx"]
    target_hmtx = target["hmtx"]

    source_map = unicode_map(source)
    target_map = unicode_map(target)

    copied = 0

    for cp, source_glyph in source_map.items():
        target_glyph = target_map.get(cp)

        if target_glyph is None:
            continue

        if source_glyph not in source_hmtx.metrics:
            continue

        target_hmtx.metrics[target_glyph] = source_hmtx.metrics[source_glyph]
        copied += 1

    print(f"  copied {copied} glyph metrics")


def copy_vertical_metrics(source, target):
    for table_name in ("OS/2", "hhea"):
        if not (table_name in source and table_name in target):
            continue

        src = source[table_name]
        dst = target[table_name]

        for attr in dir(src):
            if attr.startswith("_"):
                continue

            if hasattr(dst, attr):
                try:
                    setattr(dst, attr, getattr(src, attr))
                except Exception:
                    pass


def copy_kerning(source, target):
    # Legacy kern table is safe only if glyph names exist
    if "kern" in source:
        try:
            target["kern"] = source["kern"]
        except Exception:
            pass

    # GPOS contains glyph-name references.
    # Rebuilding it requires mapping every glyph class.
    # Skip it for now.
    if "GPOS" in target:
        del target["GPOS"]

    print("  skipped GPOS kerning")


def convert(open_font, metric_font, output):
    print(f"\n{open_font.name} <- {metric_font.name}")

    src = TTFont(metric_font)
    dst = TTFont(open_font)

    copy_metrics(src, dst)
    copy_vertical_metrics(src, dst)
    copy_kerning(src, dst)

    # Make sure variable fonts don't surprise us
    if "fvar" in dst:
        del dst["fvar"]

    dst.save(output)

    print(f"saved {output}")


fonts = [
    (
        "Karla-VariableFont_wght.ttf",
        "reMarkableSans.ttf",
        "CompSans.ttf"
    ),
    (
        "LibreBaskerville-VariableFont_wght.ttf",
        "reMarkableSerif.ttf",
        "CompSerif.ttf"
    ),
    (
        "LibreBaskerville-Italic-VariableFont_wght.ttf",
        "reMarkableSerifItalic.ttf",
        "CompSerifItalic.ttf"
    ),
    (
        "Karla-Italic-VariableFont_wght.ttf",
        "reMarkableSans.ttf",
        "CompSansItalic.ttf"
    ),
]

for open_name, metric_name, output_name in fonts:
    convert(
        BASE / open_name,
        BASE / metric_name,
        BASE / output_name
    )
