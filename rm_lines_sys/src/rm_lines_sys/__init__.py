import ctypes
import os
import sys
from logging import Logger
from typing import Optional, Annotated

logger = Logger("rm_lines_sys")
MODULE_FOLDER = os.path.dirname(os.path.abspath(__file__))


# noinspection PyPep8Naming
class LibAnnotations(ctypes.Structure):
    # Tree stuff
    def buildTree(self, file: bytes) -> bytes:
        pass
    def destroyTree(self, tree_id: bytes) -> int:
        pass
    def convertToJson(self, tree_id: bytes, json_file: bytes) -> bool:
        pass
    def getSceneInfo(self, tree_id: bytes) -> bytes:
        pass

    # Renderer stuff
    def makeRenderer(self, tree_id: bytes, page_type: int, landscape: bool) -> bytes:
        pass
    def destroyRenderer(self, renderer_id: bytes) -> int:
        pass
    def getParagraphs(self, renderer_id: bytes) -> bytes:
        pass

def load_lib() -> Optional[ctypes.CDLL]:
    lib_name = {
        'win32': 'rm_lines.dll',
        'linux': 'librm_lines.so',
        'darwin': 'librm_lines.dylib'
    }.get(sys.platform)

    if not lib_name:
        logger.error(f"Unsupported platform: {sys.platform}")
        return None

    lib_path = os.path.abspath(os.path.join(MODULE_FOLDER, lib_name))
    if not os.path.exists(lib_path):
        logger.error(f"Library file not found, path: {lib_path}")
        return None

    if sys.platform == 'win32':
        _lib = ctypes.WinDLL(lib_path)
    else:
        _lib = ctypes.CDLL(lib_path)

    # Add function signatures for tree

    # Function buildTree(int) -> str
    _lib.buildTree.argtypes = [ctypes.c_char_p]
    _lib.buildTree.restype = ctypes.c_char_p

    # Function destroyTree(int) -> int
    _lib.destroyTree.argtypes = [ctypes.c_char_p]
    _lib.destroyTree.restype = ctypes.c_int

    # Function convertToJson(str, int) -> bool
    _lib.convertToJson.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    _lib.convertToJson.restype = ctypes.c_bool

    # Function getSceneInfo(str) -> str
    _lib.getSceneInfo.argtypes = [ctypes.c_char_p]
    _lib.getSceneInfo.restype = ctypes.c_char_p

    # Add function signatures for renderer

    # Functon makeRenderer(str) -> str
    _lib.makeRenderer.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_bool]
    _lib.makeRenderer.restype = ctypes.c_char_p

    # Function destroyRenderer(str) -> int
    _lib.destroyRenderer.argtypes = [ctypes.c_char_p]
    _lib.destroyRenderer.restype = ctypes.c_int

    # Function getParagraphs(str) -> str
    _lib.getParagraphs.argtypes = [ctypes.c_char_p]
    _lib.getParagraphs.restype = ctypes.c_char_p

    return _lib


lib: Optional[LibAnnotations] = load_lib()

__all__ = ['lib']
