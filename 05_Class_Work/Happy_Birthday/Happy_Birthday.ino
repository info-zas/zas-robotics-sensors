// Happy Birthday - matches the accompanying sheet music in C major, 3/4.
// Kit1 buzzer signal: D12. Use a passive buzzer to play different pitches.
// Plays once on power-up or reset. No extra libraries required.

const byte BUZZER_PIN = 12;
const unsigned int TEMPO = 100; // quarter-note beats per minute

// Pitch names: the number is the octave; C4 is middle C. Values are Hz.
const unsigned int NOTE_G4 = 392;
const unsigned int NOTE_A4 = 440;
const unsigned int NOTE_B4 = 494;
const unsigned int NOTE_C5 = 523;
const unsigned int NOTE_D5 = 587;
const unsigned int NOTE_E5 = 659;
const unsigned int NOTE_F5 = 698;
const unsigned int NOTE_G5 = 784;

struct Note {
  unsigned int pitch;
  byte value; // 8 = eighth note, 4 = quarter note, 2 = half note
};

const Note melody[] = {
  {NOTE_G4, 8}, {NOTE_G4, 8},                   // pickup: Hap-py
  {NOTE_A4, 4}, {NOTE_G4, 4}, {NOTE_C5, 4},     // bar 1
  {NOTE_B4, 2}, {NOTE_G4, 8}, {NOTE_G4, 8},     // bar 2
  {NOTE_A4, 4}, {NOTE_G4, 4}, {NOTE_D5, 4},     // bar 3
  {NOTE_C5, 2}, {NOTE_G4, 8}, {NOTE_G4, 8},     // bar 4
  {NOTE_G5, 4}, {NOTE_E5, 4}, {NOTE_C5, 4},     // bar 5
  {NOTE_B4, 4}, {NOTE_A4, 4}, {NOTE_F5, 8}, {NOTE_F5, 8}, // bar 6
  {NOTE_E5, 4}, {NOTE_C5, 4}, {NOTE_D5, 4},     // bar 7
  {NOTE_C5, 2}                                 // bar 8: final note
};

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  const unsigned long wholeNoteMs = 240000UL / TEMPO;
  const unsigned int noteCount = sizeof(melody) / sizeof(melody[0]);
  for (unsigned int i = 0; i < noteCount; i++) {
    const unsigned long durationMs = wholeNoteMs / melody[i].value;
    // A short gap separates repeated pitches while keeping the rhythm.
    tone(BUZZER_PIN, melody[i].pitch, durationMs * 9 / 10);
    delay(durationMs);
    noTone(BUZZER_PIN);
  }
}

void loop() {
  // Press RESET to hear the tune again.
}
