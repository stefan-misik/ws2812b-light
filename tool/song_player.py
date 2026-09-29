#!/usr/bin/env python3
"""
Simple player/tester for the `.tune` song sources found under `fw/shared/songs`.

This tool re-uses the exact same parser as `fw/shared/songs/compose.py`, so it always
understands the same notation as the firmware build (notes, silences, octave switches
and loops), without needing to build the firmware or the `animations` native extension.
It only operates on the `.tune` *source* files - the generated/"compiled" `.inc` files
are not involved at all.
"""
from array import array
from dataclasses import dataclass
from math import sin, pi
from pathlib import Path
from shutil import which
import argparse
import importlib.util
import subprocess
import sys
import tempfile
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

SAMPLE_RATE = 44100


def note_frequency(note: 'compose.Note') -> float:
    """Converts a parsed note into its equal-tempered frequency, in Hz."""
    semitone_diff = note.note_id() - _REFERENCE_NOTE_ID
    return _REFERENCE_FREQUENCY * (2.0 ** (semitone_diff / 12.0))


@dataclass
class PlaybackStep:
    name: str
    frequency: float  # 0.0 for silence
    duration_s: float


def load_song(path: Path) -> list:
    """Parses a `.tune` file into its native element list, the same as `compose.py` does."""
    text = path.read_text()
    try:
        native = compose.convert_to_native(compose.SongParser(text))
    except compose.ParsingError as e:
        line_no = text.count('\n', 0, e.position())
        raise SystemExit(f"{path}: parsing error on line {line_no}")
    # `Comment`s do not produce any code/bytes in the generated `.inc` file, so they must
    # be dropped here too, to keep loop jump targets consistent with the real firmware.
    return [el for el in native if not isinstance(el, compose.Comment)]


def build_playback_sequence(elements: list, bpm: float) -> list:
    """
    Walks the native element list the same way the firmware's `Music::play()` does
    (including loop repeats), producing a flat, linear sequence of notes/silences to play.
    """
    tick_duration_s = (60.0 / bpm) / _LENGTH_TICKS[compose.NoteLength.QUARTER]

    sequence = []
    loop_stack = []  # each entry: [remaining_count, body_start_index]
    position = 0
    while position < len(elements):
        el = elements[position]

        if isinstance(el, compose.NativeSetOctave):
            position += 1

        elif isinstance(el, compose.NativeNote):
            duration_s = _LENGTH_TICKS[el.length] * tick_duration_s
            sequence.append(PlaybackStep(el.original.sorn_name(), note_frequency(el.original), duration_s))
            position += 1

        elif isinstance(el, compose.Silence):
            duration_s = _LENGTH_TICKS[el.length] * tick_duration_s
            sequence.append(PlaybackStep('-', 0.0, duration_s))
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

        else:
            raise SystemExit(f"Unexpected song element: {el!r}")

    return sequence


def render_samples(sequence: list, volume: float) -> array:
    """Synthesizes a square wave (similar to the buzzer's tone) for the given sequence."""
    samples = array('h')
    amplitude = int(32767 * max(0.0, min(1.0, volume)))
    for step in sequence:
        sample_count = max(1, round(step.duration_s * SAMPLE_RATE))
        if step.frequency <= 0.0:
            samples.extend([0] * sample_count)
            continue
        period_samples = SAMPLE_RATE / step.frequency
        for n in range(sample_count):
            phase = (n % period_samples) / period_samples
            samples.append(amplitude if sin(2 * pi * phase) >= 0 else -amplitude)
    return samples


def write_wav(path: Path, samples: array) -> None:
    with wave.open(str(path), 'wb') as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(SAMPLE_RATE)
        wav_file.writeframes(samples.tobytes())


def play_wav(path: Path) -> bool:
    """Attempts to play a WAV file using whatever playback tool is available on the system."""
    try:
        import winsound
        winsound.PlaySound(str(path), winsound.SND_FILENAME)
        return True
    except ImportError:
        pass

    for player, extra_args in (('aplay', []), ('paplay', []), ('afplay', [])):
        if which(player):
            subprocess.run([player, *extra_args, str(path)], check=False)
            return True

    return False


def print_sequence(name: str, sequence: list) -> None:
    print(f"== {name} ({len(sequence)} steps) ==")
    for step in sequence:
        print(f"  {step.name:>4}  {step.frequency:7.2f} Hz  {step.duration_s * 1000:6.1f} ms")


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
        'song', nargs='*',
        help="Song(s) to play: `.tune` file path, or song name (e.g. `jingle_bells`) resolved "
             f"against {SONGS_DIR.relative_to(TOOL_DIR.parent)}. When omitted, all songs found "
             "there are played."
    )
    arg_parser.add_argument(
        '-t', '--tempo', type=float, default=120.0, metavar='BPM',
        help="Tempo, in quarter notes per minute (default: %(default)s)"
    )
    arg_parser.add_argument(
        '-v', '--volume', type=float, default=0.5, metavar='0..1',
        help="Playback volume, from 0.0 to 1.0 (default: %(default)s)"
    )
    arg_parser.add_argument(
        '-l', '--list', action='store_true',
        help="Only list the note/silence sequence, do not synthesize or play any audio"
    )
    arg_parser.add_argument(
        '-o', '--out', metavar='WAV', type=Path, default=None,
        help="Write the (first) song's audio to a WAV file instead of playing it directly"
    )
    args = arg_parser.parse_args()

    if args.song:
        paths = [resolve_song_path(name) for name in args.song]
    else:
        paths = discover_songs()
        if not paths:
            raise SystemExit(f"No .tune files found in {SONGS_DIR}")

    for path in paths:
        elements = load_song(path)
        sequence = build_playback_sequence(elements, args.tempo)
        print_sequence(path.stem, sequence)

        if args.list:
            continue

        samples = render_samples(sequence, args.volume)

        if args.out is not None:
            write_wav(args.out, samples)
            print(f"Wrote {args.out}")
            continue

        with tempfile.NamedTemporaryFile(suffix='.wav', delete=False) as tmp_file:
            tmp_path = Path(tmp_file.name)
        try:
            write_wav(tmp_path, samples)
            if not play_wav(tmp_path):
                print(
                    "No audio playback method found (tried winsound/aplay/paplay/afplay); "
                    f"use --out to save the audio to a file instead.",
                    file=sys.stderr
                )
        finally:
            tmp_path.unlink(missing_ok=True)


if __name__ == '__main__':
    main()
