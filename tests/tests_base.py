import io
import json
from typing import List

import colorama
import ctypes
import os

import shutil
import sys
import time

from PIL import Image
from colorama import Fore

colorama.init()


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_logger(msg):
    print(msg.decode('utf-8', errors='replace'))


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_error_logger(msg):
    print(f"{Fore.RED}{msg.decode('utf-8', errors='replace')}{Fore.RESET}")


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_debug_logger(msg):
    print(f"{Fore.LIGHTYELLOW_EX}{msg.decode('utf-8', errors='replace')}{Fore.RESET}")


def check_decode(raw: bytes, name: str):
    try:
        raw.decode('utf-8')
    except UnicodeDecodeError as e:
        print(f"{name} Invalid UTF-8 bytes: {raw[e.start:e.end]}")
        raise


script_folder = os.path.dirname(os.path.abspath(__file__))
output_folder = os.path.join(script_folder, 'output')
rm_lines_sys_src_path = os.path.join(script_folder, '..', 'rm_lines_sys', 'src')
sys.path.append(rm_lines_sys_src_path)
svg_output_folder = os.path.join(output_folder, 'svg')
png_output_folder = os.path.join(output_folder, 'png')
zoom_output_folder = os.path.join(output_folder, 'zoom')
json_output_folder = os.path.join(output_folder, 'json')
md_output_folder = os.path.join(output_folder, 'md')
txt_output_folder = os.path.join(output_folder, 'txt')
html_output_folder = os.path.join(output_folder, 'html')
files_draw_folder = os.path.join(script_folder, 'draw_files')
files_folder = os.path.join(script_folder, 'files')

os.makedirs(svg_output_folder, exist_ok=True)
os.makedirs(png_output_folder, exist_ok=True)
os.makedirs(zoom_output_folder, exist_ok=True)
os.makedirs(json_output_folder, exist_ok=True)
os.makedirs(md_output_folder, exist_ok=True)
os.makedirs(txt_output_folder, exist_ok=True)
os.makedirs(html_output_folder, exist_ok=True)

if sys.platform == 'win32':
    # Windows-specific code
    for sub_path in ('', 'Debug', 'Release'):
        lib_path = os.path.join(os.path.dirname(script_folder), 'build', sub_path, 'rm_lines.dll')
        if os.path.exists(lib_path):
            break
    sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding='utf-8', errors='replace')
else:
    # Unix-specific code (Linux, macOS)
    lib_path = os.path.join(os.path.dirname(script_folder), 'build',
                            f'librm_lines.{"so" if sys.platform == "linux" else "dylib"}')
shutil.copy(lib_path, copy_to := os.path.join('..', 'rm_lines_sys', 'src', 'rm_lines_sys', os.path.basename(lib_path)))
print(f"Copied the dynamic library from {lib_path} to {os.path.realpath(copy_to)} for {os.name}")

from rm_lines_sys import lib

lib.setLogger(python_logger)
lib.setErrorLogger(python_error_logger)
lib.setDebugLogger(python_debug_logger)
# lib.setDebugMode(True)
