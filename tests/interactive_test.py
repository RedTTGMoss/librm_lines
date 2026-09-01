import atexit
import os.path
import threading
from typing import Optional, TYPE_CHECKING

from functools import lru_cache
from pygameextra import Rect

from tests_base import *
import pygameextra as pe

if TYPE_CHECKING:
    from rm_lines_sys.src.rm_lines_sys import LibAnnotations

pe.init((0, 0))
lib: Optional["LibAnnotations"]
lib.setDebugMode(True)


class ConfigEditor(pe.Context):
    AREA = (200, 550)
    BACKGROUND = (0, 0, 0, 50)
    FLOAT = pe.FLOAT_BOTTOMLEFT

    D_INACTIVE = (150, 100, 100, 200)
    D_ACTIVE = (100, 50, 50, 255)
    E_INACTIVE = (100, 150, 100, 200)
    E_ACTIVE = (50, 100, 50, 255)

    def __init__(self, reset_frame):
        super().__init__()
        self.config = None
        self.button_y = 10
        self.pen_x = 30
        self.pen_y = 10
        self.reset_frame = reset_frame
        self.icons = {}

        for icon_name in os.listdir(icons_folder):
            icon_path = os.path.join(icons_folder, icon_name)
            self.icons[icon_name.split('.')[0]] = pe.Sprite(icon_path, resize=(40, 40))

    @lru_cache
    def get_text(self, text: str, background: bool = False):
        return pe.Text(text, font_size=15, colors=(pe.colors.white, (100, 100, 100) if background else None))

    def bool_option(self, name: str, attr: str):
        if not self.config:
            return
        value = getattr(self.config.contents, attr)
        if value:
            pe.button.rect((10, self.button_y, self.width - 20, 30), self.E_INACTIVE, self.E_ACTIVE,
                           self.get_text(f"{name}: True"),
                           action=self.set_attr, data=(attr, False))
        else:
            pe.button.rect((10, self.button_y, self.width - 20, 30), self.D_INACTIVE, self.D_ACTIVE,
                           self.get_text(f"{name}: False"),
                           action=self.set_attr, data=(attr, True))
        self.button_y += 40

    def pen_icon(self, icon: str, pen: int, is_v2: bool):
        state = self.get_pen_state(pen)
        rect = (self.pen_x, self.pen_y, 40, 40)

        if state:
            pe.button.rect(rect, self.E_INACTIVE, self.E_ACTIVE, None,
                           action=self.set_pen_state, data=(pen, False))
        else:
            pe.button.rect(rect, self.D_INACTIVE, self.D_ACTIVE, None,
                           action=self.set_pen_state, data=(pen, True))

        self.icons[icon].display((self.pen_x, self.pen_y))
        if is_v2:
            text = self.get_text(f"V2", True)
        else:
            text = self.get_text(f"V1", True)
        text.rect.bottomright = (self.pen_x + 40, self.pen_y + 40)
        text.display()

        self.pen_x += 50
        if self.pen_x + 50 > self.width:
            self.pen_x = 30
            self.pen_y += 50

    def get_pen_state(self, pen: int):
        if not self.config:
            return None
        whitelist = self.config.contents.penWhitelist
        blacklist = self.config.contents.penBlacklist
        use_whitelist = self.config.contents.useWhitelist

        if use_whitelist:
            return pen in whitelist
        else:
            return pen not in blacklist

    def set_pen_state(self, pen: int, state: bool):
        if not self.config:
            return
        whitelist = self.config.contents.penWhitelist
        blacklist = self.config.contents.penBlacklist
        use_whitelist = self.config.contents.useWhitelist

        if use_whitelist:
            for i in range(len(whitelist)):
                if whitelist[i] == pen:
                    if not state:
                        whitelist[i] = -1
                    break
            else:
                if state:
                    for i in range(len(whitelist)):
                        if whitelist[i] == -1:
                            whitelist[i] = pen
                            break
        else:
            for i in range(len(blacklist)):
                if blacklist[i] == pen:
                    if state:
                        blacklist[i] = -1
                    break
            else:
                if not state:
                    for i in range(len(blacklist)):
                        if blacklist[i] == -1:
                            blacklist[i] = pen
                            break

        self.reset_frame()

    def set_attr(self, attr: str, value):
        if not self.config:
            return
        setattr(self.config.contents, attr, value)
        self.reset_frame()

    def loop(self):
        if not self.config:
            return
        self.button_y = 10
        self.pen_x = 30
        self.bool_option("Enable Text", "enableText")
        self.bool_option("Enable Images", "enableImages")
        self.bool_option("Enable Glyphs", "enableGlyphHighlights")
        self.bool_option("Enable Backdrop", "enableBackdrop")
        self.bool_option("Whitelist MODE", "useWhitelist")

        self.pen_y = self.button_y
        self.pen_icon('ballpoint', 2, False)
        self.pen_icon('fineliner', 4, False)
        self.pen_icon('highlighter', 5, False)
        self.pen_icon('pencil', 1, False)
        self.pen_icon('mechanical_pencil', 7, False)
        self.pen_icon('calligraphy', 21, True)
        self.pen_icon('marker', 3, False)
        self.pen_icon('shader', 23, True)
        self.pen_icon('paintbrush', 0, False)

        pe.draw.line(pe.colors.black, (0, self.pen_y), (self.width, self.pen_y), 2)
        self.pen_y += 10
        self.pen_icon('ballpoint', 15, True)
        self.pen_icon('fineliner', 17, True)
        self.pen_icon('highlighter', 18, True)
        self.pen_icon('pencil', 14, True)
        self.pen_icon('mechanical_pencil', 13, True)
        self.pen_icon('calligraphy', 21, True)
        self.pen_icon('marker', 16, True)
        self.pen_icon('shader', 23, True)
        self.pen_icon('paintbrush', 12, True)


