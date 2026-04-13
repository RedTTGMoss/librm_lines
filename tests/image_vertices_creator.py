from typing import Tuple

import numpy as np
import pygameextra as pe
from pygameextra import event

pe.init()

PAPER_SIZE = (1404, 1872)
IMG_SIZE = (100, 100)
IMG_WIDTH = IMG_SIZE[0]
IMG_HEIGHT = IMG_SIZE[1]


class GC(pe.GameContext):
    OFFSET = 50

    def __init__(self):
        super().__init__()

        self.ratio = IMG_HEIGHT / IMG_WIDTH

        self.rect = pe.Rect(0, 0, *PAPER_SIZE)
        self.scale = (self.height - self.OFFSET) / PAPER_SIZE[1]
        self.rect.scale_by_ip(self.scale, self.scale)
        self.rect.center = self.width / 2, self.height / 2

        self.area = self.rect.copy()
        self.area.height = self.area.width * self.ratio
        self.area.scale_by_ip(0.5, 0.5)
        self.area.center = self.rect.center

        self.rotation = -90

        self.draggable = pe.Draggable(self.area.center)

        self.ctrl_hold = False
        self.shift_hold = False

        self.free_mode = False
        self.running = True

    def handle_event(self, e: event.Event):
        super().handle_event(e)
        if pe.event.key_DOWN(pe.K_LEFT):
            self.rotation -= 5
        elif pe.event.key_DOWN(pe.K_RIGHT):
            self.rotation += 5

        if e.type == pe.MOUSEWHEEL:
            if self.ctrl_hold:
                amount = e.y / 20
            else:
                amount = e.y / 50
            if self.free_mode:
                if self.shift_hold:
                    self.area.scale_by_ip(1 + amount, 1)
                else:
                    self.area.scale_by_ip(1, 1 + amount)
            else:
                self.area.scale_by_ip(1 + amount, 1 + amount)

    def quit_check(self):
        self.running = False

    def pre_loop(self):
        super().pre_loop()
        mods = pe.pygame.key.get_mods()
        self.ctrl_hold = mods & pe.KMOD_CTRL
        self.shift_hold = mods & pe.KMOD_SHIFT

    def loop(self):
        pe.draw.line(pe.colors.darkaqua, self.rect.midleft, self.rect.midright, 2)
        pe.draw.line(pe.colors.darkaqua, self.rect.midtop, self.rect.midbottom, 2)

        pe.draw.rect(pe.colors.aqua, self.rect, 2)

        drag, pos = self.draggable.check()

        points = self.points(pos)
        top = pe.math.lerp(points[0], points[1], 0.5)

        pe.draw.line(pe.colors.darkred, points[1], points[3], 2)
        pe.draw.line(pe.colors.darkred, top, pos, 2)

        pe.draw.polygon(pe.colors.red, points, 4)

    def points(self, pos: Tuple[int, int]):
        # Account for rotation
        tsx_height = pe.TSX(pos, self.area.height / 2)
        top = tsx_height[self.rotation]
        bottom = tsx_height[self.rotation + 180]

        tsx_top = pe.TSX(top, self.area.width / 2)
        top_left = tsx_top[self.rotation - 90]
        top_right = tsx_top[self.rotation + 90]

        tsx_bottom = pe.TSX(bottom, self.area.width / 2)
        bottom_left = tsx_bottom[self.rotation - 90]
        bottom_right = tsx_bottom[self.rotation + 90]
        return [
            top_left,
            top_right,
            bottom_right,
            bottom_left,
        ]

    def points_final(self):
        raw = self.points(gc.draggable.pos)
        points = np.array(raw)

        # Subtract rect topleft
        points -= self.rect.topleft

        # Multiply by scale
        points /= self.scale

        # Zero x is as mid, so subtract half paper size
        points[:, 0] -= PAPER_SIZE[0] / 2

        return points.tolist()


if __name__ == "__main__":
    gc = GC()
    while gc.running:
        gc()
    points = gc.points_final()

    vertices = [
        *points[0],
        *points[1],
        *points[2],
        *points[3]
    ]

    print("{")
    for i in range(0, len(vertices), 2):
        r = [float(p) for p in vertices[i:i + 2]]
        print(f"    V{{{r[0]:.2f}, {r[1]:.2f}}},")
    print("}")
