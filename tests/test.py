import colorama
import ctypes
import mmap
import os
import time
from colorama import Fore

os.makedirs('output', exist_ok=True)

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

if os.name == 'nt':
    # Windows-specific code
    lib = ctypes.WinDLL(os.path.join(script_folder, 'build', 'Debug',  'rm_lines.dll'))
else:
    # Unix-specific code (Linux, macOS)
    lib = ctypes.CDLL(os.path.join(script_folder, '..', 'cmake-build-debug', 'librm_lines.so'))

# Function signature: (int, size_t, int) -> size_t
lib.convertToSvg.argtypes = [ctypes.c_int, ctypes.c_int]
lib.convertToSvg.restype = ctypes.c_bool

lib.setLogger(python_logger)
lib.setErrorLogger(python_error_logger)
lib.setDebugLogger(python_debug_logger)

for file in os.listdir("files"):
    output_path = os.path.join('output', file.replace('.rm', '.svg'))
    with open(output_path, 'w') as f:
        pass
    print("Processing file:", file)
    with open(os.path.join("files", file), "r+b") as fin, open(output_path, "r+b") as fout:
        # Call the shared library (it will expand the output file)
        begin = time.time()
        success = lib.convertToSvg(fin.fileno(), fout.fileno())
        print(f"[{success}] Time taken:", time.time() - begin)