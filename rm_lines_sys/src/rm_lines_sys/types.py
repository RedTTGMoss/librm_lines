import ctypes


class C_CrdtId(ctypes.Structure):
    _fields_ = [
        ("first", ctypes.c_uint8),
        ("second", ctypes.c_uint64),
    ]


class C_RendererConfig(ctypes.Structure):
    _fields_ = [
        ("configVersion", ctypes.c_uint8),
        ("penWhitelist", ctypes.c_int8 * 20),
        ("penBlacklist", ctypes.c_int8 * 20),
        ("useWhitelist", ctypes.c_uint8),
        ("disabledLayers", C_CrdtId * 10),
        ("enableText", ctypes.c_bool),
        ("enableImages", ctypes.c_bool),
        ("enableGlyphHighlights", ctypes.c_bool),
    ]
