#!/usr/bin/env python3

import fitz
import sys
import argparse
from colorama import init as colorama_init
from colorama import Fore

colorama_init(autoreset=True)

MEDIUM_WIDTH_START = 234
TEXT_TOP = 140
DEFAULT_DPI = 227.5
print(f"Default DPI: {DEFAULT_DPI}")


def parse_font_style(font_name):
    name = font_name.lower()

    italic = "italic" in name

    if "extrabold" in name:
        weight = 800
    elif "bold" in name:
        weight = 700
    elif "medium" in name:
        weight = 500
    elif "light" in name:
        weight = 300
    else:
        weight = 400

    if "sans" in name:
        family = "Sans"
    elif "serif" in name:
        family = "Serif"
    else:
        family = "Unknown"

    return family, weight, italic


def pt_to_pixels(points, dpi):
    return points * dpi / 72.0


def extract_fonts(pdf_path, dpi):
    doc = fitz.open(pdf_path)

    results = []

    for page_number, page in enumerate(doc):
        data = page.get_text("dict")

        for block in data["blocks"]:
            if "lines" not in block:
                continue

            for line in block["lines"]:
                for span in line["spans"]:
                    family, weight, italic = parse_font_style(span["font"])

                    results.append({
                        "page": page_number + 1,
                        "text": span["text"],
                        "font": span["font"],

                        "family": family,
                        "weight": weight,
                        "italic": italic,

                        "size_pt": span["size"],
                        "size_px": pt_to_pixels(span["size"], dpi),

                        "x": span["bbox"][0],
                        "y": span["bbox"][1],
                        "text_x_px": pt_to_pixels(span["origin"][0], dpi),
                        "baseline_y": pt_to_pixels(span["origin"][1], dpi)
                    })

    results.sort(key=lambda x: (x["page"], x["y"], x["x"]))

    return results


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("pdf")
    parser.add_argument(
        "--dpi",
        type=float,
        default=DEFAULT_DPI,
        help="Vertical rendering DPI"
    )

    args = parser.parse_args()

    last_y = TEXT_TOP
    for item in extract_fonts(args.pdf, args.dpi):
        print(
            f"Page {item['page']:2} "
            f"{Fore.RED}X={item['text_x_px']:8.2f}   "
            f"MD={item['text_x_px'] - MEDIUM_WIDTH_START:8.2f}   "
            f"{Fore.GREEN}BY={item['baseline_y']:8.2f}   "
            f"YD={item['baseline_y'] - last_y:8.2f}   "
            f"{Fore.CYAN}{item['family']:6} "
            f"{item['size_pt']:6.2f}pt "
            f"{item['size_px']:4.2f}px "
            f"weight={item['weight']:3} "
            f"italic={str(item['italic']):5} "
            f"{Fore.RESET}| {item['text']!r}"
        )
        last_y = item['baseline_y']
