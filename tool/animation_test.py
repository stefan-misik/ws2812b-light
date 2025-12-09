from abc import ABC, abstractmethod
from math import ceil
from itertools import chain
from functools import partial
import tkinter as tk
from tkinter import ttk
import animations


Color = animations.LedState


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
    def render(self, lights: animations.LedStrip):
        pass

    @abstractmethod
    def get_parameter(self, param_id: int) -> int | None:
        pass

    @abstractmethod
    def set_parameter(self, param_id: int, value: int) -> bool:
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
    def render(self, lights: animations.LedStrip) -> int:
        pass

    @abstractmethod
    def reset(self):
        pass

    @abstractmethod
    def animation(self) -> Animation:
        pass


class ColorGridView:
    _ROW_LENGTH = 20
    _INTERVAL = 8  # ms
    _BAD_PARAM_VALUE = -9999


    def __init__(self, root, model: ColorGridModel):
        self._model = model
        self._root = root
        self._root.title("Color Grid Application")
        self._led_strip = animations.LedStrip()
        self._is_playing = False

        # Root frame
        root_frame = ttk.Frame(root)
        root_frame.pack(side=tk.TOP, fill=tk.BOTH)

        # Top Frame for controls
        top_frame = ttk.Frame(root_frame)
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

        # Upper Frame for controls
        upper_frame = ttk.Frame(root_frame)
        upper_frame.pack(side=tk.TOP, fill=tk.X, pady=10)

        # Parameter
        ttk.Label(upper_frame, text="Parameters:", justify=tk.RIGHT) \
            .pack(side=tk.LEFT, expand=True, anchor=tk.E, padx=5)
        self._param_id_var = tk.IntVar(value=65536)
        param_id_sb = ttk.Spinbox(upper_frame, from_=0, to=(1 << 32) - 1, increment=1, \
            textvariable=self._param_id_var, command=self._on_param_id_change)
        param_id_sb.pack(side=tk.LEFT, expand=False, anchor=tk.W, padx=5)

        self._param_var = tk.IntVar(value=0)
        param_sb = ttk.Spinbox(upper_frame, from_=-(1 << 31), to=(1 << 31) - 1, increment=1, \
            textvariable=self._param_var, command=self._on_param_change)
        param_sb.pack(side=tk.LEFT, expand=True, anchor=tk.W, padx=5)

        # Mid Frame for controls
        mid_frame = ttk.Frame(root_frame)
        mid_frame.pack(side=tk.TOP, fill=tk.X, pady=10)

        # Buttons
        apply_btn = ttk.Button(mid_frame, text="⏹", command=self._on_stop)
        apply_btn.pack(side=tk.LEFT, expand=True, anchor=tk.E, padx=5)

        reset_btn = ttk.Button(mid_frame, text="▶", command=self._on_play)
        reset_btn.pack(side=tk.LEFT, padx=5)

        random_btn = ttk.Button(mid_frame, text="⏸", command=self._on_pause)
        random_btn.pack(side=tk.LEFT, expand=True, anchor=tk.W, padx=5)

        # Canvas for circles
        self._canvas = tk.Canvas(
            root_frame,
            width=40 * self._ROW_LENGTH, height=65 * ceil(len(self._led_strip) / self._ROW_LENGTH),
            bg="black", borderwidth=4, relief="groove"
        )
        self._canvas.pack(pady=10)
        # Grid of circles
        self._light_circles = []
        padding = 10
        ext_padding = 25
        circle_radius = 15
        for light_id in range(len(self._led_strip)):
            row = light_id // self._ROW_LENGTH
            row_start = row * self._ROW_LENGTH
            row_pos = light_id - row_start
            col = row_pos if 0 == (row & 0x1) else (self._ROW_LENGTH - 1 - row_pos)
            row_section = 0 if 0 == row_pos else (2 if (self._ROW_LENGTH - 1) == row_pos else 1)
            x = padding + col * (circle_radius * 2 + padding) + circle_radius
            y = padding + row * (circle_radius * 2 + (ext_padding + padding)) + circle_radius + \
                (row_section * ext_padding // 2)
            light = self._canvas.create_oval(
                x - circle_radius, y - circle_radius, x + circle_radius, y + circle_radius,
                fill="black", outline="gray"
            )
            self._light_circles.append(light)
        self._reload()

    def _on_animation_change(self, event):
        self._model.set_animation(event.widget.current())
        self._reload()

    def _on_param_id_change(self):
        self._read_param()

    def _on_param_change(self):
        success = self._model.animation().set_parameter(
            self._param_id_var.get(), self._param_var.get()
        )
        if not success:
            self._param_var.set(self._BAD_PARAM_VALUE)
        else:
            if not self._is_playing:
                self._render_frame()

    def _on_stop(self):
        self._is_playing = False
        self._model.reset()
        self._reload()

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

    def _reload(self):
        if not self._is_playing:
            self._render_frame()
        self._read_param()

    def _read_param(self):
        value = self._model.animation().get_parameter(self._param_id_var.get())
        if value is None:
            self._param_var.set(self._BAD_PARAM_VALUE)
            return
        self._param_var.set(value)

    def _render_frame(self):
        count = self._model.render(self._led_strip)
        self._update_lights()
        self._frame_counter.set(count)

    def _update_lights(self):
        for light_id, circle in enumerate(self._light_circles):
            color = self._led_strip[light_id]
            self._canvas.itemconfig(circle, fill=f"#{color.red:02x}{color.green:02x}{color.blue:02x}")


class NativeAnimation(Animation):
    def __init__(self, anim_id: int):
        self._anim_id = anim_id
        self._storage = animations.AnimationStorage()
        self._storage.change(self._anim_id)
        self._anim = self._storage.get()

    def render(self, lights):
        self._anim.render(lights)

    def get_parameter(self, param_id: int) -> int | None:
        return self._anim.get_parameter(param_id)

    def set_parameter(self, param_id: int, value: int) -> bool:
        return self._anim.set_parameter(param_id, value)

    def reset(self):
        data = bytearray(1024)
        data_len = self._anim.store(data, animations.DataType.ONLY_CONFIG)
        self._storage.change(self._anim_id)
        self._anim = self._storage.get()
        if data_len > 0:
            self._anim.restore((memoryview(data))[:data_len], animations.DataType.ONLY_CONFIG)


class MyModel(ColorGridModel):
    _ANIMATIONS = tuple(
        chain(
            ((f"Native animation {n}", partial(NativeAnimation, n)) for n in range(14)),
            ()
        )
    )

    def __init__(self):
        self._count = 0
        self._animation = self._ANIMATIONS[0][1]()

    def list_animations(self) -> list[str] | tuple[str]:
        return [name for name, _ in self._ANIMATIONS]

    def set_animation(self, anim_id: int):
        self._animation = self._ANIMATIONS[anim_id][1]()
        self._count = 0

    def render(self, lights: animations.LedStrip) -> int:
        frame_id = self._count
        self._count += 1
        self._animation.render(lights)
        return frame_id

    def reset(self):
        self._animation.reset()
        self._count = 0

    def animation(self) -> Animation:
        return self._animation


if __name__ == "__main__":
    root = tk.Tk()
    style = ttk.Style()
    style.theme_use('alt')
    app = ColorGridView(root, MyModel())
    root.mainloop()
