import ctypes
import os
import sys
from logging import Logger
from typing import Optional

logger = Logger("librm_lines_sys")


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_logger(msg):
    logger.info(msg.decode())


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_error_logger(msg):
    logger.error(msg.decode())


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_debug_logger(msg):
    logger.debug(msg.decode())


script_folder = os.path.dirname(os.path.abspath(__file__))


def load_lib() -> Optional[ctypes.CDLL]:
    lib_name = {
        'win32': 'rm_lines.dll',
        'linux': 'librm_lines.so',
        'darwin': 'librm_lines.dylib'
    }.get(sys.platform)

    if not lib_name:
        logger.error(f"Unsupported platform: {sys.platform}")
        return None

    lib_path = os.path.join(script_folder, lib_name)

    if not os.path.exists(lib_path):
        logger.error(f"Library file not found, path: {lib_path}")
        return None

    if sys.platform == 'win32':
        _lib = ctypes.WinDLL(lib_path)
    else:
        _lib = ctypes.CDLL(lib_path)

    # Function convertToSvg(int, int) -> size_t
    _lib.convertToSvg.argtypes = [ctypes.c_int, ctypes.c_int]
    _lib.convertToSvg.restype = ctypes.c_bool

    # Attach logging functions
    _lib.setLogger(python_logger)
    _lib.setErrorLogger(python_error_logger)
    _lib.setDebugLogger(python_debug_logger)
    return _lib


lib: Optional[ctypes.CDLL] = load_lib()

__all__ = ['lib']
