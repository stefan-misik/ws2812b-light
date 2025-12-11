from abc import ABC, abstractmethod
from math import ceil, floor
from struct import Struct
from itertools import chain
from functools import partial
import tkinter as tk
from tkinter import ttk
import time
from typing import Any, Tuple, Iterable
import animations


class PeriodicTimer:
    __slots__ = '_start',

    NOT_ELAPSED = 0
    DONE = 1
    MORE_PENDING = 2

    @staticmethod
    def _time() -> int:
        return floor(time.perf_counter() * 1000.0) & 0xFFFFFFFF

    def __init__(self):
        self._start = self._time()

    def reset(self, diff: int = 0) -> None:
        self._start = self._time() + diff

    def has_elapsed(self, period: int) -> int:
        diff = (self._time() - self._start) & 0xFFFFFFFF
        if diff < period:
            return self.NOT_ELAPSED
        self._start = (self._start + period) & 0xFFFFFFFF
        return self.DONE if diff < (2 * period) else self.MORE_PENDING


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
    def status(self) -> str:
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
    _POLLING_INTERVAL = 3  # ms (must be shorter than actual animation interval)
    _INTERVAL = 8  # ms
    _BAD_PARAM_VALUE = -9999


    def __init__(self, root, model: ColorGridModel):
        self._model = model
        self._root = root
        self._root.title("Color Grid Application")
        self._led_strip = animations.LedStrip()
        self._is_playing = False
        self._periodic_timer = PeriodicTimer()

        style = ttk.Style()

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

        # Status text
        self._status_txt = tk.Text(root_frame, width=90, height=5, state=tk.DISABLED)
        self._status_txt.config(
            bg=style.lookup('TLabel', 'background') or self._status_txt.cget('bg'),
            fg=style.lookup('TLabel', 'foreground') or self._status_txt.cget('fg')
        )
        self._status_txt.pack(expand=True, pady=5)

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
            self._periodic_timer.reset(-self._INTERVAL)  # Ensure the frame is immediately rendered
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
            self._root.after(self._POLLING_INTERVAL, self._on_animate)
            while True:
                timer_result = self._periodic_timer.has_elapsed(self._INTERVAL)
                if PeriodicTimer.NOT_ELAPSED == timer_result:
                    break
                # Only render the frame onto the GUI when current frame is last one for the time
                self._render_frame(do_update=(PeriodicTimer.DONE == timer_result))

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

    def _render_frame(self, do_update: bool = True):
        count = self._model.render(self._led_strip)
        if do_update:
            self._update_lights()
            self._frame_counter.set(count)
            self._update_status()

    def _update_status(self):
        colors = " ".join(
            f"#{color.red:02X}{color.green:02X}{color.blue:02X}" for color in (self._led_strip[n] for n in range(9))
        )
        anim_status = self._model.animation().status()
        self._set_status("\n".join((anim_status, colors)) if len(anim_status) > 0 else colors)

    def _update_lights(self):
        for light_id, circle in enumerate(self._light_circles):
            color = self._led_strip[light_id]
            self._canvas.itemconfig(circle, fill=f"#{color.red:02x}{color.green:02x}{color.blue:02x}")

    def _set_status(self, text: str):
        self._status_txt.config(state=tk.NORMAL)
        self._status_txt.delete(0.0, tk.END)
        self._status_txt.insert(0.0,text)
        self._status_txt.config(state=tk.DISABLED)


class NativeAnimation(Animation):
    class StateObserver(ABC):
        @abstractmethod
        def observe(self, state: memoryview) -> str:
            pass


    class StructStateObserver(StateObserver):
        def __init__(self, structs: Tuple[Struct, ...], templates: Tuple[str, ...]) -> None:
            self._structs = structs
            self._templates = templates

        def observe(self, state: memoryview) -> str:
            def unpack_all(structs: Tuple[Struct, ...], data: memoryview) -> Iterable[Tuple[Any, ...]]:
                offset = 0
                for s in structs:
                    yield s.unpack_from(data, offset)
                    offset += s.size
            return ", ".join(
                template.format(value) for template, value in \
                    zip(self._templates, chain(*unpack_all(self._structs, state)))
            )

    class ShiftingColorAnimationStateObserver(StateObserver):
        _FRACTION_BITS = 4
        _CONFIG_STRUCT = Struct("@BB")
        _STATE_STRUCT = Struct("@I")

        def observe(self, state: memoryview) -> str:
            variant, speed = self._CONFIG_STRUCT.unpack_from(state, 0)
            offset, = self._STATE_STRUCT.unpack_from(state, self._CONFIG_STRUCT.size)
            return f"Variant: {variant}, Speed: {speed}, " \
                f"Offset: {offset >> self._FRACTION_BITS:3} + " \
                f"{offset & ((1 << self._FRACTION_BITS) - 1):3}/{1 << self._FRACTION_BITS}"


    class DummyStateObserver(StateObserver):
        def observe(self, state: memoryview) -> str:
            return " ".join(f"{x:02X}" for x in state)

    @classmethod
    def _make_observer(cls, anim_id: int) -> StateObserver:
        if 0 == anim_id:
            return cls.StructStateObserver(
                (Struct("@B"), ),
                ("color ID: {}", )
            )
        elif 1 == anim_id:
            return cls.StructStateObserver(
                (Struct("@BBH"), ),
                ("Space increment: {}", "Time Increment: {}", "Hue: {}")
            )
        elif 2 <= anim_id <= 5:
            return cls.StructStateObserver(
                (Struct("@BB"), ),
                ("Variant: {}", "State: {}")
            )
        elif 8 <= anim_id <= 13:
            return cls.ShiftingColorAnimationStateObserver()
        else:
            return cls.DummyStateObserver()

    def __init__(self, anim_id: int):
        self._state = bytearray(1024)
        self._anim_id = anim_id
        self._observer = self._make_observer(anim_id)
        self._storage = animations.AnimationStorage()
        self._storage.change(self._anim_id)
        self._anim = self._storage.get()

    def render(self, lights):
        self._anim.render(lights)

    def get_parameter(self, param_id: int) -> int | None:
        return self._anim.get_parameter(param_id)

    def set_parameter(self, param_id: int, value: int) -> bool:
        return self._anim.set_parameter(param_id, value)

    def status(self) -> str:
        data_len = self._anim.store(self._state, animations.DataType.BOTH)
        return self._observer.observe((memoryview(self._state))[:data_len])

    def reset(self):
        data_len = self._anim.store(self._state, animations.DataType.ONLY_CONFIG)
        self._storage.change(self._anim_id)
        self._anim = self._storage.get()
        if data_len > 0:
            self._anim.restore((memoryview(self._state))[:data_len], animations.DataType.ONLY_CONFIG)


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
