import io

import colorama
import ctypes
import mmap
import os
import shutil
import sys
import time
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
output_folder = os.path.join(script_folder, 'output')
rm_lines_sys_src_path = os.path.join(script_folder, '..', 'rm_lines_sys', 'src')
sys.path.append(rm_lines_sys_src_path)
svg_output_folder = os.path.join(output_folder, 'svg')
json_output_folder = os.path.join(output_folder, 'json')
md_output_folder = os.path.join(output_folder, 'md')
html_output_folder = os.path.join(output_folder, 'html')
files_folder = os.path.join(script_folder, 'files')

os.makedirs(svg_output_folder, exist_ok=True)
os.makedirs(json_output_folder, exist_ok=True)
os.makedirs(md_output_folder, exist_ok=True)
os.makedirs(html_output_folder, exist_ok=True)

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

total_time = 0
total_size_of_renderers = 0
total_size_of_trees = 0
for file in (files := os.listdir(files_folder)):
    svg_output_path = os.path.join(svg_output_folder, file.replace('.rm', '.svg'))
    json_output_path = os.path.join(json_output_folder, file.replace('.rm', '.json'))
    md_output_path = os.path.join(md_output_folder, file.replace('.rm', '.md'))
    html_output_path = os.path.join(html_output_folder, file.replace('.rm', '.html'))

    print("Processing file:", file)
    begin = time.time()
    tree_id = lib.buildTree(os.path.join(files_folder, file).encode())
    total_time += (process_time := time.time() - begin)
    print(f"[{tree_id.decode()}] Read, time taken:", process_time)
    if not tree_id:
        raise Exception("Failed to build tree")

    begin = time.time()
    success = lib.convertToJsonFile(tree_id, json_output_path.encode())
    print(f"JSON (file) [{success}] Time taken:", time.time() - begin)
    begin = time.time()
    result = lib.convertToJson(tree_id)
    print(f"JSON (raw) [{len(result)}] Time taken:", time.time() - begin)

    # Confirm it complies with UTF-8
    try:
        result.decode('utf-8')
    except UnicodeDecodeError as e:
        print(f"JSON (raw) failed to decode: {e}")
        raise

    scene_info = lib.getSceneInfo(tree_id)
    if scene_info:
        print(f"Scene info: {scene_info.decode()}")

    # Make a renderer

    begin = time.time()
    renderer_id = lib.makeRenderer(tree_id, 0, False)
    if not renderer_id:
        raise Exception("Failed to make renderer")

    total_time += (renderer_time := time.time() - begin)
    print(f"It took {renderer_time:.04f} to initialize the renderer")

    # Get the paragraphs
    paragraphs = lib.getParagraphs(renderer_id)
    if paragraphs:
        print(f"Paragraphs: {paragraphs.decode()}")

    begin = time.time()
    success = lib.textToMdFile(renderer_id, md_output_path.encode())
    print(f"MD (file) [{success}] Time taken:", time.time() - begin)
    begin = time.time()
    result: bytes = lib.textToMd(renderer_id)
    print(f"MD (raw) [{len(result)}] Time taken:", time.time() - begin)

    # Confirm it complies with UTF-8
    try:
        result.decode('utf-8')
    except UnicodeDecodeError as e:
        print(f"MD (raw) failed to decode: {e}")
        raise

    begin = time.time()
    success = lib.textToHtmlFile(renderer_id, html_output_path.encode())
    print(f"HTML (file) [{success}] Time taken:", time.time() - begin)
    begin = time.time()
    result = lib.textToHtml(renderer_id)
    print(f"HTML (raw) [{len(result)}] Time taken:", time.time() - begin)

    # Confirm it complies with UTF-8
    try:
        result.decode('utf-8')
    except UnicodeDecodeError as e:
        print(f"HTML (raw) failed to decode: {e}")
        raise

    begin = time.time()
    size_of_renderer = lib.destroyRenderer(renderer_id)
    destroy_time = time.time() - begin
    print(f"Destroyed renderer [{size_of_renderer} bytes] in {destroy_time:.04f}")
    total_size_of_renderers += size_of_renderer

    begin = time.time()
    size_of_tree = lib.destroyTree(tree_id)
    destroy_time = time.time() - begin
    print(f"Destroyed tree [{size_of_tree} bytes] in {destroy_time:.04f}")
    total_size_of_trees += size_of_tree

    print("=" * 20)

print(f"All {len(files)} files processed in:", total_time)
print(f"Total size of all renderers: {total_size_of_renderers}")
print(f"Total size of all trees: {total_size_of_trees}")
