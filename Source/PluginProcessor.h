#pragma once

#include <JuceHeader.h>

#include "DSP/PhaseScarParameters.h"
#include "DSP/PreEQProcessor.h"
#include "DSP/FlangerProcessor.h"
#include "DSP/NotchBankProcessor.h"
#include "DSP/DistortionStageProcessor.h"
#include "DSP/PostEQProcessor.h"
#include "DSP/OutputProtection.h"
#include "DSP/MeteringData.h"

//==============================================================================
class PhaseScarAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PhaseScarAudioProcessor();
    ~PhaseScarAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    PhaseScar::MeteringData meteringData;

private:
    //==============================================================================
    PhaseScar::PreEQProcessor preEq;
    PhaseScar::FlangerProcessor flanger;
    PhaseScar::NotchBankProcessor notchBank;
    PhaseScar::DistortionStageProcessor distortion;
    PhaseScar::PostEQProcessor postEq;
    PhaseScar::OutputProtection outputProtection;

    juce::AudioBuffer<float> dryBuffer;

    juce::SmoothedValue<float> inputGainSmoothed, outputGainSmoothed, dryWetSmoothed;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaseScarAudioProcessor)
};
