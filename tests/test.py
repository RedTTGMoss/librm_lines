import colorama
import ctypes
import mmap
import os
import time
from colorama import Fore

colorama.init()


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_logger(msg):
    print(msg.decode())


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_error_logger(msg):
    print(f"{Fore.RED}{msg.decode()}{Fore.RESET}")


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_debug_logger(msg):
    print(f"{Fore.LIGHTYELLOW_EX}{msg.decode()}{Fore.RESET}")


script_folder = os.path.dirname(os.path.abspath(__file__))
svg_output_folder = os.path.join(script_folder, 'output_svg')
json_output_folder = os.path.join(script_folder, 'output_json')
files_folder = os.path.join(script_folder, 'files')

os.makedirs(svg_output_folder, exist_ok=True)
os.makedirs(json_output_folder, exist_ok=True)

if os.name == 'nt':
    # Windows-specific code
    lib = ctypes.WinDLL(os.path.join(script_folder, 'build', 'Debug', 'rm_lines.dll'))
else:
    # Unix-specific code (Linux, macOS)
    lib = ctypes.CDLL(os.path.join(script_folder, '..', 'build', 'librm_lines.so'))

lib.convertToSvg.argtypes = [ctypes.c_char_p, ctypes.c_int]
lib.convertToSvg.restype = ctypes.c_bool

lib.convertToJson.argtypes = [ctypes.c_char_p, ctypes.c_int]
lib.convertToJson.restype = ctypes.c_bool

lib.buildTree.argtypes = [ctypes.c_int]
lib.buildTree.restype = ctypes.c_char_p

# lib.setLogger(python_logger)
# lib.setErrorLogger(python_error_logger)
# lib.setDebugLogger(python_debug_logger)

begin_all = time.time()
for file in (files := os.listdir(files_folder)):
    svg_output_path = os.path.join(svg_output_folder, file.replace('.rm', '.svg'))
    json_output_path = os.path.join(json_output_folder, file.replace('.rm', '.json'))
    with open(svg_output_path, 'w') as _:
        pass
    with open(json_output_path, 'w') as _:
        pass
    print("Processing file:", file)
    with open(os.path.join(files_folder, file), "r+b") as fin:
        print(fn := fin.fileno())
        begin = time.time()
        tree_id = lib.buildTree(fn).decode()
        print(f"[{tree_id}] Read, time taken:", time.time() - begin)
    if not tree_id:
        continue
    with open(json_output_path, "r+b") as fout:
        begin = time.time()
        success = lib.convertToJson(tree_id.encode(), fout.fileno())
        print(f"JSON [{success}] Time taken:", time.time() - begin)
    with open(svg_output_path, "r+b") as fout:
        begin = time.time()
        success = lib.convertToSvg(tree_id.encode(), fout.fileno())
        print(f"SVG [{success}] Time taken:", time.time() - begin)
    print("=" * 20)

print(f"All {len(files)} files processed in:", time.time() - begin_all)
