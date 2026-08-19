import ctypes
import os
import sys
from logging import Logger
from typing import Optional, Annotated
from .types import C_CrdtId, C_RendererConfig

logger = Logger("rm_lines_sys")
MODULE_FOLDER = os.path.dirname(os.path.abspath(__file__))


# noinspection PyPep8Naming
class LibAnnotations(ctypes.Structure):
    # Tree stuff
    def buildTree(self, file: bytes) -> bytes:
        """Build the document tree from a file and return a tree ID."""
        pass

    def destroyTree(self, tree_id: bytes) -> int:
        """Destroy the document tree and free any associated resources."""
        pass

    def convertToJsonFile(self, tree_id: bytes, json_file: bytes) -> bool:
        """Convert the document tree to JSON and save it to a file."""
        pass

    def convertToJson(self, tree_id: bytes) -> bytes:
        """Convert the document tree to JSON bytes."""
        pass

    def getSceneInfo(self, tree_id: bytes) -> bytes:
        """Get the scene information, including paper size and other metadata."""
        pass

    def getImageInfo(self, tree_id: bytes) -> bytes:
        """Get the list of images with basic information, including image UUIDs and paths."""
        pass

    # Renderer stuff
    def makeRenderer(self, tree_id: bytes, page_type: int, landscape: bool) -> bytes:
        """Create a renderer for the given tree ID, page type, and orientation."""
        pass

    def destroyRenderer(self, renderer_id: bytes) -> int:
        """Destroy the renderer and free any associated resources."""
        pass

    def getParagraphs(self, renderer_id: bytes) -> bytes:
        """Get the list of paragraphs with full information, including text content and formatting."""
        pass

    def getAnchors(self, renderer_id: bytes) -> bytes:
        """Get the list of anchors with basic information"""
        pass

    def getLayers(self, renderer_id: bytes) -> bytes:
        """Get the list of layers with basic information"""
        pass

    def getLayerFull(self, renderer_id: bytes, layer_id: bytes) -> bytes:
        """Get the full layer information and items for a specific layer ID."""
        pass

    def textToMdFile(self, renderer_id: bytes, md_file: bytes) -> bool:
        """Export the text content of the document as markdown bytes to a file."""
        pass

    def textToMd(self, renderer_id: bytes) -> bytes:
        """Export the text content of the document as markdown bytes."""
        pass

    def textToTxtFile(self, renderer_id: bytes, txt_file: bytes) -> bool:
        """Export the text content of the document as plain text bytes to a file."""
        pass

    def textToTxt(self, renderer_id: bytes) -> bytes:
        """Export the text content of the document as plain text bytes."""
        pass

    def textToHtmlFile(self, renderer_id: bytes, html_file: bytes) -> bool:
        """Export the text content of the document as HTML bytes to a file."""
        pass

    def textToHtml(self, renderer_id: bytes) -> bytes:
        """Export the text content of the document as HTML bytes."""
        pass

    def getFrame(self, renderer_id: bytes, data_buffer, data_size, x: int, y: int, frame_width: int, frame_height: int,
                 width: int, height: int, antialias: bool):
        """Get the rendered frame for the given size and position, and store it in the provided data buffer."""
        pass

    def getConfig(self, renderer_id: bytes) -> C_RendererConfig:
        """Get a pointer to the internal renderer configuration structure."""
        pass

    def setTemplate(self, renderer_id: bytes, template: bytes):
        """Set the name of the template to apply to the rendered frame"""
        pass

    def getSizeTracker(self, renderer_id: bytes, layer_id: bytes) -> bytes:
        """Get the size tracker information for a specific layer ID."""
        pass

    def addImage(self, renderer_id: bytes, image_uuid: bytes, image_path: bytes):
        """Include an image file for an image UUID"""
        pass

    def setBackdrop(self, renderer_id: bytes, data_buffer, data_size, width: int, height: int, stride: int):
        """Set the backdrop image for the renderer."""
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

    # Function getImageInfo(str) -> str
    _lib.getImageInfo.argtypes = [ctypes.c_char_p]
    _lib.getImageInfo.restype = ctypes.c_char_p

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

    # Function getAnchors(str) -> str
    _lib.getAnchors.argtypes = [ctypes.c_char_p]
    _lib.getAnchors.restype = ctypes.c_char_p

    # Function getLayers(str) -> str
    _lib.getLayers.argtypes = [ctypes.c_char_p]
    _lib.getLayers.restype = ctypes.c_char_p

    # Function getLayerFull(str, str) -> str
    _lib.getLayerFull.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    _lib.getLayerFull.restype = ctypes.c_char_p

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

    # Function getConfig(str) -> C_RendererConfig
    _lib.getConfig.argtypes = [ctypes.c_char_p]
    _lib.getConfig.restype = ctypes.POINTER(C_RendererConfig)

    # Function setTemplate(str, str)
    _lib.setTemplate.argtypes = [ctypes.c_char_p, ctypes.c_char_p]

    # Function getSizeTracker(str, int, int) -> str
    _lib.getSizeTracker.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    _lib.getSizeTracker.restype = ctypes.c_char_p

    # Function addImage(str, str, str)
    _lib.addImage.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    _lib.addImage.restype = None

    # Function setBackdrop(void*, size_t, uint32_t, uint32_t, uint32_t)
    _lib.setBackdrop.argtypes = [
        ctypes.c_char_p,
        ctypes.c_void_p,
        ctypes.c_size_t,
        ctypes.c_uint32,
        ctypes.c_uint32,
        ctypes.c_uint32,
    ]
    _lib.setBackdrop.restype = None

    # Function setDebugMode(bool)
    _lib.setDebugMode.argtypes = [ctypes.c_bool]

    # Function getDebugMode() -> bool
    _lib.getDebugMode.restype = ctypes.c_bool

    return _lib


lib: Optional[LibAnnotations] = load_lib()

__all__ = ['lib', 'LibAnnotations']
