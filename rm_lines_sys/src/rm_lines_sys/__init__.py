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

    def convertToJsonFile(self, tree_id: bytes, json_file: bytes) -> bool:
        pass

    def convertToJson(self, tree_id: bytes) -> bytes:
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

    def getLayers(self, renderer_id: bytes) -> bytes:
        pass

    def textToMdFile(self, renderer_id: bytes, md_file: bytes) -> bool:
        pass

    def textToMd(self, renderer_id: bytes) -> bytes:
        pass

    def textToTxtFile(self, renderer_id: bytes, md_file: bytes) -> bool:
        pass

    def textToTxt(self, renderer_id: bytes) -> bytes:
        pass

    def textToHtmlFile(self, renderer_id: bytes, html_file: bytes) -> bool:
        pass

    def textToHtml(self, renderer_id: bytes) -> bytes:
        pass

    def getFrame(self, renderer_id: bytes, data_buffer, data_size, x: int, y: int, frame_width: int, frame_height: int,
                 width: int, height: int, antialias: bool):
        pass

    def getSizeTracker(self, renderer_id: bytes, layer_id: bytes) -> bytes:
        """Get the size tracker for the library."""
        pass

    # Library control functions
    def setDebugMode(self, mode: bool):
        """Set debug mode for the library."""
        pass

    def getDebugMode(self) -> bool:
        """Get the current debug mode status."""
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

    # Function convertToJsonFile(str, int) -> bool
    _lib.convertToJsonFile.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    _lib.convertToJsonFile.restype = ctypes.c_bool

    # Function convertToJson(str) -> str
    _lib.convertToJson.argtypes = [ctypes.c_char_p]
    _lib.convertToJson.restype = ctypes.c_char_p

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

    # Function getLayers(str) -> str
    _lib.getLayers.argtypes = [ctypes.c_char_p]
    _lib.getLayers.restype = ctypes.c_char_p

    # Function textToMdFile(str, str) -> bool
    _lib.textToMdFile.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    _lib.textToMdFile.restype = ctypes.c_bool

    # Function textToMd(str) -> str
    _lib.textToMd.argtypes = [ctypes.c_char_p]
    _lib.textToMd.restype = ctypes.c_char_p

    # Function textToTxtFile(str, str) -> bool
    _lib.textToTxtFile.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    _lib.textToTxtFile.restype = ctypes.c_bool

    # Function textToTxt(str) -> str
    _lib.textToTxt.argtypes = [ctypes.c_char_p]
    _lib.textToTxt.restype = ctypes.c_char_p

    # Function textToHtmlFile(str, str) -> bool
    _lib.textToHtmlFile.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    _lib.textToHtmlFile.restype = ctypes.c_bool

    # Function textToHtml(str) -> str
    _lib.textToHtml.argtypes = [ctypes.c_char_p]
    _lib.textToHtml.restype = ctypes.c_char_p

    # Function getFrame(str, *, size_t, (x)int, (y)int, (fw)int, (fh)int, (w)int, (h)int, bool)
    _lib.getFrame.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t, ctypes.c_int,
                              ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_bool]

    # Function getSizeTracker(str, int, int) -> str
    _lib.getSizeTracker.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    _lib.getSizeTracker.restype = ctypes.c_char_p

    # Function setDebugMode(bool)
    _lib.setDebugMode.argtypes = [ctypes.c_bool]

    # Function getDebugMode() -> bool
    _lib.setDebugMode.restype = ctypes.c_bool

    return _lib


lib: Optional[LibAnnotations] = load_lib()

__all__ = ['lib']
