#!/usr/bin/env python3
"""
Simple preview generator for the `.tune` song sources found under `fw/shared/songs`.
"""
from array import array
from dataclasses import dataclass
from pathlib import Path
from typing import List, Union, Optional
import argparse
import importlib.util
import wave


TOOL_DIR = Path(__file__).resolve().parent
SONGS_DIR = TOOL_DIR.parent / 'fw' / 'shared' / 'songs'
COMPOSE_PATH = SONGS_DIR / 'compose.py'


def _load_compose_module():
    """Dynamically loads `compose.py` so its parser can be reused without duplicating it."""
    spec = importlib.util.spec_from_file_location('compose', COMPOSE_PATH)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


compose = _load_compose_module()


# Number of 1/32nd-note "ticks" per note length, matching the firmware's LENGTHS tables
# (see `fw/att85/music.cpp` and `fw/stm32g0/app/music.cpp`).
_LENGTH_TICKS = {
    compose.NoteLength.WHOLE: 32,
    compose.NoteLength.HALF_DOT: 16 + 8,
    compose.NoteLength.HALF: 16,
    compose.NoteLength.QUARTER_DOT: 8 + 4,
    compose.NoteLength.QUARTER: 8,
    compose.NoteLength.EIGHTH_DOT: 4 + 2,
    compose.NoteLength.EIGHTH: 4,
    compose.NoteLength.SIXTEENTH: 2,
}

# Reference note used to convert a note into a real-world frequency: A4 (440 Hz), which
# is `octave=4, tone=TONE_A` using the same octave/tone numbering as the `.tune` notation.
_REFERENCE_NOTE_ID = compose.Note(4, compose.Tone.TONE_A).note_id()
_REFERENCE_FREQUENCY = 440.0

# The firmware (see `DIVIDE_FACTOR` in `fw/att85/music.h` and `fw/stm32g0/app/music.hpp`)
# subdivides every tick into this many steps, and silences the buzzer one step before a
# note's duration actually elapses ("Stop the note just before end of last section" in
# `fw/att85/music.cpp` / `fw/stm32g0/app/music.cpp`). This carves out a short, fixed-size
# staccato gap at the end of every note, so consecutive notes never sound truly legato.
_DIVIDE_FACTOR = 8

SAMPLE_RATE = 44100


def note_frequency(note: compose.Note) -> float:
    """Converts a parsed note into its equal-tempered frequency, in Hz."""
    semitone_diff = note.note_id() - _REFERENCE_NOTE_ID
    return _REFERENCE_FREQUENCY * (2.0 ** (semitone_diff / 12.0))


@dataclass
class PlaybackStep:
    note: Optional[compose.Note]
    duration: float
    gap: float = 0.0


def load_song(path: Path) -> List[compose.NativeSongElementType]:
    """Parses a `.tune` file into its native element list, the same as `compose.py` does."""
    text = path.read_text()
    try:
        native = compose.convert_to_native(compose.SongParser(text))
    except compose.ParsingError as e:
        line_no = text.count('\n', 0, e.position())
        raise SystemExit(f"{path}: parsing error on line {line_no}")
    return native


def build_playback_sequence(elements: list, bpm: float) -> List[PlaybackStep]:
    """
    Walks the native element list the same way the firmware's `Music::play()` does
    (including loop repeats), producing a flat, linear sequence of notes/silences to play.
    """
    tick_duration = (60.0 / bpm) / _LENGTH_TICKS[compose.NoteLength.QUARTER]

    sequence = []
    loop_stack = []  # each entry: [remaining_count, body_start_index]
    position = 0
    while position < len(elements):
        el = elements[position]

        if isinstance(el, compose.NativeSetOctave):
            position += 1

        elif isinstance(el, compose.NativeNote):
            duration = _LENGTH_TICKS[el.length] * tick_duration
            # Short staccato pause, matching the firmware silencing the buzzer one step
            # before the note's duration actually elapses.
            gap = tick_duration / _DIVIDE_FACTOR
            sequence.append(
                PlaybackStep(el.original, duration, gap)
            )
            position += 1

        elif isinstance(el, compose.Silence):
            duration = _LENGTH_TICKS[el.length] * tick_duration
            sequence.append(PlaybackStep(None, duration))
            position += 1

        elif isinstance(el, compose.LoopControl):
            if el.is_end():
                if loop_stack and loop_stack[-1][0] > 0:
                    loop_stack[-1][0] -= 1
                    position = loop_stack[-1][1]
                else:
                    if loop_stack:
                        loop_stack.pop()
                    position += 1
            else:
                loop_stack.append([el.count, position + 1])
                position += 1

        elif isinstance(el, compose.Comment):
            position += 1

        else:
            raise SystemExit(f"Unexpected song element: {el!r}")

    return sequence


