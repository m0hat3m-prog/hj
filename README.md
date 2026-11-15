# Maqam — quarter-tone maqam MIDI plugin (AU + VST3)

Maqam is a **MIDI effect**. Put it in front of any instrument (Serum, Zenology, Logic synths, samplers…). It turns what you play into a maqam, quarter tones included.

## What it does
- **20 maqamat:** Rast, Bayati, Husseini, Sikah, Huzam, Hijaz, Hijaz Kar, Saba, Saba Zamzam, Nahawand, Kurd, Ajam, Jiharkah, Nikriz, Nawa Athar, Athar Kurd, Suznak, Nairuz, Iraq, Rahat al-Arwah.
- **Tonic:** any of the 24 quarter-tone pitches. When you pick a maqam, the tonic jumps to its traditional one (Rast on C, Bayati on D, Sikah on E½♭…).
- **Keyboard modes**
  - *Arabic keys:* E plays E½♭, B plays B½♭ (like an Arabic keyboard). Keys outside the maqam stay normal.
  - *Snap every key:* every key snaps to the nearest maqam note, so you can't play a wrong note.
  - *White keys = degrees:* C D E F G A B play degrees 1–7 of the maqam. Black keys are muted.
- **Direction:** *Auto* switches to the descending form when your melody goes down (for example Rast's B½♭ becomes B♭, and Nahawand's B becomes B♭).
- **Output**
  - *MPE (polyphonic):* each note gets its own channel and pitch-bend, so chords stay in tune. Turn on MPE in the synth.
  - *Mono pitch-bend:* works with **any** synth. Set the synth's pitch-bend range to match (default ±2). Play one note at a time.
  - *No retune:* snaps to the maqam only, without quarter tones.
- **Quarter-tone depth:** 50 = exact 24-TET. Lower or raise it for regional intonation (for example Turkish-style or Iraqi-style neutral notes).
- **Click any degree** in the window to hear it.
- **Export .scl / .tun:** saves a tuning file for synths with built-in microtuning (Vital, Pigments, Surge, Kontakt, Serum 2…). You can then use the synth even without the plugin.

## Using it
- **Logic Pro:** on a Software Instrument track, open the **MIDI FX** slot and choose Audio Units → Maqam Tools → Maqam.
- **FL Studio:** add *Maqam* (VST3) to the Channel Rack. In its wrapper settings, set a **MIDI output port**, then set the same port as the **input port** on your synth.
- **Ableton Live:** Live doesn't pass MIDI out of VST/AU plugins. Use the **Export .scl** button and load the file in Live 12's *Tuning* system (or in your synth). The live plugin needs a Max for Live version.

## Building again
```
cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```
After the build, the plugins are copied to `~/Library/Audio/Plug-Ins/Components` and `~/Library/Audio/Plug-Ins/VST3`.
