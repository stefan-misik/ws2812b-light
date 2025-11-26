from abc import ABC, abstractmethod
from math import ceil
from dataclasses import dataclass
import tkinter as tk
from tkinter import ttk


@dataclass
class Color:
    red: int
    green: int
    blue: int


def blend(first: Color, second: Color, num: int, den: int) -> Color:
    POINT_POS = 16
    secondary_blend = (num << POINT_POS) // den
    primary_blend = (1 << POINT_POS) - secondary_blend
    return Color(
        (first.red * primary_blend + second.red * secondary_blend) >> POINT_POS,
        (first.green * primary_blend + second.green * secondary_blend) >> POINT_POS,
        (first.blue * primary_blend + second.blue * secondary_blend) >> POINT_POS,
    )


class Animation(ABC):
    @abstractmethod
    def render(self, lights: list[Color]):
        pass

    @abstractmethod
    def reset(self):
        pass


class ColorGridModel(ABC):
    @abstractmethod
    def list_animations(self) -> list[str] | tuple[str]:
        pass

    @abstractmethod
    def set_animation(self, anim_id: int):
        pass

    @abstractmethod
    def render(self, lights: list[Color]) -> int:
        pass

    @abstractmethod
    def reset(self):
        pass


class ColorGridView:
    _ROW_LENGTH = 10
    _ROW_COUNT = 10
    _LIGHT_COUNT = _ROW_LENGTH * _ROW_COUNT
    _INTERVAL = 8  # ms

    def __init__(self, root, model: ColorGridModel):
        self._model = model
        self._root = root
        self._root.title("Color Grid Application")
        self._lights = [Color(0, 0, 0) for _ in range(self._LIGHT_COUNT)]
        self._is_playing = False

        # Top Frame for controls
        top_frame = ttk.Frame(root)
        top_frame.pack(side=tk.TOP, fill=tk.X, pady=10)

        # Combobox for color schemes
        self._animation = ttk.Combobox(top_frame, values=model.list_animations(), state="readonly")
        self._animation.current(0)
        self._animation.bind("<<ComboboxSelected>>", self._on_animation_change)
        self._animation.pack(side=tk.LEFT, padx=5)

        self._frame_counter = tk.IntVar(value=0)
        ttk.Entry(top_frame, width=10, textvariable=self._frame_counter, state="readonly", justify=tk.RIGHT) \
            .pack(side=tk.RIGHT)
        ttk.Label(top_frame, text="Counter:").pack(side=tk.RIGHT, padx=5)

        # Mid Frame for controls
        mid_frame = ttk.Frame(root)
        mid_frame.pack(side=tk.TOP, fill=tk.X, pady=10)

        # Buttons
        apply_btn = ttk.Button(mid_frame, text="⏹", command=self._on_stop)
        apply_btn.pack(side=tk.LEFT, expand=True, anchor=tk.E, padx=5)

        reset_btn = ttk.Button(mid_frame, text="▶", command=self._on_play)
        reset_btn.pack(side=tk.LEFT, padx=5)

        random_btn = ttk.Button(mid_frame, text="⏸", command=self._on_pause)
        random_btn.pack(side=tk.LEFT, expand=True, anchor=tk.W, padx=5)

        # Canvas for circles
        self._canvas = tk.Canvas(root, width=400, height=400, bg="black", borderwidth=4, relief="groove")
        self._canvas.pack(pady=10)
        # Grid of circles
        self._light_circles = []
        padding = 10
        circle_radius = 15
        for i in range(ceil(self._ROW_COUNT)):
            for j in range(self._ROW_LENGTH):
                x = padding + j * (circle_radius * 2 + padding) + circle_radius
                y = padding + i * (circle_radius * 2 + padding) + circle_radius
                light = self._canvas.create_oval(
                    x - circle_radius, y - circle_radius,
                    x + circle_radius, y + circle_radius,
                    fill="black", outline="gray"
                )
                self._light_circles.append(light)
        self._render_frame()

    def _on_animation_change(self, event):
        self._model.set_animation(event.widget.current())
        self._render_frame()

    def _on_stop(self):
        self._is_playing = False
        self._model.reset()
        self._render_frame()

    def _on_play(self):
        if not self._is_playing:
            self._is_playing = True
            self._on_animate()
        else:
            self._is_playing = False

    def _on_pause(self):
        if self._is_playing:
            self._is_playing = False
        else:
            self._render_frame()

    def _on_animate(self):
        if self._is_playing:
            self._root.after(self._INTERVAL, self._on_animate)
            self._render_frame()

    def _render_frame(self):
        count = self._model.render(self._lights)
        self._update_lights()
        self._frame_counter.set(count)

    def _update_lights(self):
        for light_id, color in enumerate(self._lights):
            row = light_id // self._ROW_LENGTH
            row_start = row * self._ROW_LENGTH
            row_pos = light_id - row_start
            circle_pos = row_start + (row_pos if 0 == (0x1 & row) else self._ROW_LENGTH - row_pos - 1)
            self._canvas.itemconfig(
                self._light_circles[circle_pos], fill=f"#{color.red:02x}{color.green:02x}{color.blue:02x}"
            )


