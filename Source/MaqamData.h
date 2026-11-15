#pragma once
#include <array>
#include <cmath>
#include <juce_core/juce_core.h>

namespace maqam
{
// All pitches are in cents. Scale degrees are given relative to the tonic,
// in 24-TET (multiples of 50 cents). The "quarter-tone depth" parameter later
// re-tunes every neutral (half-flat / half-sharp) note.
struct Maqam
{
    const char* name;
    const char* arabic;
    int defaultTonicQ;              // tonic in quarter-tones above C (0..23)
    std::array<int, 7> asc;         // cents from tonic, ascending form
    std::array<int, 7> desc;        // cents from tonic, descending form
    const char* ajnas;
    const char* info;
};

// Tonic quarter-tone indices: C=0 D=4 E½♭=7 F=10 G=14 A=18 B♭=20 B½♭=21
inline const std::array<Maqam, 20>& all()
{
    static const std::array<Maqam, 20> list {{
        { "Rast", "راست", 0,
          { 0, 200, 350, 500, 700, 900, 1050 }, { 0, 200, 350, 500, 700, 900, 1000 },
          "Rast (C) + Rast (G) up / Nahawand (G) down",
          "The 'king' of maqamat: bright and dignified. Ghammaz (dominant) on the 5th. Ascends with B half-flat, usually descends with B flat." },
        { "Bayati", "بياتي", 4,
          { 0, 150, 300, 500, 700, 800, 1000 }, { 0, 150, 300, 500, 700, 800, 1000 },
          "Bayati (D) + Nahawand (G)",
          "The most loved folk and sacred maqam: warm and earthy. Ghammaz on the 4th. Lives around the neutral 2nd (E half-flat)." },
        { "Husseini", "حسيني", 4,
          { 0, 150, 300, 500, 700, 850, 1000 }, { 0, 150, 300, 500, 700, 850, 1000 },
          "Bayati (D) + Bayati (A)",
          "Bayati's sibling with a second Bayati on the 5th. Ghammaz on the 5th. Lyrical and wide." },
        { "Sikah", "سيكاه", 7,
          { 0, 150, 350, 550, 700, 850, 1050 }, { 0, 150, 350, 550, 700, 850, 1050 },
          "Sikah (E half-flat) + Rast (G)",
          "Starts on the neutral note itself: mystical, used in Quran recitation and Sufi music. Ghammaz on the 3rd (G)." },
        { "Huzam", "هزام", 7,
          { 0, 150, 350, 450, 750, 850, 1050 }, { 0, 150, 350, 450, 750, 850, 1050 },
          "Sikah (E half-flat) + Hijaz (G)",
          "Sikah with Hijaz on top: dramatic and full of longing. Ghammaz on the 3rd (G)." },
        { "Hijaz", "حجاز", 4,
          { 0, 100, 400, 500, 700, 850, 1000 }, { 0, 100, 400, 500, 700, 800, 1000 },
          "Hijaz (D) + Rast (G) up / Nahawand (G) down",
          "The augmented 2nd (E flat to F sharp) gives the iconic 'oriental' colour. Ghammaz on the 4th (G)." },
        { "Hijaz Kar", "حجاز كار", 0,
          { 0, 100, 400, 500, 700, 800, 1100 }, { 0, 100, 400, 500, 700, 800, 1100 },
          "Hijaz (C) + Hijaz (G)",
          "Two Hijaz ajnas, two augmented 2nds: intense and exotic. Also called Shad Araban." },
        { "Saba", "صبا", 4,
          { 0, 150, 300, 400, 700, 800, 1000 }, { 0, 150, 300, 400, 700, 800, 1000 },
          "Saba (D) + Hijaz (F)",
          "The saddest maqam: the flat 4th (G flat) makes it tense and mournful. Ghammaz on the 3rd (F)." },
        { "Saba Zamzam", "صبا زمزم", 4,
          { 0, 100, 300, 400, 700, 800, 1000 }, { 0, 100, 300, 400, 700, 800, 1000 },
          "Saba Zamzam (D) + Hijaz (F)",
          "Saba with a plain half-step 2nd instead of the neutral one: darker, common in Egyptian music." },
        { "Nahawand", "نهاوند", 0,
          { 0, 200, 300, 500, 700, 800, 1100 }, { 0, 200, 300, 500, 700, 800, 1000 },
          "Nahawand (C) + Hijaz (G) up / Kurd (G) down",
          "Close to Western minor: romantic. B natural going up, B flat coming down." },
        { "Kurd", "كرد", 4,
          { 0, 100, 300, 500, 700, 800, 1000 }, { 0, 100, 300, 500, 700, 800, 1000 },
          "Kurd (D) + Nahawand (G)",
          "Close to Phrygian: gentle melancholy. Very common in modern Arabic pop." },
        { "Ajam", "عجم", 20,
          { 0, 200, 400, 500, 700, 900, 1100 }, { 0, 200, 400, 500, 700, 900, 1100 },
          "Ajam (B flat) + Ajam (F)",
          "Close to Western major: joyful and festive. Traditionally on B flat." },
        { "Jiharkah", "جهاركاه", 10,
          { 0, 200, 400, 500, 700, 900, 1050 }, { 0, 200, 400, 500, 700, 900, 1050 },
          "Jiharkah (F) + Rast (C)",
          "Major-like with a neutral 7th (E half-flat over F): light and celebratory." },
        { "Nikriz", "نكريز", 0,
          { 0, 200, 300, 600, 700, 900, 1000 }, { 0, 200, 300, 600, 700, 900, 1000 },
          "Nikriz (C) + Nahawand (G)",
          "Raised 4th with an augmented 2nd (E flat to F sharp): bittersweet." },
        { "Nawa Athar", "نوى أثر", 0,
          { 0, 200, 300, 600, 700, 800, 1100 }, { 0, 200, 300, 600, 700, 800, 1100 },
          "Nawa Athar (C) + Hijaz (G)",
          "Nikriz below, Hijaz above: two augmented 2nds, dark and exotic." },
        { "Athar Kurd", "أثر كرد", 0,
          { 0, 100, 300, 600, 700, 800, 1100 }, { 0, 100, 300, 600, 700, 800, 1100 },
          "Athar Kurd (C) + Hijaz (G)",
          "Kurd colour with a raised 4th: haunting and cinematic." },
        { "Suznak", "سوزناك", 0,
          { 0, 200, 350, 500, 700, 800, 1100 }, { 0, 200, 350, 500, 700, 800, 1100 },
          "Rast (C) + Hijaz (G)",
          "Rast below, Hijaz above: brightness and longing together." },
        { "Nairuz", "نيروز", 0,
          { 0, 200, 350, 500, 700, 850, 1000 }, { 0, 200, 350, 500, 700, 850, 1000 },
          "Rast (C) + Bayati (G)",
          "Rast below, Bayati above: soft and nostalgic." },
        { "Iraq", "عراق", 21,
          { 0, 150, 350, 500, 650, 850, 1050 }, { 0, 150, 350, 500, 650, 850, 1050 },
          "Sikah (B half-flat) + Bayati (D)",
          "Sikah family, starting on B half-flat, with Bayati on D: Iraqi and Levantine flavour." },
        { "Rahat al-Arwah", "راحة الأرواح", 21,
          { 0, 150, 350, 450, 750, 850, 1050 }, { 0, 150, 350, 450, 750, 850, 1050 },
          "Sikah (B half-flat) + Hijaz (D)",
          "'Rest of the souls': Huzam's sibling, starting on B half-flat." },
    }};
    return list;
}

inline juce::StringArray maqamNames()
{
    juce::StringArray s;
    for (auto& m : all()) s.add (m.name);
    return s;
}

inline juce::StringArray tonicNames()
{
    return { juce::CharPointer_UTF8 ("C"),  juce::CharPointer_UTF8 ("C\xc2\xbd\xe2\x99\xaf"), juce::CharPointer_UTF8 ("C\xe2\x99\xaf"),
             juce::CharPointer_UTF8 ("D\xc2\xbd\xe2\x99\xad"), juce::CharPointer_UTF8 ("D"), juce::CharPointer_UTF8 ("D\xc2\xbd\xe2\x99\xaf"),
             juce::CharPointer_UTF8 ("E\xe2\x99\xad"), juce::CharPointer_UTF8 ("E\xc2\xbd\xe2\x99\xad"), juce::CharPointer_UTF8 ("E"),
             juce::CharPointer_UTF8 ("E\xc2\xbd\xe2\x99\xaf"), juce::CharPointer_UTF8 ("F"), juce::CharPointer_UTF8 ("F\xc2\xbd\xe2\x99\xaf"),
             juce::CharPointer_UTF8 ("F\xe2\x99\xaf"), juce::CharPointer_UTF8 ("G\xc2\xbd\xe2\x99\xad"), juce::CharPointer_UTF8 ("G"),
             juce::CharPointer_UTF8 ("G\xc2\xbd\xe2\x99\xaf"), juce::CharPointer_UTF8 ("A\xe2\x99\xad"), juce::CharPointer_UTF8 ("A\xc2\xbd\xe2\x99\xad"),
             juce::CharPointer_UTF8 ("A"), juce::CharPointer_UTF8 ("A\xc2\xbd\xe2\x99\xaf"), juce::CharPointer_UTF8 ("B\xe2\x99\xad"),
             juce::CharPointer_UTF8 ("B\xc2\xbd\xe2\x99\xad"), juce::CharPointer_UTF8 ("B"), juce::CharPointer_UTF8 ("B\xc2\xbd\xe2\x99\xaf") };
}

inline juce::String noteName (double nominalCents)
{
    int q = (int) std::lround (nominalCents / 50.0);
    q = ((q % 24) + 24) % 24;
    return tonicNames()[q];
}

inline juce::String keyName (int pitchClass)
{
    static const char* n[] = { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
    return n[((pitchClass % 12) + 12) % 12];
}

// Nominal (24-TET) absolute pitch of a degree, in cents above C of octave 0.
inline int nominal (int maqamIdx, int tonicQ, int degree, bool desc)
{
    auto& m = all()[(size_t) juce::jlimit (0, (int) all().size() - 1, maqamIdx)];
    int d = degree % 7, oct = degree / 7;
    return tonicQ * 50 + (desc ? m.desc[(size_t) d] : m.asc[(size_t) d]) + 1200 * oct;
}

inline bool isNeutral (int nominalCents) { return ((nominalCents % 100) + 100) % 100 != 0; }

// Neutral notes are placed "depth" cents below the semitone above them (50 = exact 24-TET).
inline double tuned (int nominalCents, double depth)
{
    return isNeutral (nominalCents) ? nominalCents + 50.0 - depth : (double) nominalCents;
}

// Which keyboard key plays a degree, Arabic-keyboard style: E key plays E half-flat, etc.
inline int keyStepsFor (int nominalCents)
{
    return isNeutral (nominalCents) ? (nominalCents + 50) / 100 : nominalCents / 100;
}

struct KeyMap
{
    std::array<double, 12> cents {};   // pitch of each key in octave 0 (key k ~ k*100)
    std::array<int, 12> degree {};     // maqam degree on this key, or -1
};

inline KeyMap buildKeyMap (int maqamIdx, int tonicQ, double depth, bool desc)
{
    KeyMap km;
    for (int k = 0; k < 12; ++k) { km.cents[(size_t) k] = k * 100.0; km.degree[(size_t) k] = -1; }

    for (int d = 0; d < 7; ++d)
    {
        int nom = nominal (maqamIdx, tonicQ, d, desc);
        int steps = keyStepsFor (nom);
        int pc = steps % 12, octShift = steps / 12;
        km.cents[(size_t) pc] = tuned (nom, depth) - 1200.0 * octShift;
        km.degree[(size_t) pc] = d;
    }
    return km;
}

// Interval label between two nominal pitches, in tones: ¼ ½ ¾ 1 1¼ 1½ ...
inline juce::String intervalLabel (int cents)
{
    int quarters = (int) std::lround (cents / 50.0);
    int whole = quarters / 4, rem = quarters % 4;
    static const char* frac[] = { "", "\xc2\xbc", "\xc2\xbd", "\xc2\xbe" };
    juce::String s = whole > 0 ? juce::String (whole) : juce::String();
    s += juce::String (juce::CharPointer_UTF8 (frac[rem]));
    return s.isEmpty() ? "0" : s;
}
} // namespace maqam
