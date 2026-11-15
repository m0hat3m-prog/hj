#include "PluginEditor.h"

namespace col
{
    const juce::Colour bg     { 0xff12141c };
    const juce::Colour panel  { 0xff1c2030 };
    const juce::Colour panel2 { 0xff262b40 };
    const juce::Colour gold   { 0xffd8aa4c };
    const juce::Colour teal   { 0xff3cc0ac };
    const juce::Colour text   { 0xffeae4d2 };
    const juce::Colour dim    { 0xff8a8fa6 };
}

static juce::Font font (float size, bool bold = false)
{
    return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
}

static void setParam (juce::AudioProcessorValueTreeState& s, const juce::String& id, float value)
{
    if (auto* p = s.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (value));
        p->endChangeGesture();
    }
}

//==============================================================================
MaqamLookAndFeel::MaqamLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, col::panel2);
    setColour (juce::ComboBox::outlineColourId, col::gold.withAlpha (0.4f));
    setColour (juce::ComboBox::textColourId, col::text);
    setColour (juce::ComboBox::arrowColourId, col::gold);
    setColour (juce::PopupMenu::backgroundColourId, col::panel);
    setColour (juce::PopupMenu::textColourId, col::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, col::gold.withAlpha (0.3f));
    setColour (juce::Slider::thumbColourId, col::gold);
    setColour (juce::Slider::trackColourId, col::gold.withAlpha (0.6f));
    setColour (juce::Slider::backgroundColourId, col::panel2);
    setColour (juce::Slider::textBoxTextColourId, col::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, col::dim);
    setColour (juce::ToggleButton::textColourId, col::text);
    setColour (juce::ToggleButton::tickColourId, col::gold);
    setColour (juce::TextButton::buttonColourId, col::panel2);
    setColour (juce::TextButton::textColourOffId, col::gold);
}

//==============================================================================
void DegreeView::paint (juce::Graphics& g)
{
    const int m = proc.getMaqam(), t = proc.getTonicQ();
    const double depth = proc.getDepth();
    const auto& M = maqam::all()[(size_t) m];
    const bool same = M.asc == M.desc;
    const int dirMode = proc.getDirectionMode();
    const bool curDesc = dirMode == MaqamProcessor::dirDesc
                      || (dirMode == MaqamProcessor::dirAuto && proc.isShowingDescending());
    const uint32_t act = proc.activeDegrees.load();
    const bool whiteKeys = proc.getInputMode() == MaqamProcessor::inputWhiteKeys;
    static const char* whiteNames[] = { "C", "D", "E", "F", "G", "A", "B", "C" };

    g.setColour (col::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 10.0f);

    boxes.clear();
    auto area = getLocalBounds().toFloat().reduced (12.0f, 10.0f);
    const int rows = same ? 1 : 2;
    const float rowH = area.getHeight() / rows;

    for (int r = 0; r < rows; ++r)
    {
        const bool desc = r == 1;
        auto row = area.removeFromTop (rowH).reduced (0.0f, 5.0f);
        auto lab = row.removeFromLeft (92.0f);

        const bool rowIsCurrent = rows == 1 || desc == curDesc;
        g.setColour (rowIsCurrent ? col::gold : col::dim);
        g.setFont (font (14.0f, true));
        g.drawFittedText (rows == 1 ? juce::String ("Scale") : (desc ? juce::String ("Descending") : juce::String ("Ascending")),
                          lab.toNearestInt(), juce::Justification::centredLeft, 2);

        const float gap = 28.0f;
        const float bw = (row.getWidth() - gap * 7.0f) / 8.0f;
        const double tonicTuned = maqam::tuned (maqam::nominal (m, t, 0, desc), depth);

        for (int i = 0; i < 8; ++i)
        {
            const int nom = maqam::nominal (m, t, i, desc);
            const double rel = maqam::tuned (nom, depth) - tonicTuned;
            juce::Rectangle<float> b (row.getX() + i * (bw + gap), row.getY(), bw, row.getHeight());
            const bool lit = rowIsCurrent && (act & (1u << (i % 7))) != 0;
            const bool isTonic = (i == 0 || i == 7);

            g.setColour (lit ? col::teal : (isTonic ? col::panel2.brighter (0.15f) : col::panel2));
            g.fillRoundedRectangle (b, 8.0f);
            g.setColour (isTonic ? col::gold : col::gold.withAlpha (0.25f));
            g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, isTonic ? 1.6f : 1.0f);

            auto inner = b.reduced (6.0f, 4.0f);
            g.setColour (lit ? col::bg : col::dim);
            g.setFont (font (11.0f));
            g.drawText (juce::String (i + 1), inner.removeFromTop (14.0f).toNearestInt(), juce::Justification::centredLeft);

            auto bottom = inner.removeFromBottom (28.0f);
            g.setColour (lit ? col::bg : col::text);
            g.setFont (font (bw > 70 ? 24.0f : 19.0f, true));
            g.drawFittedText (maqam::noteName (nom), inner.toNearestInt(), juce::Justification::centred, 1);

            g.setColour (lit ? col::bg : col::dim);
            g.setFont (font (11.0f));
            g.drawText (juce::String (juce::roundToInt (rel)) + juce::String (juce::CharPointer_UTF8 ("\xc2\xa2")),
                        bottom.removeFromTop (14.0f).toNearestInt(), juce::Justification::centred);
            const juce::String key = whiteKeys ? juce::String (whiteNames[i]) : maqam::keyName (maqam::keyStepsFor (nom));
            g.drawText ("key " + key, bottom.toNearestInt(), juce::Justification::centred);

            if (i < 7)
            {
                const int next = maqam::nominal (m, t, i + 1, desc);
                auto gr = juce::Rectangle<float> (b.getRight(), b.getCentreY() - 10.0f, gap, 20.0f);
                g.setColour (col::gold.withAlpha (0.85f));
                g.setFont (font (13.0f, true));
                g.drawText (maqam::intervalLabel (next - nom), gr.toNearestInt(), juce::Justification::centred);
            }

            boxes.push_back ({ b, i + (desc ? 8 : 0) });
        }
    }
}

