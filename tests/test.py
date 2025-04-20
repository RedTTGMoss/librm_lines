import colorama
import ctypes
import mmap
import os
import shutil
import time
from colorama import Fore

colorama.init()


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_logger(msg):
    print(msg.decode(errors='replace'))


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_error_logger(msg):
    print(f"{Fore.RED}{msg.decode(errors='replace')}{Fore.RESET}")


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_debug_logger(msg):
    print(f"{Fore.LIGHTYELLOW_EX}{msg.decode(errors='replace')}{Fore.RESET}")


script_folder = os.path.dirname(os.path.abspath(__file__))
svg_output_folder = os.path.join(script_folder, 'output_svg')
json_output_folder = os.path.join(script_folder, 'output_json')
files_folder = os.path.join(script_folder, 'files')

os.makedirs(svg_output_folder, exist_ok=True)
os.makedirs(json_output_folder, exist_ok=True)

if os.name == 'nt':
    # Windows-specific code
    lib_path = os.path.join(script_folder, 'build', 'Debug', 'rm_lines.dll')
else:
    # Unix-specific code (Linux, macOS)
    lib_path = os.path.join(script_folder, '..', 'build', 'librm_lines.so')

shutil.copy(lib_path, os.path.join('..', 'rm_lines_sys', 'src', 'rm_lines_sys', os.path.basename(lib_path)))
os.chdir(os.path.join('..', 'rm_lines_sys'))
os.system(f'poetry install')
os.chdir(script_folder)

from rm_lines_sys import lib

lib.setLogger(python_logger)
lib.setErrorLogger(python_error_logger)
lib.setDebugLogger(python_debug_logger)

total_time = 0
for file in (files := os.listdir(files_folder)):
    svg_output_path = os.path.join(svg_output_folder, file.replace('.rm', '.svg'))
    json_output_path = os.path.join(json_output_folder, file.replace('.rm', '.json'))
    print("Processing file:", file)
    begin = time.time()
    tree_id = lib.buildTree(os.path.join(files_folder, file).encode()).decode()
    total_time += (process_time := time.time() - begin)
    print(f"[{tree_id}] Read, time taken:", process_time)
    if not tree_id:
        continue
    begin = time.time()
    success = lib.convertToJson(tree_id.encode(), json_output_path.encode())
    print(f"JSON [{success}] Time taken:", time.time() - begin)
    # Make a renderer

    begin = time.time()
    renderer_id = lib.makeRenderer(tree_id.encode(), 0, False).decode()
    if not renderer_id:
        continue

    total_time += (renderer_time := time.time() - begin)
    print(f"It took {renderer_time:.04f} to initialize the renderer")

    print("=" * 20)

print(f"All {len(files)} files processed in:", total_time)
