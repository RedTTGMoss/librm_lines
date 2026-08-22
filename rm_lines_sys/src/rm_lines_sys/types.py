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
        ("useWhitelist", ctypes.c_bool),
        ("disabledLayers", C_CrdtId * 10),
        ("enableText", ctypes.c_bool),
        ("enableImages", ctypes.c_bool),
        ("enableGlyphHighlights", ctypes.c_bool),
        ("enableBackdrop", ctypes.c_bool),
        ("useBackdropForSamplingOnly", ctypes.c_bool),
        ("followRulesInJson", ctypes.c_bool),
        ("backdropOffsetX", ctypes.c_float),
        ("backdropOffsetY", ctypes.c_float),
        ("backdropAlign", ctypes.c_uint8),
    ]
