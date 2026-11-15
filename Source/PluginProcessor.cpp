#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

APVTS::ParameterLayout MaqamProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "maqam", 1 }, "Maqam", maqam::maqamNames(), 0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "tonic", 1 }, "Tonic", maqam::tonicNames(), 0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "input", 1 }, "Keyboard",
        juce::StringArray { "Arabic keys (E = E half-flat)", "Snap every key to maqam", "White keys = degrees" }, 0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "direction", 1 }, "Direction",
        juce::StringArray { "Auto (follows melody)", "Always ascending", "Always descending" }, 0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "output", 1 }, "Output",
        juce::StringArray { "MPE (polyphonic)", "Mono pitch-bend (any synth)", "No retune (12-TET)" }, 0));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "depth", 1 }, "Quarter-tone depth",
        juce::NormalisableRange<float> (20.0f, 80.0f, 1.0f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel ("cents")));
    p.push_back (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "mpeBend", 1 }, "MPE bend range", 1, 96, 48));
    p.push_back (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "monoBend", 1 }, "Mono bend range", 1, 24, 2));
    p.push_back (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "mpeChannels", 1 }, "MPE voices", 1, 15, 15));
    p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "sendSetup", 1 }, "Send bend-range setup", true));

    return { p.begin(), p.end() };
}

MaqamProcessor::MaqamProcessor()
    : AudioProcessor (BusesProperties()),
      apvts (*this, nullptr, "MaqamState", createLayout())
{
    pendingAudition.reserve (64);
}

int    MaqamProcessor::getMaqam() const         { return (int) apvts.getRawParameterValue ("maqam")->load(); }
int    MaqamProcessor::getTonicQ() const        { return (int) apvts.getRawParameterValue ("tonic")->load(); }
double MaqamProcessor::getDepth() const         { return (double) apvts.getRawParameterValue ("depth")->load(); }
int    MaqamProcessor::getInputMode() const     { return (int) apvts.getRawParameterValue ("input")->load(); }
int    MaqamProcessor::getDirectionMode() const { return (int) apvts.getRawParameterValue ("direction")->load(); }

void MaqamProcessor::prepareToPlay (double, int)
{
    prevOutMode = -1;   // re-send setup after (re)start
}

double MaqamProcessor::pitchForKey (int note, bool desc, int& degreeOut) const
{
    const int m = getMaqam(), t = getTonicQ();
    const double depth = getDepth();
    const int pc = note % 12, oct = note / 12;
    degreeOut = -1;

    if (getInputMode() == inputWhiteKeys)
    {
        static const int wk[12] = { 0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6 };
        const int d = wk[pc];
        if (d < 0) return -1.0;
        int nom = maqam::nominal (m, t, d, desc);
        if (t * 50 >= 600) nom -= 1200;           // keep tonic close to the C key
        degreeOut = d;
        return maqam::tuned (nom, depth) + 1200.0 * oct;
    }

    const auto km = maqam::buildKeyMap (m, t, depth, desc);

    if (km.degree[(size_t) pc] >= 0)
    {
        degreeOut = km.degree[(size_t) pc];
        return km.cents[(size_t) pc] + 1200.0 * oct;
    }

    if (getInputMode() == inputArabicKeys)
        return note * 100.0;

    // Snap: nearest maqam degree (ties go down)
    const double target = note * 100.0;
    double best = target, bestDist = 1.0e9;
    for (int o = oct - 1; o <= oct + 1; ++o)
        for (int k = 0; k < 12; ++k)
        {
            if (km.degree[(size_t) k] < 0) continue;
            const double cand = km.cents[(size_t) k] + 1200.0 * o;
            const double dist = std::abs (cand - target);
            if (dist < bestDist - 1.0e-6 || (std::abs (dist - bestDist) < 1.0e-6 && cand < best))
            {
                bestDist = dist; best = cand; degreeOut = km.degree[(size_t) k];
            }
        }
    return best;
}

int MaqamProcessor::bendValue (double semis, double range)
{
    const int v = 8192 + (int) std::lround (semis / range * 8192.0);
    return juce::jlimit (0, 16383, v);
}