class ShiftingColorAnimation(Animation):
    @dataclass
    class Segment:
        color: Color
        length: int
        transition_length: int

    _SEQ = (
        (
            Segment(Color(0xFF, 0x00, 0x00), 25, 10),
            Segment(Color(0x00, 0xFF, 0x00), 25, 10),
            Segment(Color(0xC0, 0x5F, 0x00), 25, 10),
            Segment(Color(0x00, 0x00, 0xFF), 25, 10),
        ),
    )

    _FRACTION_BITS = 8


    class _SequenceWalker:
        def _make_segment(self):
            seg = self._seq[self._id]
            return ShiftingColorAnimation.Segment(
                seg.color,
                seg.length << ShiftingColorAnimation._FRACTION_BITS,
                seg.transition_length << ShiftingColorAnimation._FRACTION_BITS,
            )

        def __init__(self, seq: tuple["ShiftingColorAnimation.Segment"]):
            self._seq: tuple[ShiftingColorAnimation.Segment] = seq
            self._id = 0
            self._segment = self._make_segment()

        def start(self, offset: int) -> int:
            seg_len = sum(s.length for s in self._seq) << ShiftingColorAnimation._FRACTION_BITS
            offset = offset % seg_len

            # Rewind to correct segment
            remaining = offset
            seg_id = 0
            seg_len = 0
            while remaining > seg_len:
                remaining -= seg_len
                seg_id = seg_id - 1 if seg_id != 0 else len(self._seq) - 1
                seg_len = self._seq[seg_id].length << ShiftingColorAnimation._FRACTION_BITS
            self._id = seg_id
            self._segment = self._make_segment()
            self._segment.length = seg_len - remaining
            return offset

        def next(self) -> Color:
            pass


    def __init__(self, variant: int, speed: int = 3):
        self._variant = variant
        self._speed = speed
        self._offset = 0

    def render(self, lights: list[Color]):
        for n, cl in enumerate(lights):
            cl.red = 0xFF if n == self._offset else 0x00
            cl.green = 0x00
            cl.blue = 0x00
        self._offset = (self._offset + 1) % len(lights)

    def reset(self):
        self._offset = 0


class MyModel(ColorGridModel):
    _ANIMATIONS = (
        (
            "Shifting Color (0)",
            lambda: ShiftingColorAnimation(0)
        ),
    )

    def __init__(self):
        self._count = 0
        self._animation = self._ANIMATIONS[0][1]()

    def list_animations(self) -> list[str] | tuple[str]:
        return [name for name, _ in self._ANIMATIONS]

    def set_animation(self, anim_id: int):
        self._animation = self._ANIMATIONS[anim_id][1]()
        self._count = 0

    def render(self, lights: list[Color]) -> int:
        frame_id = self._count
        self._count += 1
        self._animation.render(lights)
        return frame_id

    def reset(self):
        self._animation.reset()
        self._count = 0


if __name__ == "__main__":
    root = tk.Tk()
    style = ttk.Style()
    style.theme_use('alt')
    app = ColorGridView(root, MyModel())
    root.mainloop()
