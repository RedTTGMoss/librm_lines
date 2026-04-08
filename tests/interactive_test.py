import atexit
import os.path
import threading
from typing import Optional, TYPE_CHECKING

from pygameextra import Rect

from tests_base import *
import pygameextra as pe

if TYPE_CHECKING:
    from rm_lines_sys.src.rm_lines_sys import LibAnnotations

pe.init()
lib: Optional["LibAnnotations"]
lib.setDebugMode(True)


class GC(pe.GameContext):
    AREA = (500, 500)
    MODE = pe.display.DISPLAY_MODE_RESIZABLE
    BACKGROUND = pe.colors.white
    TITLE = "Interactive Test"

    FPS_LOGGER = True
    LANDSCAPES = (
        'Landscape',
        'Scaling landscape',
    )
    TEMPLATES = (
        'Blank',
        'P Grid large',
        'P Grid medium',
        'P Grid small',
        'P Grid margin med',
        'P Grid margin large',
    )

    def __init__(self):
        self.items = []
        self.loaded = {}
        self.filenames = []
        self._index = 0
        self._scale = 0.4
        self.scale = 1
        self.buffer = (None, None, None)
        self.rect = None
        self.frame = None
        self.anchors = None
        self.paragraphs = None
        self.template_index = 0
        self.draggable = pe.Draggable((0, 0))
        self.text = pe.Text(colors=(pe.colors.white, pe.colors.black))

        for folder in (files_draw_folder, files_folder, files_color_folder, rm_output_folder):
            for filename in os.listdir(folder):
                file = os.path.join(folder, filename)
                self.items.append(file)
        self.items.sort(key=lambda x: os.path.basename(x))
        for item in self.items:
            filename = os.path.basename(item)
            self.filenames.append(filename[:-3].replace('_', ' ') + f' [{len(self.items)}]')
        self.index = 42
        if os.path.exists('pos'):
            try:
                with open('pos', 'r') as f:
                    self.index = int(f.read())
            except:
                pass
        atexit.register(self.save_index)
        super().__init__()
        self.sprite = pe.Sprite("rm_lines_cat.png", (100, 100))

    def save_index(self):
        with open('pos', 'w') as f:
            f.write(str(self.index))
        print(f"Saved index: {self.index}")

    def prepare_renderer(self, item: str, index: int):
        tree_id = lib.buildTree(item.encode())
        if not tree_id:
            print(f"Failed to build tree for {item}")
            return
        renderer_id = lib.makeRenderer(tree_id, 0, any(
            self.filenames[index].startswith(landscape) for landscape in self.LANDSCAPES
        ))
        if not renderer_id:
            print(f"Failed to make renderer for {item}")
            return
        self.loaded[item] = (tree_id, renderer_id)
        self.set_template(self.TEMPLATES[self.template_index])

    def get_renderer(self):
        renderer = self.loaded.get(self.item)

        if renderer is None:
            self.loaded[self.item] = (None, None)
            threading.Thread(target=self.prepare_renderer, args=(self.item, self.index), daemon=True).start()
            return None, None
        return renderer

    def handle_event(self, e: pe.event.Event):
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
        elif pe.event.key_DOWN(pe.K_UP):
            self.template_index -= 1
            if self.template_index < 0:
                self.template_index = len(self.TEMPLATES) - 1
            self.set_template(self.TEMPLATES[self.template_index])
        elif pe.event.key_DOWN(pe.K_DOWN):
            self.template_index += 1
            if self.template_index > len(self.TEMPLATES) - 1:
                self.template_index = 0
            self.set_template(self.TEMPLATES[self.template_index])
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
        rect = Rect(x, y, w, h)
        rect.x -= w / 2
        rect.y -= h / 2
        if scale != 1:
            rect.scale_by_ip(scale - 1, scale - 1)
        self.rect = rect
        lib.getFrame(
            renderer[1],  # Renderer ID
            self.buffer[2],  # Buffer
            self.buffer_size * 4,  # Buffer size in bytes
            *rect.topleft,  # Position
            *rect.size,  # Frame size
            w, h,  # Buffer size
            False
        )
        raw_frame = bytes(self.buffer[2])
        frame = pe.pygame.image.frombuffer(raw_frame, (w, h), 'RGBA')
        self.anchors = json.loads(lib.getAnchors(renderer[1]))
        self.paragraphs = json.loads(lib.getParagraphs(renderer[1]))
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

        x = self.centerx - offset[0]
        y = self.centery - offset[1]

        if self.frame is None or drag:
            self.frame = self.get_frame(
                x, y,
                *self.size, self.scale)
        if self.frame:
            pe.display.blit(self.frame)
            self.sprite.alpha = 100
        else:
            self.sprite.alpha = 255
        if self.anchors is not None:
            prev_pos = None
            prev_end = None
            for anchor_id, pos in self.anchors.items():
                pos -= 140  # remove top margin
                pos *= 1 - self.scale
                pe.draw.line(pe.colors.red, (0, pos), (150, pos), 3)
                if text := self.get_paragraph(anchor_id):
                    text = pe.Text(text, font_size=20, colors=[pe.colors.white, pe.colors.black])
                    if prev_pos is not None and abs(pos - prev_pos) < 20:
                        text.rect.midleft = (prev_end + 20, pos)
                    else:
                        text.rect.midleft = (160, pos)
                    text.position = text.rect.center
                    text.display()
                    prev_pos = pos
                    prev_end = text.rect.right
        self.sprite.display((self.width - 100, self.height - 100))

    def get_paragraph(self, anchor_id):
        for paragraph in self.paragraphs:
            if paragraph["startId"] == anchor_id:
                if len(paragraph["contents"]) > 0:
                    return paragraph["contents"][0]["text"]
        return None

    def set_template(self, template: str):
        renderer = self.get_renderer()
        if renderer[0] is None:
            return
        lib.setTemplate(renderer[1], template.encode())
        self.frame = None


gm = GC()

while True:
    gm()