void DegreeView::mouseDown (const juce::MouseEvent& e)
{
    for (auto& b : boxes)
        if (b.r.contains (e.position))
        {
            heldId = b.id;
            proc.audition (heldId, true);
            return;
        }
}

void DegreeView::mouseUp (const juce::MouseEvent&)
{
    if (heldId >= 0) proc.audition (heldId, false);
    heldId = -1;
}

//==============================================================================
MaqamEditor::MaqamEditor (MaqamProcessor& p)
    : AudioProcessorEditor (p), proc (p), degrees (p)
{
    setLookAndFeel (&lnf);

    auto& names = maqam::all();
    for (int i = 0; i < (int) names.size(); ++i)
        maqamBox.addItem (juce::String (names[(size_t) i].name) + "   " + juce::String::fromUTF8 (names[(size_t) i].arabic), i + 1);
    maqamBox.setSelectedItemIndex (proc.getMaqam(), juce::dontSendNotification);
    maqamBox.onChange = [this]
    {
        const int idx = maqamBox.getSelectedItemIndex();
        if (idx < 0 || idx == proc.getMaqam()) return;
        setParam (proc.apvts, "maqam", (float) idx);
        setParam (proc.apvts, "tonic", (float) maqam::all()[(size_t) idx].defaultTonicQ);  // jump to traditional tonic
        repaint();
    };

    auto addChoices = [] (juce::ComboBox& box, juce::RangedAudioParameter* param)
    {
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (param))
            box.addItemList (c->choices, 1);
    };
    addChoices (tonicBox, proc.apvts.getParameter ("tonic"));
    addChoices (inputBox, proc.apvts.getParameter ("input"));
    addChoices (dirBox,   proc.apvts.getParameter ("direction"));
    addChoices (outBox,   proc.apvts.getParameter ("output"));

    tonicA = std::make_unique<CA> (proc.apvts, "tonic", tonicBox);
    inputA = std::make_unique<CA> (proc.apvts, "input", inputBox);
    dirA   = std::make_unique<CA> (proc.apvts, "direction", dirBox);
    outA   = std::make_unique<CA> (proc.apvts, "output", outBox);

    auto setupLabel = [this] (juce::Label& l, const juce::String& s)
    {
        l.setText (s, juce::dontSendNotification);
        l.setFont (font (12.0f, true));
        addAndMakeVisible (l);
    };
    setupLabel (maqamLbl, "MAQAM");
    setupLabel (tonicLbl, "TONIC");
    setupLabel (inputLbl, "KEYBOARD");
    setupLabel (dirLbl,   "DIRECTION");
    setupLabel (outLbl,   "OUTPUT TO SYNTH");
    setupLabel (depthLbl, "QUARTER-TONE DEPTH (50 = exact)");
    setupLabel (mpeBendLbl, "MPE BEND RANGE (semitones)");
    setupLabel (mpeChLbl,   "MPE VOICES");
    setupLabel (monoBendLbl, "MONO BEND RANGE (semitones)");

    for (auto* b : { &maqamBox, &tonicBox, &inputBox, &dirBox, &outBox })
        addAndMakeVisible (*b);

    for (auto* s : { &depthSl, &mpeBendSl, &mpeChSl, &monoBendSl })
    {
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::TextBoxRight, false, 44, 20);
        addAndMakeVisible (*s);
    }
    depthA    = std::make_unique<SA> (proc.apvts, "depth", depthSl);
    mpeBendA  = std::make_unique<SA> (proc.apvts, "mpeBend", mpeBendSl);
    mpeChA    = std::make_unique<SA> (proc.apvts, "mpeChannels", mpeChSl);
    monoBendA = std::make_unique<SA> (proc.apvts, "monoBend", monoBendSl);

    setupA = std::make_unique<BA> (proc.apvts, "sendSetup", setupBtn);
    addAndMakeVisible (setupBtn);

    sclBtn.onClick = [this] { exportTuning (false); };
    tunBtn.onClick = [this] { exportTuning (true); };
    sclBtn.setTooltip ("Scala tuning for synths with built-in tuning (Vital, Pigments, Surge, Kontakt...)");
    addAndMakeVisible (sclBtn);
    addAndMakeVisible (tunBtn);

    addAndMakeVisible (degrees);

    setSize (880, 600);
    startTimerHz (30);
}

