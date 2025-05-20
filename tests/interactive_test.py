import threading

from tests_base import *
import pygameextra as pe

pe.init()


class GC(pe.GameContext):
    AREA = (500, 500)
    MODE = pe.display.DISPLAY_MODE_RESIZABLE
    BACKGROUND = pe.colors.white

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
        self.index = 24
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
        lib.getFrame(renderer[1], self.buffer[2], self.buffer_size * 4, int(x), int(y), int(w / scale), int(h / scale),
                     w, h,
                     False)
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
