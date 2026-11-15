#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "MaqamData.h"

class MaqamProcessor : public juce::AudioProcessor
{
public:
    enum InputMode  { inputArabicKeys = 0, inputSnap, inputWhiteKeys };
    enum Direction  { dirAuto = 0, dirAsc, dirDesc };
    enum OutputMode { outMPE = 0, outMono, outNone };

    MaqamProcessor();

    void prepareToPlay (double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override { return true; }
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Maqam"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // --- shared with the editor ---
    int    getMaqam() const;
    int    getTonicQ() const;
    double getDepth() const;
    int    getInputMode() const;
    int    getDirectionMode() const;
    bool   isShowingDescending() const { return lastDirDesc.load(); }

    std::atomic<uint32_t> activeDegrees { 0 };   // bit d = degree d sounding
    void audition (int id, bool on);             // UI click: id 0..7 ascending, 8..15 descending degree

    // pitch (cents above MIDI note 0) that a key plays with the current settings
    double pitchForKey (int note, bool desc, int& degreeOut) const;

private:
    struct Active { int ch = 0, note = 0, degree = -1; bool on = false; };

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    void handleNoteOn  (int inCh, int inNote, juce::uint8 vel, double pitch, int degree, int pos, juce::MidiBuffer& out);
    void handleNoteOff (int inCh, int inNote, juce::uint8 vel, int pos, juce::MidiBuffer& out);
    void allNotesOff (int pos, juce::MidiBuffer& out);
    void emitSetup (int pos, juce::MidiBuffer& out);
    int  allocateChannel();
    static int bendValue (double semis, double range);

    std::array<std::array<Active, 128>, 17> active {};   // [input channel 0..15, 16 = UI audition][note]
    std::array<int, 17> chanNotes {};
    std::array<juce::int64, 17> chanLastUsed {};
    juce::int64 useCounter = 0;

    int lastInNote = -1;
    std::atomic<bool> lastDirDesc { false };
    double monoOffsetSemis = 0.0, wheelNorm = 0.0;

    int prevOutMode = -1, prevMpeBend = -1, prevMonoBend = -1, prevMpeCh = -1;
    bool prevSendSetup = false;

    juce::SpinLock auditionLock;
    std::vector<std::pair<int, bool>> pendingAudition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaqamProcessor)
};
