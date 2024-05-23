

const uint8_t SILENT_NIGHT[] PROGMEM = {
    // 1
    MusicElement::SetOctave(4),
    MusicElement{NoteLength::QUARTER_DOT, 2},  // D4
    MusicElement{NoteLength::EIGHTH, 2},  // E4
    MusicElement{NoteLength::QUARTER, -2},  // D4
    // 2
    MusicElement{NoteLength::HALF_DOT, -3},  // B3
    MusicElement::Silence(NoteLength::QUARTER),
    // 3
    MusicElement{NoteLength::QUARTER_DOT, 3},  // D4
    MusicElement{NoteLength::EIGHTH, 2},  // E4
    MusicElement{NoteLength::QUARTER, -2},  // D4
    // 4
    MusicElement{NoteLength::HALF_DOT, -3},  // B3
    MusicElement::Silence(NoteLength::QUARTER),
    // 5
    MusicElement::SetOctave(5),
    MusicElement{NoteLength::HALF, -3},  // A4
    MusicElement{NoteLength::QUARTER, 0},  // A4
    // 6
    MusicElement{NoteLength::HALF_DOT, -3},  // Fs4
    MusicElement::Silence(NoteLength::QUARTER),
    // 7
    MusicElement{NoteLength::HALF, 1},  // G4
    MusicElement{NoteLength::QUARTER, 0},  // G4
    // 8
    MusicElement{NoteLength::HALF_DOT, -5},  // D4
    MusicElement::Silence(NoteLength::QUARTER),
    // 9
    MusicElement{NoteLength::HALF, 2},  // E4
    MusicElement{NoteLength::QUARTER, 0},  // E4
    // 10
    MusicElement{NoteLength::QUARTER_DOT, 3},  // G4
    MusicElement{NoteLength::EIGHTH, -1},  // Fs4
    MusicElement{NoteLength::QUARTER, -2},  // E4
    // 11
    MusicElement{NoteLength::QUARTER_DOT, -2},  // D4
    MusicElement{NoteLength::EIGHTH, 2},  // E4
    MusicElement{NoteLength::QUARTER, -2},  // D4
    // 12
    MusicElement{NoteLength::HALF_DOT, -3},  // B3
    MusicElement::Silence(NoteLength::QUARTER),
    // 13
    MusicElement{NoteLength::HALF, 5},  // E4
    MusicElement{NoteLength::QUARTER, 0},  // E4
    // 14
    MusicElement{NoteLength::QUARTER_DOT, 3},  // G4
    MusicElement{NoteLength::EIGHTH, -1},  // Fs4
    MusicElement{NoteLength::QUARTER, -2},  // E4
    // 15
    MusicElement{NoteLength::QUARTER_DOT, -2},  // D4
    MusicElement{NoteLength::EIGHTH, 2},  // E4
    MusicElement{NoteLength::QUARTER, -2},  // D4
    // 16
    MusicElement{NoteLength::HALF_DOT, -3},  // B3
    MusicElement::Silence(NoteLength::QUARTER),
    // 17
    MusicElement::SetOctave(5),
    MusicElement{NoteLength::HALF, -3},  // A4
    MusicElement{NoteLength::QUARTER, 0},  // A4
    // 18
    MusicElement{NoteLength::QUARTER, 3},  // C5
    MusicElement{NoteLength::QUARTER, -3},  // A4
    MusicElement{NoteLength::QUARTER, -3},  // Fs4
    // 19
    MusicElement{NoteLength::HALF_DOT, 1},  // G4
    // 20
    MusicElement{NoteLength::HALF_DOT, 4},  // B4
    MusicElement::Silence(NoteLength::QUARTER),
    // 21
    MusicElement{NoteLength::QUARTER, -4},  // G4
    MusicElement{NoteLength::QUARTER, -5},  // D4
    MusicElement{NoteLength::QUARTER, -3},  // B3
    // 22
    MusicElement{NoteLength::QUARTER, 3},  // D4
    MusicElement{NoteLength::QUARTER, -2},  // C4
    MusicElement{NoteLength::QUARTER, -3},  // A3
    // 23
    MusicElement{NoteLength::HALF_DOT, -2},  // G3

    MusicElement::Terminate()
};
