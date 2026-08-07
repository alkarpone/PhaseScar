#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace PhaseScar;

//==============================================================================
PhaseScarAudioProcessor::PhaseScarAudioProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

PhaseScarAudioProcessor::~PhaseScarAudioProcessor()
{
}

//==============================================================================
const juce::String PhaseScarAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhaseScarAudioProcessor::acceptsMidi() const
{
    return false;
}

bool PhaseScarAudioProcessor::producesMidi() const
{
    return false;
}

bool PhaseScarAudioProcessor::isMidiEffect() const
{
    return false;
}

double PhaseScarAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhaseScarAudioProcessor::getNumPrograms()
{
    return 1;
}

int PhaseScarAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhaseScarAudioProcessor::setCurrentProgram (int)
{
}

const juce::String PhaseScarAudioProcessor::getProgramName (int)
{
    return {};
}

void PhaseScarAudioProcessor::changeProgramName (int, const juce::String&)
{
}

//==============================================================================
void PhaseScarAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) juce::jmax (1, getTotalNumOutputChannels());

    preEq.prepare (spec);
    flanger.prepare (spec);
    notchBank.prepare (spec);
    distortion.prepare (spec);
    postEq.prepare (spec);
    outputProtection.prepare (spec);

    preEq.reset();
    flanger.reset();
    notchBank.reset();
    distortion.reset();
    postEq.reset();
    outputProtection.reset();

    dryBuffer.setSize (juce::jmax (1, getTotalNumOutputChannels()), juce::jmax (1, samplesPerBlock));

    spectrumFifo.prepare (1 << 15);
    spectrumFifo.reset();

    const auto rampSeconds = 0.05;
    inputGainSmoothed.reset (sampleRate, rampSeconds);
    outputGainSmoothed.reset (sampleRate, rampSeconds);
    dryWetSmoothed.reset (sampleRate, rampSeconds);

    inputGainSmoothed.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue (ParamID::inputGain)->load()));
    outputGainSmoothed.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue (ParamID::outputGain)->load()));
    dryWetSmoothed.setCurrentAndTargetValue (apvts.getRawParameterValue (ParamID::dryWet)->load() / 100.0f);

    meteringData.reset();
}

void PhaseScarAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhaseScarAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