class GC(pe.GameContext):
    AREA = (500, 500)
    MODE = pe.display.DISPLAY_MODE_RESIZABLE
    BACKGROUND = pe.colors.white
    TITLE = "Interactive Test"

    TEST_BACKDROP = False

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
        self.original_filenames = []
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
        self.debug_mode = True
        self.config_editor = ConfigEditor(self.reset_frame)

        if self.TEST_BACKDROP:
            self._test_backdrop_width = 500
            self._test_backdrop_height = 500
            self._test_backdrop_stride = self._test_backdrop_width * 4
            self._test_backdrop_buffer = bytearray(
                [230, 230, 230, 255] * (self._test_backdrop_width * self._test_backdrop_height))
            self._test_backdrop_ptr = (ctypes.c_uint8 * len(self._test_backdrop_buffer)).from_buffer(
                self._test_backdrop_buffer)

        for folder in (files_draw_folder, files_folder, files_color_folder, rm_output_folder):
            for filename in os.listdir(folder):
                file = os.path.join(folder, filename)
                self.items.append(file)
        self.items.sort(key=lambda x: os.path.basename(x))
        for item in self.items:
            filename = os.path.basename(item)
            self.original_filenames.append(filename[:-3])
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

    def reset_frame(self):
        self.frame = None

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
        config = lib.getConfig(renderer_id)
        if not renderer_id:
            print(f"Failed to make renderer for {item}")
            return
        self.loaded[item] = (tree_id, renderer_id, config)
        self.set_template(self.TEMPLATES[self.template_index])
        image_info = lib.getImageInfo(tree_id)
        if image_info:
            info = json.loads(image_info)
            for uuid, image in info.items():
                for name in os.listdir(images_folder):
                    if name != image['fileName']['value']:
                        continue
                    image_path = os.path.join(images_folder, name)
                    lib.addImage(renderer_id, uuid.encode(), image_path.encode())

        if self.TEST_BACKDROP:
            lib.setBackdrop(
                renderer_id,
                self._test_backdrop_ptr,
                len(self._test_backdrop_buffer),
                self._test_backdrop_width,
                self._test_backdrop_height,
                self._test_backdrop_stride
            )

    def get_renderer(self):
        renderer = self.loaded.get(self.item)

        if renderer is None:
            self.loaded[self.item] = (None, None, None)
            threading.Thread(target=self.prepare_renderer, args=(self.item, self.index), daemon=True).start()
            return None, None
        return renderer

    def handle_event(self, e: pe.event.Event):
        if pe.event.key_DOWN(pe.K_s):
            tree_id, renderer_id, _ = self.get_renderer()
            layers = lib.getLayers(renderer_id)
            layer_0 = json.loads(layers.decode())[0]['groupId']
            size_tracker_raw = lib.getSizeTracker(renderer_id, layer_0.encode())
            size_tracker = json.loads(size_tracker_raw.decode())
            x, y = size_tracker.get('l'), size_tracker.get('t')
            w = int(size_tracker.get('r') - size_tracker.get('l'))
            h = int(size_tracker.get('b') - size_tracker.get('t'))
            x += w / 2
            y += h / 2

            frame = self.get_frame(x, y, w, h, 1)
            overlay_file = f"debug_overlays/{self.original_filenames[self.index]}.png"
            save_output = f"output/png/{self.original_filenames[self.index]}.png"
            if os.path.exists(overlay_file):
                overlay_surface = pe.get_surface_file(overlay_file)
                with overlay_surface:
                    pe.fill.transparency(pe.colors.white, 100)
                    pe.display.blit(frame)
                    print("Using overlay!")
                overlay_surface.save_to_file(save_output)
                del overlay_surface
            else:
                pe.pygame.image.save(frame, save_output)
            del frame
        if pe.event.key_DOWN(pe.K_d):
            self.debug_mode = not self.debug_mode
            lib.setDebugMode(self.debug_mode)
            self.frame = None
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

    def get_frame(self, x, y, w, h, scale, antialias: bool = False):
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
            antialias
        )
        raw_frame = bytes(self.buffer[2])
        frame = pe.pygame.image.frombuffer(raw_frame, (w, h), 'RGBA')
        self.anchors = json.loads(lib.getAnchors(renderer[1]))
        self.paragraphs = json.loads(lib.getParagraphs(renderer[1]))
        return frame

    def resize(self, new_size):
        self.frame = None

    # def pre_loop(self):
    #     rect = pe.Rect(0, 0, *self.config_editor.size)
    #     rect.bottomleft = (0, self.height)
    #     self.config_editor.position = rect.topleft

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
        self.sprite.display((self.width - 100, self.height - 100))

        self.config_editor.config = self.get_renderer()[2]
        self.config_editor()

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