def render_samples(sequence: List[PlaybackStep], volume: float) -> array:
    """Synthesizes a square wave (similar to the buzzer's tone) for the given sequence."""
    samples = array('h')
    amplitude = int(32767 * max(0.0, min(1.0, volume)))
    for step in sequence:
        sample_count = max(1, round(step.duration * SAMPLE_RATE))
        sample_count_gap = max(0, round(step.gap * SAMPLE_RATE))
        if step.note is None:
            samples.extend(0 for _ in range(sample_count))
            continue
        period_samples = SAMPLE_RATE / note_frequency(step.note)
        samples.extend(
            (amplitude if (n % period_samples) < (period_samples // 2) else -amplitude) for n in \
                range(sample_count - sample_count_gap)
        )
        samples.extend(0 for _ in range(sample_count_gap))
    return samples


def write_wav(path: Path, samples: array) -> None:
    with wave.open(str(path), 'wb') as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(SAMPLE_RATE)
        wav_file.writeframes(samples.tobytes())


def print_sequence(name: str, sequence: list) -> None:
    print(f"== {name} ({len(sequence)} steps) ==")
    for step in sequence:
        name = step.note.sorn_name() if step.note is not None else "-"
        print(f"  {name:>4}  {step.duration * 1000:6.1f} ms")


def discover_songs() -> list:
    return sorted(SONGS_DIR.glob('*.tune'))


def resolve_song_path(name: str) -> Path:
    path = Path(name)
    if path.suffix != '.tune':
        path = path.with_suffix('.tune')
    if not path.is_file():
        candidate = SONGS_DIR / path.name
        if candidate.is_file():
            path = candidate
    if not path.is_file():
        raise SystemExit(f"Song not found: {name}")
    return path


def main() -> None:
    arg_parser = argparse.ArgumentParser(description=__doc__)
    arg_parser.add_argument(
        'song', nargs=1,
        help="Song(s) to play: `.tune` file path, or song name (e.g. `jingle_bells`) resolved "
             f"against {SONGS_DIR.relative_to(TOOL_DIR.parent)}. When omitted, all songs found "
             "there are played."
    )
    arg_parser.add_argument(
        '-t', '--tempo', type=float, default=120.0, metavar='BPM',
        help="Tempo, in quarter notes per minute (default: %(default)s)"
    )
    arg_parser.add_argument(
        '-v', '--volume', type=float, default=0.25, metavar='0..1',
        help="Playback volume, from 0.0 to 1.0 (default: %(default)s)"
    )
    arg_parser.add_argument(
        '-o', '--out', metavar='WAV', type=Path, default=None,
        help="Write the song's audio to a WAV file"
    )
    args = arg_parser.parse_args()

    if args.song:
        paths = [resolve_song_path(name) for name in args.song]
    else:
        raise SystemExit("No .tune files provided; use --help for usage information.")

    for path in paths:
        elements = load_song(path)
        sequence = build_playback_sequence(elements, args.tempo)
        print_sequence(path.stem, sequence)

        if args.out is None:
            continue

        samples = render_samples(sequence, args.volume)

        write_wav(args.out, samples)
        print(f"Wrote {args.out}")
        continue


if __name__ == '__main__':
    main()