MaqamEditor::~MaqamEditor()
{
    setLookAndFeel (nullptr);
}

void MaqamEditor::timerCallback()
{
    const int m = proc.getMaqam();
    if (m != shownMaqam)
    {
        shownMaqam = m;
        maqamBox.setSelectedItemIndex (m, juce::dontSendNotification);
        repaint();
    }
    degrees.repaint();
}

void MaqamEditor::paint (juce::Graphics& g)
{
    g.fillAll (col::bg);

    // header
    g.setColour (col::gold);
    g.setFont (font (28.0f, true));
    g.drawText ("MAQAM", 18, 10, 200, 40, juce::Justification::centredLeft);
    g.setFont (font (26.0f));
    g.drawText (juce::String::fromUTF8 ("مقام"), 140, 10, 100, 40, juce::Justification::centredLeft);
    g.setColour (col::dim);
    g.setFont (font (13.0f));
    g.drawText ("Quarter-tone maqam engine for any instrument  -  click a degree to hear it",
                getWidth() - 520, 10, 500, 40, juce::Justification::centredRight);
    g.setColour (col::gold.withAlpha (0.3f));
    g.drawHorizontalLine (58, 16.0f, getWidth() - 16.0f);

    // info panel
    const auto& M = maqam::all()[(size_t) proc.getMaqam()];
    juce::Rectangle<int> info (16, 366, getWidth() - 32, 96);
    g.setColour (col::panel);
    g.fillRoundedRectangle (info.toFloat(), 10.0f);
    auto in = info.reduced (16, 10);
    auto top = in.removeFromTop (24);
    g.setColour (col::text);
    g.setFont (font (18.0f, true));
    const juce::String title = juce::String (M.name) + "  " + juce::String::fromUTF8 (M.arabic);
    g.drawText (title, top, juce::Justification::centredLeft);
    g.setColour (col::gold);
    g.setFont (font (13.0f));
    g.drawText (juce::String ("Ajnas: ") + M.ajnas, top, juce::Justification::centredRight);
    g.setColour (col::dim);
    g.setFont (font (13.5f));
    g.drawFittedText (M.info, in.withTrimmedTop (4), juce::Justification::topLeft, 3);

    // hint for the chosen output mode
    const int outMode = (int) proc.apvts.getRawParameterValue ("output")->load();
    const char* hint = outMode == MaqamProcessor::outMPE
        ? "MPE: turn on MPE in your synth (Serum, Vital, Logic synths...) and set its bend range to match. Chords stay in tune."
        : outMode == MaqamProcessor::outMono
            ? "Mono: works with ANY synth. Set the synth's pitch-bend range to match. Play one note at a time (like a ney or oud line)."
            : "No retune: notes snap to the maqam but quarter tones round to the nearest key. Use Export to load a tuning instead.";
    g.setColour (col::dim.withAlpha (0.9f));
    g.setFont (font (12.0f));
    g.drawFittedText (hint, 16, getHeight() - 26, getWidth() - 32, 20, juce::Justification::centredLeft, 1);
}

