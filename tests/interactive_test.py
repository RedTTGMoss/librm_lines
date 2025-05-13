import ctypes
import io
import os
import shutil
import sys
import threading

import pygameextra as pe
from colorama import Fore
from pygameextra import event

pe.init()


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_logger(msg):
    print(msg.decode('utf-8', errors='replace'))


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_error_logger(msg):
    print(f"{Fore.RED}{msg.decode('utf-8', errors='replace')}{Fore.RESET}")


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_debug_logger(msg):
    print(f"{Fore.LIGHTYELLOW_EX}{msg.decode('utf-8', errors='replace')}{Fore.RESET}")


script_folder = os.path.dirname(os.path.abspath(__file__))
output_folder = os.path.join(script_folder, 'draw_output')
rm_lines_sys_src_path = os.path.join(script_folder, '..', 'rm_lines_sys', 'src')
sys.path.append(rm_lines_sys_src_path)
files_draw_folder = os.path.join(script_folder, 'draw_files')
files_folder = os.path.join(script_folder, 'files')

os.makedirs(output_folder, exist_ok=True)

if os.name == 'nt':
    # Windows-specific code
    for sub_path in ('', 'Debug', 'Release'):
        lib_path = os.path.join(os.path.dirname(script_folder), 'build', sub_path, 'rm_lines.dll')
        if os.path.exists(lib_path):
            break
    sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding='utf-8', errors='replace')
else:
    # Unix-specific code (Linux, macOS)
    lib_path = os.path.join(os.path.dirname(script_folder), 'build', 'librm_lines.so')

shutil.copy(lib_path, copy_to := os.path.join('..', 'rm_lines_sys', 'src', 'rm_lines_sys', os.path.basename(lib_path)))
print(f"Copied the dynamic library from {lib_path} to {os.path.realpath(copy_to)} for {os.name}")

from rm_lines_sys import lib

lib.setLogger(python_logger)
lib.setErrorLogger(python_error_logger)
lib.setDebugLogger(python_debug_logger)


class GC(pe.GameContext):
    AREA = (500, 500)
    MODE = pe.display.DISPLAY_MODE_RESIZABLE
    BACKGROUND = pe.colors.whitesmoke

    FPS_LOGGER = True

    def __init__(self):
        self.items = []
        self.loaded = {}
        self.filenames = []
        self._index = 0
        self._scale = 0.4
        self.scale = 1
        self.buffer = (None, None, None)
        self.frame = None
        self.draggable = pe.Draggable((0, 0))
        self.text = pe.Text(colors=(pe.colors.white, pe.colors.black))

        for folder in (files_draw_folder, files_folder):
            for filename in os.listdir(folder):
                self.filenames.append(filename[:-3].replace('_', ' ') + f' [{len(self.items)}]')
                file = os.path.join(folder, filename)
                self.items.append(file)
        self.index = 8
        super().__init__()
        self.sprite = pe.Sprite("rm_lines_cat.png", (100, 100))

    def prepare_renderer(self):
        tree_id = lib.buildTree(self.item.encode())
        renderer_id = lib.makeRenderer(tree_id, 0, False)
        self.loaded[self.item] = (tree_id, renderer_id)

    def get_renderer(self):
        renderer = self.loaded.get(self.item)

        if renderer is None:
            threading.Thread(target=self.prepare_renderer, daemon=True).start()
            # self.prepare_renderer()
            self.loaded[self.item] = (None, None)
            return None, None
        return renderer

    def handle_event(self, e: event.Event):
        if pe.event.key_DOWN(pe.K_RIGHT):
            self.index += 1
            if self.index >= len(self.items):
                self.index = 0
            self.frame = None
        elif pe.event.key_DOWN(pe.K_LEFT):
            self.index -= 1
            if self.index < 0:
                self.index = len(self.items) - 1
            self.frame = None
        if e.type == pe.MOUSEWHEEL:
            self._scale += e.y * self.delta_time
        super().handle_event(e)

    @property
    def index(self):
        return self._index

    @index.setter
    def index(self, value):
        self._index = value
        self.draggable.pos = (0, 0)
        try:
            self.text.text = self.filenames[self._index]
            self.text.init()
            self.text.rect.topleft = (0, 0)
        except IndexError:
            pass

    @property
    def item(self):
        return self.items[self.index]

    @property
    def centerx(self):
        return self.width / 2

    @property
    def centery(self):
        return self.height / 2

    @property
    def buffer_size(self):
        return self.buffer[0] * self.buffer[1]

    def get_frame(self, x, y, w, h, scale):
        # x, y = (
        #     x - (w * scale) * 0.5,
        #     y - (h * scale) * 0.5,
        # )
        renderer = self.get_renderer()
        if renderer[0] is None:
            return None
        if self.buffer[0] != w or self.buffer[1] != h:
            buffer_size = w * h
            self.buffer = (w, h, (ctypes.c_uint32 * buffer_size)())
        lib.getFrame(renderer[1], self.buffer[2], self.buffer_size * 4, int(x), int(y), w, h, scale)
        raw_frame = bytes(self.buffer[2])
        frame = pe.pygame.image.frombuffer(raw_frame, (w, h), 'RGBA')
        return frame

    def resize(self, new_size):
        self.frame = None

    def loop(self):
        self.text.display()

        delta = (self._scale - self.scale)
        if abs(delta) > 0.01:
            self.scale += delta * min(0.1, self.delta_time) * 10
            self.frame = None
        drag, offset = self.draggable.check()

        if self.frame is None or drag:
            self.frame = self.get_frame(
                self.centerx + offset[0], self.centery + offset[1],
                *self.size, self.scale)
        if self.frame:
            pe.display.blit(self.frame)
            self.sprite.alpha = 100
        else:
            self.sprite.alpha = 255
        self.sprite.display((self.width - 100, self.height - 100))


gm = GC()

while True:
    gm()
