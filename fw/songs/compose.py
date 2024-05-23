from enum import Enum
from typing import Union, Tuple, List, NamedTuple
import sys
import os.path
import re
import argparse


class Tone(Enum):
    TONE_C = 0
    TONE_CS = 1
    TONE_D = 2
    TONE_DS = 3
    TONE_E = 4
    TONE_F = 5
    TONE_FS = 6
    TONE_G = 7
    TONE_GS = 8
    TONE_A = 9
    TONE_AS = 10
    TONE_B = 11


class NoteLength(Enum):
    WHOLE = 0
    HALF_DOT = 1
    HALF = 2
    QUARTER_DOT = 3
    QUARTER = 4
    EIGHTH_DOT = 5
    EIGHTH = 6
    SIXTEENTH = 7


class Note(NamedTuple):
    octave: int
    tone: Tone

    _SHORT_TONES = ('C', 'Cs', 'D', 'Ds', 'E', 'F', 'Fs', 'G', 'Gs', 'A', 'As', 'B')

    def note_id(self) -> int:
        return (self.octave * 12) + self.tone.value

    def sorn_name(self) -> str:
        return f"{self._SHORT_TONES[self.tone.value]}{self.octave}"


class NoteWithLength(NamedTuple):
    length: NoteLength
    note: Note


class Silence(NamedTuple):
    length: NoteLength


class Comment(NamedTuple):
    value: str


SongElementType = Union[NoteWithLength, Silence, Comment]


class ParsingError(RuntimeError):
    def __init__(self, pos: int):
        self._pos = pos
        super().__init__(f'Tone parsing error at character {pos}')

    def position(self) -> int:
        return self._pos


class SongParser:
    __slots__ = '_text', '_pos'

    _LENGTHS = {
        '1':  NoteLength.WHOLE,
        '2.': NoteLength.HALF_DOT,
        '2':  NoteLength.HALF,
        '4.': NoteLength.QUARTER_DOT,
        '4':  NoteLength.QUARTER,
        '8.': NoteLength.EIGHTH_DOT,
        '8':  NoteLength.EIGHTH,
        '16': NoteLength.SIXTEENTH,
    }

    _TONES = {
        'c': Tone.TONE_C,
        '#c': Tone.TONE_CS,
        'd': Tone.TONE_D,
        '#d': Tone.TONE_DS,
        'e': Tone.TONE_E,
        'f': Tone.TONE_F,
        '#f': Tone.TONE_FS,
        'g': Tone.TONE_G,
        '#g': Tone.TONE_GS,
        'a': Tone.TONE_A,
        '#a': Tone.TONE_AS,
        'b': Tone.TONE_B,
    }

    _ELEMENT_RE = re.compile(
        r"\s*(?:(//[^\n]*)\n|((?:1|2|4|8|16)\.?)(#?[cdefgab][0-9]|-))",
        re.MULTILINE
    )

    def __init__(self, text: str):
        self._text = text
        self._pos = 0

    def __iter__(self):
        self._pos = 0
        return self

    def __next__(self) -> SongElementType:
        element, parsed_len = self._parse_with_length()
        self._pos += parsed_len
        return element

    def _parse_with_length(self) -> Tuple[SongElementType, int]:
        found = self._ELEMENT_RE.search(self._text, self._pos)
        if found is None:
            raise StopIteration()
        parsed_length = len(found.group(0))
        present_groups = sum((1 << pos if group is not None else 0) for pos, group in enumerate(found.groups()))
        if 1 == present_groups:
            return self._parse_comment(found.group(1)), parsed_length
        elif 6 == present_groups:
            note_length = self._parse_note_length(found.group(2))
            param = found.group(3)
            if '-' == param:
                return Silence(note_length), parsed_length
            else:
                return (
                    NoteWithLength(
                        note_length,
                        Note(self._parse_octave(param[-1:]), self._parse_note_tone(param[:-1]))
                    ),
                    parsed_length
                )
        else:
            raise ParsingError(self._pos)

    def _parse_note_length(self, s: str) -> NoteLength:
        nl = self._LENGTHS.get(s)
        if None is nl:
            raise ParsingError(self._pos)
        return nl

    def _parse_note_tone(self, s: str) -> Tone:
        tone = self._TONES.get(s)
        if None is tone:
            raise ParsingError(self._pos)
        return tone

    def _parse_octave(self, s: str) -> int:
        try:
            return int(s)
        except ValueError:
            raise ParsingError(self._pos)

    def _parse_comment(self, s: str) -> Comment:
        if s.startswith('//'):
            return Comment(s)
        else:
            raise ParsingError(self._pos)


class NativeNote(NamedTuple):
    length: NoteLength
    diff: int
    original: Note


class NativeSetOctave(NamedTuple):
    octave: int


NativeSongElementType = Union[NativeNote, NativeSetOctave, Silence, Comment]


def convert_to_native(parser: SongParser) -> List[NativeSongElementType]:
    elements = []
    current_note = Note(-1, Tone.TONE_C)  # Ensure set octave at start
    for el in parser:
        if isinstance(el, NoteWithLength):
            diff = el.note.note_id() - current_note.note_id()
            if diff > 7 or diff < -8:
                current_note = Note(el.note.octave + (1 if el.note.tone.value > 7 else 0), Tone.TONE_C)
                elements.append(NativeSetOctave(current_note.octave))
                diff = el.note.note_id() - current_note.note_id()
            elements.append(NativeNote(el.length, diff, el.note))
            current_note = el.note
        elif isinstance(el, Silence) or isinstance(el, Comment):
            elements.append(el)
    return elements


def native_to_lines(native: List[NativeSongElementType]) -> List[str]:
    lines = []
    for el in native:
        if isinstance(el, NativeNote):
            lines.append(
                f"    MusicElement{{NoteLength::{el.length.name}, {el.diff}}},  // {el.original.sorn_name()}\n"
            )
        elif isinstance(el, NativeSetOctave):
            lines.append(f"    MusicElement::SetOctave({el.octave}),\n")
        elif isinstance(el, Silence):
            lines.append(f"    MusicElement::Silence(NoteLength::{el.length.name}),\n")
        elif isinstance(el, Comment):
            lines.append(f"    {el.value}\n")
    lines.append("\n    MusicElement::Terminate()\n")
    return lines


def open_file_for_reading(name: str):
    if '-' == name:
        return sys.stdin
    return open(name, "r")


def open_file_for_writing(name: str):
    if '-' == name:
        return sys.stdout
    return open(name, "w", newline='\n')


def main():
    arg_parser = argparse.ArgumentParser(description="Tool to convert songs to code")
    arg_parser.add_argument("source", metavar="SOURCE", help="Tune source file (*.tune)")
    arg_parser.add_argument(
        "-o", "--out", metavar="OUTPUT", dest="out", required=False, default='-',
        help="Tune code to be generated (*.inc)"
    )
    args = arg_parser.parse_args()

    song_text = open_file_for_reading(args.source).read()
    try:
        native = convert_to_native(SongParser(song_text))
    except ParsingError as e:
        line_no = song_text.count('\n', 0, e.position())
        print(f'Error on line {line_no}', file=sys.stderr)
        sys.exit(255)

    out_file = open_file_for_writing(args.out)
    out_file.write(f"\n\nconst uint8_t {os.path.splitext(os.path.basename(args.out))[0].upper()}[] PROGMEM = {{\n")
    out_file.writelines(native_to_lines(native))
    out_file.write("};\n")


if __name__ == "__main__":
    main()
