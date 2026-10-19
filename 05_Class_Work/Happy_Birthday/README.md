# Happy Birthday on Kit1

Open `Happy_Birthday.ino` in Arduino IDE. The sketch uses D12, matching the existing Harry Potter sketch. A passive buzzer is needed for changing musical pitches; an active buzzer may produce only its fixed tone. For an external passive piezo buzzer, connect + to D12 and - to GND. If using the kit's fitted buzzer, keep its existing connections.

Connect the Arduino by USB. In Arduino IDE select your actual board and port, click Verify, then Upload. The tune plays once after upload or reset. Press RESET to replay. Change TEMPO to adjust speed (larger = faster).

Print `../output/pdf/Happy_Birthday_Student_Sheet.pdf`. It matches all 25 notes in the sketch. The tune is in C major, in 3/4 time, with two eighth notes as a one-beat pickup. The final two-beat bar completes that pickup.

## From music to code

- Staff height determines pitch. G4 is lower than G5; C4 is middle C.
- NOTE_C5 means C in octave 5; its frequency is approximately 523 Hz.
- `{NOTE_G4, 8}` means play G4 for an eighth note (half a beat).
- `{NOTE_C5, 4}` means play C5 for a quarter note (one beat).
- `{NOTE_B4, 2}` means play B4 for a half note (two beats).
- At TEMPO = 100, one beat lasts 600 ms; an eighth lasts 300 ms.
- Each full bar has three beats. The short opening and ending bars together have three beats.

Teaching sequence: clap the rhythm, say the pitch names, find the matching entries in melody[], then upload and listen. Try changing one pitch and predict how the melody will change.

The supplied files have not been uploaded to hardware. Verify and Upload require your board and USB port.