void MaqamEditor::resized()
{
    int x = 16;
    auto place = [&x] (juce::Label& l, juce::ComboBox& b, int w)
    {
        l.setBounds (x - 3, 66, w, 18);
        b.setBounds (x, 86, w, 28);
        x += w + 10;
    };
    place (maqamLbl, maqamBox, 180);
    place (tonicLbl, tonicBox, 90);
    place (inputLbl, inputBox, 210);
    place (dirLbl,   dirBox,   160);
    place (outLbl,   outBox,   getWidth() - 16 - x);

    degrees.setBounds (16, 126, getWidth() - 32, 232);

    const int colW = (getWidth() - 32 - 30) / 4;
    int sx = 16;
    for (auto [l, s] : { std::pair<juce::Label*, juce::Slider*> { &depthLbl, &depthSl }, { &mpeBendLbl, &mpeBendSl },
                         { &mpeChLbl, &mpeChSl }, { &monoBendLbl, &monoBendSl } })
    {
        l->setBounds (sx - 3, 472, colW, 18);
        s->setBounds (sx, 492, colW, 26);
        sx += colW + 10;
    }

    setupBtn.setBounds (14, 530, 320, 28);
    tunBtn.setBounds (getWidth() - 16 - 120, 530, 120, 28);
    sclBtn.setBounds (getWidth() - 16 - 250, 530, 120, 28);
}

//==============================================================================
juce::String MaqamEditor::asciiTitle() const
{
    juce::String tonic = maqam::tonicNames()[proc.getTonicQ()];
    tonic = tonic.replace (juce::CharPointer_UTF8 ("\xc2\xbd\xe2\x99\xad"), " half-flat")
                 .replace (juce::CharPointer_UTF8 ("\xc2\xbd\xe2\x99\xaf"), " half-sharp")
                 .replace (juce::CharPointer_UTF8 ("\xe2\x99\xad"), "b")
                 .replace (juce::CharPointer_UTF8 ("\xe2\x99\xaf"), "#");
    return "Maqam " + juce::String (maqam::all()[(size_t) proc.getMaqam()].name) + " on " + tonic;
}

juce::String MaqamEditor::makeScl() const
{
    const bool desc = proc.getDirectionMode() == MaqamProcessor::dirDesc;
    const auto km = maqam::buildKeyMap (proc.getMaqam(), proc.getTonicQ(), proc.getDepth(), desc);
    juce::String s;
    s << "! " << asciiTitle().replaceCharacters (" #", "_s") << ".scl\n!\n"
      << asciiTitle() << " - 12-key Arabic keyboard layout, quarter-tone depth "
      << juce::String (proc.getDepth(), 0) << " cents (key C = 1/1)\n 12\n!\n";
    for (int k = 1; k < 12; ++k)
        s << " " << juce::String (km.cents[(size_t) k] - km.cents[0], 3) << "\n";
    s << " 2/1\n";
    return s;
}

juce::String MaqamEditor::makeTun() const
{
    const bool desc = proc.getDirectionMode() == MaqamProcessor::dirDesc;
    const auto km = maqam::buildKeyMap (proc.getMaqam(), proc.getTonicQ(), proc.getDepth(), desc);
    juce::String s;
    s << "; " << asciiTitle() << " - generated by the Maqam plugin\n"
      << "; Arabic keyboard layout: in-maqam keys are retuned, other keys stay 12-TET\n\n[Tuning]\n";
    for (int n = 0; n < 128; ++n)
        s << "note " << n << "=" << juce::roundToInt (km.cents[(size_t) (n % 12)] + 1200.0 * (n / 12)) << "\n";
    s << "\n[Exact Tuning]\nBaseFreq=8.1757989156\n";
    for (int n = 0; n < 128; ++n)
        s << "note " << n << "=" << juce::String (km.cents[(size_t) (n % 12)] + 1200.0 * (n / 12), 6) << "\n";
    return s;
}

void MaqamEditor::exportTuning (bool tun)
{
    const auto name = asciiTitle().replace (" ", "_").replace ("#", "sharp") + (tun ? ".tun" : ".scl");
    chooser = std::make_unique<juce::FileChooser> ("Save tuning file",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile (name),
        tun ? "*.tun" : "*.scl");

    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
        [this, tun] (const juce::FileChooser& fc)
        {
            const auto f = fc.getResult();
            if (f != juce::File())
                f.replaceWithText (tun ? makeTun() : makeScl());
        });
}
