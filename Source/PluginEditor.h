#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class MaqamLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MaqamLookAndFeel();
};

// Shows the scale degrees; click a degree to hear it.
class DegreeView : public juce::Component
{
public:
    explicit DegreeView (MaqamProcessor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    struct Box { juce::Rectangle<float> r; int id; };
    std::vector<Box> boxes;
    int heldId = -1;
    MaqamProcessor& proc;
};

class MaqamEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MaqamEditor (MaqamProcessor&);
    ~MaqamEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void exportTuning (bool tun);
    juce::String makeScl() const;
    juce::String makeTun() const;
    juce::String asciiTitle() const;

    MaqamProcessor& proc;
    MaqamLookAndFeel lnf;
    DegreeView degrees;

    juce::ComboBox maqamBox, tonicBox, inputBox, dirBox, outBox;
    juce::Label maqamLbl, tonicLbl, inputLbl, dirLbl, outLbl;
    juce::Slider depthSl, mpeBendSl, mpeChSl, monoBendSl;
    juce::Label depthLbl, mpeBendLbl, mpeChLbl, monoBendLbl;
    juce::ToggleButton setupBtn { "Send bend-range setup to synth" };
    juce::TextButton sclBtn { "Export .scl" }, tunBtn { "Export .tun" };

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<CA> tonicA, inputA, dirA, outA;
    std::unique_ptr<SA> depthA, mpeBendA, mpeChA, monoBendA;
    std::unique_ptr<BA> setupA;

    std::unique_ptr<juce::FileChooser> chooser;
    int shownMaqam = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaqamEditor)
};