//==============================================================================
void PhaseScarAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused (midiMessages);

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = getTotalNumOutputChannels();

    for (auto i = getTotalNumInputChannels(); i < numChannels; ++i)
        buffer.clear (i, 0, numSamples);

    // -------------------------------------------------------------- Global in
    const auto inputGainDb  = apvts.getRawParameterValue (ParamID::inputGain)->load();
    const auto outputGainDb = apvts.getRawParameterValue (ParamID::outputGain)->load();
    const auto dryWetPct    = apvts.getRawParameterValue (ParamID::dryWet)->load();
    const auto bypassed     = apvts.getRawParameterValue (ParamID::globalBypass)->load() > 0.5f;

    inputGainSmoothed.setTargetValue (juce::Decibels::decibelsToGain (inputGainDb));
    outputGainSmoothed.setTargetValue (juce::Decibels::decibelsToGain (outputGainDb));
    dryWetSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, dryWetPct / 100.0f));

    if (numSamples <= 0 || numChannels <= 0)
        return;

    for (int ch = 0; ch < numChannels; ++ch)
        buffer.applyGainRamp (ch, 0, numSamples,
                              inputGainSmoothed.getCurrentValue(),
                              inputGainSmoothed.skip (numSamples));

    float inputPeak = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        inputPeak = juce::jmax (inputPeak, buffer.getMagnitude (ch, 0, numSamples));
    meteringData.pushInputPeak (inputPeak);

    if (bypassed)
    {
        meteringData.pushOutputPeak (inputPeak);
        return;
    }

    dryBuffer.setSize (numChannels, numSamples, false, false, true);
    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);

    // ---------------------------------------------------------------- Pre EQ
    if (apvts.getRawParameterValue (ParamID::preEqEnabled)->load() > 0.5f)
    {
        preEq.updateParameters (apvts.getRawParameterValue (ParamID::preHighPass)->load(),
                                apvts.getRawParameterValue (ParamID::preLowPass)->load(),
                                apvts.getRawParameterValue (ParamID::preTilt)->load());
        preEq.process (buffer);
    }

    // --------------------------------------------------------------- Flanger
    float modulation = 0.0f;

    if (apvts.getRawParameterValue (ParamID::flangerEnabled)->load() > 0.5f)
    {
        flanger.updateParameters (apvts.getRawParameterValue (ParamID::flangerRate)->load(),
                                  apvts.getRawParameterValue (ParamID::flangerDepth)->load(),
                                  apvts.getRawParameterValue (ParamID::flangerDelay)->load(),
                                  apvts.getRawParameterValue (ParamID::flangerFeedback)->load(),
                                  apvts.getRawParameterValue (ParamID::flangerMix)->load(),
                                  apvts.getRawParameterValue (ParamID::flangerStereoPhase)->load());
        flanger.process (buffer);
        modulation = flanger.getLfoValue();
    }

    meteringData.setMotionValue (modulation);

    // ------------------------------------------------------------ Notch bank
    if (apvts.getRawParameterValue (ParamID::notchEnabled)->load() > 0.5f)
    {
        notchBank.updateParameters (apvts.getRawParameterValue (ParamID::notchCenter)->load(),
                                    apvts.getRawParameterValue (ParamID::notchSpread)->load(),
                                    apvts.getRawParameterValue (ParamID::notchQ)->load(),
                                    apvts.getRawParameterValue (ParamID::notchDepth)->load(),
                                    apvts.getRawParameterValue (ParamID::notchMotion)->load(),
                                    modulation);
        notchBank.process (buffer);
    }

    // ------------------------------------------------------------ Distortion
    if (apvts.getRawParameterValue (ParamID::distortionEnabled)->load() > 0.5f)
    {
        const auto mode = (DistortionMode) (int) apvts.getRawParameterValue (ParamID::distortionType)->load();

        distortion.updateParameters (apvts.getRawParameterValue (ParamID::distortionDrive)->load(),
                                     mode,
                                     apvts.getRawParameterValue (ParamID::distortionBias)->load(),
                                     apvts.getRawParameterValue (ParamID::distortionTone)->load(),
                                     apvts.getRawParameterValue (ParamID::distortionTrim)->load(),
                                     apvts.getRawParameterValue (ParamID::distortionMix)->load());
        distortion.process (buffer);
    }

    // --------------------------------------------------------------- Post EQ
    if (apvts.getRawParameterValue (ParamID::postEqEnabled)->load() > 0.5f)
    {
        postEq.updateParameters (apvts.getRawParameterValue (ParamID::postLowGain)->load(),
                                 apvts.getRawParameterValue (ParamID::postMidFrequency)->load(),
                                 apvts.getRawParameterValue (ParamID::postMidGain)->load(),
                                 apvts.getRawParameterValue (ParamID::postMidQ)->load(),
                                 apvts.getRawParameterValue (ParamID::postHighGain)->load());
        postEq.process (buffer);
    }

    // ---------------------------------------------------------------- DryWet
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* wet = buffer.getWritePointer (ch);
        const auto* dry = dryBuffer.getReadPointer (ch);

        auto wetAmount = dryWetSmoothed;

        for (int n = 0; n < numSamples; ++n)
        {
            const auto amount = wetAmount.getNextValue();
            wet[n] = dry[n] + (wet[n] - dry[n]) * amount;
        }
    }

    dryWetSmoothed.skip (numSamples);

    // ----------------------------------------------------------- Output gain
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto gain = outputGainSmoothed;
        buffer.applyGainRamp (ch, 0, numSamples, gain.getCurrentValue(), gain.skip (numSamples));
    }

    outputGainSmoothed.skip (numSamples);

    // ------------------------------------------------------------ Protection
    outputProtection.process (buffer);

    float outputPeak = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        outputPeak = juce::jmax (outputPeak, buffer.getMagnitude (ch, 0, numSamples));
    meteringData.pushOutputPeak (outputPeak);

    // Real-time-safe push of the post-processing mono mixdown for the GUI spectrum analyzer.
    // No allocations, no locks, no repaint calls happen inside push().
    spectrumFifo.push (buffer);
}

//==============================================================================
juce::AudioProcessorEditor* PhaseScarAudioProcessor::createEditor()
{
    return new PhaseScarAudioProcessorEditor (*this);
}

bool PhaseScarAudioProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
void PhaseScarAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
    }
}

void PhaseScarAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhaseScarAudioProcessor();
}
