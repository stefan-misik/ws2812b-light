from __future__ import annotations
import collections.abc
import typing
__all__: list[str] = ['Animation', 'AnimationSlotName', 'AnimationStorage', 'DataType', 'LedState', 'LedStrip']
class Animation:
    def get_parameter(self, param_id: typing.SupportsInt) -> int | None:
        ...
    def render(self, led_strip: LedStrip) -> None:
        ...
    def restore(self, buffer: collections.abc.Buffer, type: DataType) -> int:
        ...
    def set_parameter(self, param_id: typing.SupportsInt, value: typing.SupportsInt) -> bool:
        ...
    def store(self, buffer: collections.abc.Buffer, type: DataType) -> int:
        ...
class AnimationSlotName:
    """
    Members:
    
      COLOR
    
      COLOR_LAST
    
      RAINBOW
    
      RAINBOW_LAST
    
      RETRO
    
      RETRO_LAST
    
      TWINKLE
    
      TWINKLE_LAST
    
      SHIFTING_COLOR
    
      SHIFTING_COLOR_LAST
    
      LIGHTS
    
      LIGHTS_LAST
    """
    COLOR: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.COLOR: 0>
    COLOR_LAST: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.COLOR: 0>
    LIGHTS: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.LIGHTS: 15>
    LIGHTS_LAST: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.LIGHTS_LAST: 26>
    RAINBOW: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.RAINBOW: 1>
    RAINBOW_LAST: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.RAINBOW: 1>
    RETRO: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.RETRO: 2>
    RETRO_LAST: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.RETRO_LAST: 5>
    SHIFTING_COLOR: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.SHIFTING_COLOR: 9>
    SHIFTING_COLOR_LAST: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.SHIFTING_COLOR_LAST: 14>
    TWINKLE: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.TWINKLE: 6>
    TWINKLE_LAST: typing.ClassVar[AnimationSlotName]  # value = <AnimationSlotName.TWINKLE_LAST: 8>
    __members__: typing.ClassVar[dict[str, AnimationSlotName]]  # value = {'COLOR': <AnimationSlotName.COLOR: 0>, 'COLOR_LAST': <AnimationSlotName.COLOR: 0>, 'RAINBOW': <AnimationSlotName.RAINBOW: 1>, 'RAINBOW_LAST': <AnimationSlotName.RAINBOW: 1>, 'RETRO': <AnimationSlotName.RETRO: 2>, 'RETRO_LAST': <AnimationSlotName.RETRO_LAST: 5>, 'TWINKLE': <AnimationSlotName.TWINKLE: 6>, 'TWINKLE_LAST': <AnimationSlotName.TWINKLE_LAST: 8>, 'SHIFTING_COLOR': <AnimationSlotName.SHIFTING_COLOR: 9>, 'SHIFTING_COLOR_LAST': <AnimationSlotName.SHIFTING_COLOR_LAST: 14>, 'LIGHTS': <AnimationSlotName.LIGHTS: 15>, 'LIGHTS_LAST': <AnimationSlotName.LIGHTS_LAST: 26>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class AnimationStorage:
    SLOT_COUNT: typing.ClassVar[int] = 27
    def __init__(self) -> None:
        ...
    def change(self, anim_id: typing.SupportsInt) -> bool:
        ...
    def get(self) -> Animation:
        ...
    def initialize(self) -> None:
        ...
    def initialize_current_slot(self) -> None:
        ...
    def slot_id(self) -> int:
        ...
class DataType:
    """
    Members:
    
      BOTH
    
      ONLY_CONFIG
    """
    BOTH: typing.ClassVar[DataType]  # value = <DataType.BOTH: 0>
    ONLY_CONFIG: typing.ClassVar[DataType]  # value = <DataType.ONLY_CONFIG: 1>
    __members__: typing.ClassVar[dict[str, DataType]]  # value = {'BOTH': <DataType.BOTH: 0>, 'ONLY_CONFIG': <DataType.ONLY_CONFIG: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class LedState:
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, red: typing.SupportsInt, green: typing.SupportsInt, blue: typing.SupportsInt) -> None:
        ...
    @typing.overload
    def __init__(self, hex_color: typing.SupportsInt) -> None:
        ...
    @property
    def blue(self) -> int:
        ...
    @blue.setter
    def blue(self, arg0: typing.SupportsInt) -> None:
        ...
    @property
    def green(self) -> int:
        ...
    @green.setter
    def green(self, arg0: typing.SupportsInt) -> None:
        ...
    @property
    def red(self) -> int:
        ...
    @red.setter
    def red(self, arg0: typing.SupportsInt) -> None:
        ...
class LedStrip:
    def __getitem__(self, led_id: typing.SupportsInt) -> LedState:
        ...
    def __init__(self) -> None:
        ...
    def __len__(self) -> int:
        ...
    @property
    def led_count(self) -> int:
        ...
