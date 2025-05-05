import io
import json
from typing import List

import colorama
import ctypes
import os

import shutil
import sys
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

script_folder = os.path.dirname(os.path.abspath(__file__))
output_folder = os.path.join(script_folder, 'draw_output')
rm_lines_sys_src_path = os.path.join(script_folder, '..', 'rm_lines_sys', 'src')
sys.path.append(rm_lines_sys_src_path)
files_folder = os.path.join(script_folder, 'draw_files')

os.makedirs(output_folder, exist_ok=True)


if os.name == 'nt':
    # Windows-specific code
    for sub_path in ('', 'Debug', 'Release'):
        lib_path = os.path.join(os.path.dirname(script_folder), 'build', sub_path, 'rm_lines.dll')
        if os.path.exists(lib_path):
            break
    sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding='utf-8', errors='replace')
else:
    # Unix-specific code (Linux, macOS)
    lib_path = os.path.join(os.path.dirname(script_folder), 'build', 'librm_lines.so')

shutil.copy(lib_path, copy_to := os.path.join('..', 'rm_lines_sys', 'src', 'rm_lines_sys', os.path.basename(lib_path)))
print(f"Copied the dynamic library from {lib_path} to {os.path.realpath(copy_to)} for {os.name}")

from rm_lines_sys import lib

lib.setLogger(python_logger)
lib.setErrorLogger(python_error_logger)
lib.setDebugLogger(python_debug_logger)

for file in (files := os.listdir(files_folder)):
    print(f"Processing file: {file[:-3]}")
    output_path = os.path.join(output_folder, file.replace('.rm', '.png'))

    tree_id = lib.buildTree(os.path.join(files_folder, file).encode())

    if not tree_id:
        raise Exception("Failed to build tree")

    renderer_id = lib.makeRenderer(tree_id, 1, False)

    if not renderer_id:
        raise Exception("Failed to make renderer")

    scene_info = lib.getSceneInfo(tree_id)
    if scene_info:
        print(f"Scene info: {scene_info.decode()}")
    paper_size = json.loads(scene_info.decode()).get('paper_size', (1404, 1872)) if scene_info else (1404, 1872)

    buffer_size = paper_size[0] * paper_size[1]
    buffer = (ctypes.c_uint32 * buffer_size)()

    lib.getFrame(renderer_id, buffer, buffer_size * 4, 0, 0, *paper_size, 1)
    raw_frame = bytes(buffer)

    if not raw_frame:
        print(f"Couldn't get frame [{len(raw_frame)}]")
        exit(-1)
    else:
        print(f"Got frame [{len(raw_frame)}]")
        image = Image.frombytes('RGBA', paper_size, raw_frame, 'raw', 'RGBA')
        image.save(output_path, 'PNG')

    lib.destroyRenderer(renderer_id)
    lib.destroyTree(tree_id)