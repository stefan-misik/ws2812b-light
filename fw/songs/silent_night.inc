#include "song.h"


const uint8_t SILENT_NIGHT[] PROGMEM = {
    MusicElement::SetOctave(4),
    MusicElement{NoteLength::QUARTER_DOT, 2},  // D4
    MusicElement{NoteLength::EIGHTH, 2},  // E4
    MusicElement{NoteLength::QUARTER, -2},  // D4
    MusicElement{NoteLength::HALF, -3},  // B3
    MusicElement::Silence(NoteLength::HALF),
    MusicElement{NoteLength::QUARTER_DOT, 3},  // D4
    MusicElement{NoteLength::EIGHTH, 2},  // E4
    MusicElement{NoteLength::QUARTER, -2},  // D4
    MusicElement{NoteLength::HALF, -3},  // B3
    MusicElement::Silence(NoteLength::HALF),
    MusicElement::SetOctave(5),
    MusicElement{NoteLength::HALF, -3},  // A4
    MusicElement{NoteLength::QUARTER, 0},  // A4
    MusicElement{NoteLength::HALF, -3},  // Fs4
    MusicElement::Silence(NoteLength::HALF),
    MusicElement::Terminate()
};