int MaqamProcessor::allocateChannel()
{
    const int n = (int) apvts.getRawParameterValue ("mpeChannels")->load();
    int best = 2;
    for (int ch = 2; ch <= 1 + n; ++ch)
    {
        if (chanNotes[(size_t) ch] < chanNotes[(size_t) best]
            || (chanNotes[(size_t) ch] == chanNotes[(size_t) best] && chanLastUsed[(size_t) ch] < chanLastUsed[(size_t) best]))
            best = ch;
    }
    ++chanNotes[(size_t) best];
    chanLastUsed[(size_t) best] = ++useCounter;
    return best;
}

void MaqamProcessor::handleNoteOn (int inCh, int inNote, juce::uint8 vel, double pitch, int degree, int pos, juce::MidiBuffer& out)
{
    if (active[(size_t) inCh][(size_t) inNote].on)
        handleNoteOff (inCh, inNote, 0, pos, out);

    const int outMode = (int) apvts.getRawParameterValue ("output")->load();
    const int outNote = juce::jlimit (0, 127, (int) std::lround (pitch / 100.0));
    const double bend = (pitch - outNote * 100.0) / 100.0;   // semitones, about ±0.5
    int ch = 1;

    if (outMode == outMPE)
    {
        ch = allocateChannel();
        const double range = apvts.getRawParameterValue ("mpeBend")->load();
        out.addEvent (juce::MidiMessage::pitchWheel (ch, bendValue (bend, range)), pos);
    }
    else if (outMode == outMono)
    {
        monoOffsetSemis = bend;
        const double range = apvts.getRawParameterValue ("monoBend")->load();
        out.addEvent (juce::MidiMessage::pitchWheel (1, bendValue (monoOffsetSemis + wheelNorm * 2.0, range)), pos);
    }
    else
    {
        ch = juce::jlimit (1, 16, inCh + 1);
    }

    out.addEvent (juce::MidiMessage::noteOn (ch, outNote, vel), pos);
    active[(size_t) inCh][(size_t) inNote] = { ch, outNote, degree, true };
}

void MaqamProcessor::handleNoteOff (int inCh, int inNote, juce::uint8 vel, int pos, juce::MidiBuffer& out)
{
    auto& a = active[(size_t) inCh][(size_t) inNote];
    if (! a.on) return;
    out.addEvent (juce::MidiMessage::noteOff (a.ch, a.note, vel), pos);
    if (chanNotes[(size_t) a.ch] > 0) --chanNotes[(size_t) a.ch];
    a.on = false;
}

void MaqamProcessor::allNotesOff (int pos, juce::MidiBuffer& out)
{
    for (int c = 0; c < 17; ++c)
        for (int n = 0; n < 128; ++n)
            handleNoteOff (c, n, 0, pos, out);
    chanNotes.fill (0);
}

void MaqamProcessor::emitSetup (int pos, juce::MidiBuffer& out)
{
    const int outMode = (int) apvts.getRawParameterValue ("output")->load();
    if (outMode == outMPE)
    {
        const int n = (int) apvts.getRawParameterValue ("mpeChannels")->load();
        const int range = (int) apvts.getRawParameterValue ("mpeBend")->load();
        out.addEvents (juce::MPEMessages::setLowerZone (n, range, 2), 0, -1, pos);
    }
    else if (outMode == outMono)
    {
        const int range = (int) apvts.getRawParameterValue ("monoBend")->load();
        out.addEvents (juce::MidiRPNGenerator::generate (1, 0, range << 7, false, true), 0, -1, pos);
    }
}

void MaqamProcessor::audition (int id, bool on)
{
    const juce::SpinLock::ScopedLockType lock (auditionLock);
    pendingAudition.emplace_back (id, on);
}

void MaqamProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    buffer.clear();

    const int outMode  = (int) apvts.getRawParameterValue ("output")->load();
    const int mpeBend  = (int) apvts.getRawParameterValue ("mpeBend")->load();
    const int monoBend = (int) apvts.getRawParameterValue ("monoBend")->load();
    const int mpeCh    = (int) apvts.getRawParameterValue ("mpeChannels")->load();
    const bool sendSetup = apvts.getRawParameterValue ("sendSetup")->load() > 0.5f;
    const int dirMode  = getDirectionMode();

    juce::MidiBuffer out;

    if (outMode != prevOutMode || mpeCh != prevMpeCh)
        allNotesOff (0, out);

    if (sendSetup && (outMode != prevOutMode || mpeBend != prevMpeBend || monoBend != prevMonoBend
                      || mpeCh != prevMpeCh || ! prevSendSetup))
        emitSetup (0, out);

    prevOutMode = outMode; prevMpeBend = mpeBend; prevMonoBend = monoBend;
    prevMpeCh = mpeCh; prevSendSetup = sendSetup;

    // --- notes clicked in the editor ---
    std::pair<int, bool> clicks[32];
    int numClicks = 0;
    {
        const juce::SpinLock::ScopedTryLockType lock (auditionLock);
        if (lock.isLocked())
        {
            for (auto& e : pendingAudition)
                if (numClicks < 32) clicks[numClicks++] = e;
            pendingAudition.clear();
        }
    }
    for (int i = 0; i < numClicks; ++i)
    {
        // id 0..7 = ascending degree, 8..15 = descending degree
        const int id = juce::jlimit (0, 15, clicks[i].first);
        const int deg = id % 8;
        const bool desc = id >= 8;
        if (clicks[i].second)
        {
            const int t = getTonicQ();
            int nom = maqam::nominal (getMaqam(), t, deg, desc);
            if (t * 50 >= 600) nom -= 1200;
            handleNoteOn (16, id, 100, maqam::tuned (nom, getDepth()) + 6000.0, deg % 7, 0, out);
        }
        else
        {
            handleNoteOff (16, id, 0, 0, out);
        }
    }

    // --- incoming MIDI ---
    for (const auto meta : midi)
    {
        auto msg = meta.getMessage();
        const int pos = meta.samplePosition;
        const int inCh = juce::jlimit (0, 15, msg.getChannel() - 1);

        if (msg.isNoteOn())
        {
            const int n = msg.getNoteNumber();
            bool desc = dirMode == dirDesc;
            if (dirMode == dirAuto)
            {
                desc = lastDirDesc.load();
                if (lastInNote >= 0 && n < lastInNote) desc = true;
                else if (lastInNote >= 0 && n > lastInNote) desc = false;
            }
            lastInNote = n;
            lastDirDesc = desc;

            int degree = -1;
            const double pitch = pitchForKey (n, desc, degree);
            if (pitch < 0.0) continue;    // black key in "white keys" mode
            handleNoteOn (inCh, n, msg.getVelocity(), pitch, degree, pos, out);
        }
        else if (msg.isNoteOff())
        {
            handleNoteOff (inCh, msg.getNoteNumber(), msg.getVelocity(), pos, out);
        }
        else if (msg.isPitchWheel())
        {
            if (outMode == outMono)
            {
                wheelNorm = (msg.getPitchWheelValue() - 8192) / 8192.0;
                out.addEvent (juce::MidiMessage::pitchWheel (1, bendValue (monoOffsetSemis + wheelNorm * 2.0, monoBend)), pos);
            }
            else if (outMode == outMPE)
            {
                out.addEvent (juce::MidiMessage::pitchWheel (1, msg.getPitchWheelValue()), pos);  // master channel
            }
            else out.addEvent (msg, pos);
        }
        else if (msg.isAftertouch())
        {
            const auto& a = active[(size_t) inCh][(size_t) msg.getNoteNumber()];
            if (a.on)
            {
                if (outMode == outMPE)
                    out.addEvent (juce::MidiMessage::channelPressureChange (a.ch, msg.getAfterTouchValue()), pos);
                else
                    out.addEvent (juce::MidiMessage::aftertouchChange (a.ch, a.note, msg.getAfterTouchValue()), pos);
            }
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            allNotesOff (pos, out);
            if (outMode != outNone) msg.setChannel (1);
            out.addEvent (msg, pos);
        }
        else
        {
            if (outMode != outNone && msg.getChannel() > 0) msg.setChannel (1);
            out.addEvent (msg, pos);
        }
    }

    midi.swapWith (out);

    uint32_t mask = 0;
    for (auto& row : active)
        for (auto& a : row)
            if (a.on && a.degree >= 0) mask |= (1u << a.degree);
    activeDegrees = mask;
}

void MaqamProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void MaqamProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* MaqamProcessor::createEditor() { return new MaqamEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MaqamProcessor(); }
