import ctypes
import os
import sys
from logging import Logger
from typing import Optional

logger = Logger("rm_lines_sys")
MODULE_FOLDER = os.path.dirname(os.path.abspath(__file__))


def load_lib() -> Optional[ctypes.CDLL]:
    lib_name = {
        'win32': 'rm_lines.dll',
        'linux': 'librm_lines.so',
        'darwin': 'librm_lines.dylib'
    }.get(sys.platform)

    if not lib_name:
        logger.error(f"Unsupported platform: {sys.platform}")
        return None

    lib_path = os.path.join(MODULE_FOLDER, lib_name)

    if not os.path.exists(lib_path):
        logger.error(f"Library file not found, path: {lib_path}")
        return None

    if sys.platform == 'win32':
        _lib = ctypes.WinDLL(lib_path)
    else:
        _lib = ctypes.CDLL(lib_path)

    # Add function signatures

    # Function buildTree(int) -> str
    _lib.buildTree.argtypes = [ctypes.c_int]
    _lib.buildTree.restype = ctypes.c_char_p

    # Function convertToJson(str, int) -> bool
    _lib.convertToJson.argtypes = [ctypes.c_char_p, ctypes.c_int]
    _lib.convertToJson.restype = ctypes.c_bool

    # Function convertToSvg(str, int) -> bool
    _lib.convertToSvg.argtypes = [ctypes.c_char_p, ctypes.c_int]
    _lib.convertToSvg.restype = ctypes.c_bool
    return _lib


lib: Optional[ctypes.CDLL] = load_lib()

__all__ = ['lib']
