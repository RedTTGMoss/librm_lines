import argparse
import json
from collections import Counter
from pathlib import Path
from natsort import natsorted
import os

script_dir = Path(__file__).parent

TEXT_TOP = 142
MEDIUM_WIDTH_START = 234
styles = natsorted(
    [
        "PlainText",
        "Title",
        "Sub",
        "Bullet",
        "BulletTab",
        "CheckBox",
        "CheckBoxChecked",
        "CheckBoxTab",
        "CheckBoxTabChecked",
        "Numbered",
    ]
)  # We need NATURAL alphabetical sorting


def handle_page(page, style_a, style_b):
    rel_a: dict | None = None
    rel_b: dict | None = None
    for item in page:
        if item["text"] == style_a and not rel_a:
            rel_a = item
        elif item["text"] == style_b and not rel_b:
            rel_b = item
        elif rel_a and rel_b:
            break
    if rel_a is None or rel_b is None:
        raise Exception(f"Could not find both styles on the page: {style_a}, {style_b}")
    return {
        "heights": {
            f"TextTop-{style_a}": rel_a["baseline_y"] - TEXT_TOP,
            f"{style_a}-{style_b}": rel_b["baseline_y"] - rel_a["baseline_y"],
        },
        "margins": {
            style_a: rel_a['text_x_px'] - MEDIUM_WIDTH_START,
            style_b: rel_b['text_x_px'] - MEDIUM_WIDTH_START
        }
    }


def get_common(values):
    return Counter(values).most_common(1)[0][0]


def filter_basic_height(heights):
    for style_a in styles + ['TextTop']:
        values = []
        for style_b in styles:
            values.append(heights[f'{style_a}-{style_b}'])
        common_base = get_common(values)
        for style_b in styles:
            key = f'{style_a}-{style_b}'
            if heights[key] == common_base:
                heights.pop(key)
        heights[f'{style_a}-BASIC'] = common_base


def filter_basic_margins(margins):
    print(f"constexpr float TAB_LENGTH = {margins['BulletTab'] - margins['Bullet']};")
    for style_a in styles:
        if "Tab" in style_a:
            margins.pop(style_a)
    values = list(margins.values())
    common_base = get_common(values)
    for style_a in styles:
        if margins.get(style_a) == common_base:
            margins.pop(style_a)
    margins['BASIC'] = common_base


def create_output(heights, margins):
    file = script_dir / 'text_info/text_scales_part.cpp'
    tops = {}

    for height, value in heights.items():
        style_a, style_b = height.split('-')
        if not tops.get(style_a):
            tops[style_a] = {}
        tops[style_a][style_b] = value

    with open(file, 'w') as f:
        for style_a, items in tops.items():
            f.write(f"constexpr StyleScaleList {style_a}_Rel = "
                    f"{{\n\t{{\n\t\t{{BASIC, {items.pop('BASIC', 0)}}},\n")
            for i, (style_b, value) in enumerate(items.items(), 1):
                f.write(f"\t\t{{{style_b}, {value}}}")
                if i < len(items):
                    f.write(",\n")
            if len(items) + 1 < 12:
                f.write(",\n\t\tEndOfStyleList")
            f.write("\n\t}\n};\n")

        f.write("constexpr StyleScaleList StyleLeftMargins = "
                f"{{\n\t{{\n\t\t{{BASIC, {margins.pop('BASIC', 0)}}},\n")
        for i, (style_a, value) in enumerate(margins.items(), 1):
            f.write(f"\t\t{{{style_a}, {value}}}")
            if i < len(margins):
                f.write(",\n")
        if len(items) + 1 < 12:
            f.write(",\n\t\tEndOfStyleList")
        f.write("\n\t}\n};\n")


def gen_style_pairs():
    for style_a in styles:
        for style_b in styles:
            yield style_a, style_b


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--json_file",
        help="Path to the JSON file containing style scales",
        default=script_dir / "text_info/All the styles (5).json",
    )
    args = parser.parse_args()

    file_to_read = args.json_file
    with open(file_to_read, "r") as f:
        data = json.load(f)
    style_pairs = list(gen_style_pairs())

    heights = {}
    margins = {}
    for page, pair in zip(data, style_pairs):
        info = handle_page(page, *pair)
        heights.update(info['heights'])
        margins.update(info['margins'])
    filter_basic_height(heights)
    filter_basic_margins(margins)
    create_output(heights, margins)
