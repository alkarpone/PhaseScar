#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PhaseScarLookAndFeel.h"
#include "SpectrumAnalyzerComponent.h"
#include "UI/ParameterTooltips.h"

//==============================================================================
/** Compact vertical level meter, fed from the UI timer (never from the audio thread). */
class PhaseScarLevelMeter : public juce::Component
{
public:
    explicit PhaseScarLevelMeter (const juce::String& captionText) : caption (captionText) {}

    void setLevel (float newLinearLevel);
    void paint (juce::Graphics&) override;

private:
    juce::String caption;
    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaseScarLevelMeter)
};

//==============================================================================
/**
        Phase Scar editor - first polished GUI pass.

        Fixed 1024 x 768 layout: header, three columns and a bottom row.
        All parameter wiring keeps using the standard APVTS attachment classes.
*/
class PhaseScarAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit PhaseScarAudioProcessorEditor (PhaseScarAudioProcessor&);
    ~PhaseScarAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    struct SliderWithLabel
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct ComboWithLabel
    {
        juce::ComboBox combo;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    struct ButtonWithLabel
    {
        juce::ToggleButton button;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    };

    void addSlider (SliderWithLabel& s, const juce::String& paramID, const juce::String& labelText);
    void addCombo (ComboWithLabel& c, const juce::String& paramID, const juce::String& labelText);
    void addButton (ButtonWithLabel& b, const juce::String& paramID, const juce::String& labelText);

    void layoutKnobs (juce::Rectangle<int> area, int columns,
                      const std::vector<SliderWithLabel*>& knobs);

    static juce::Rectangle<int> panelContent (juce::Rectangle<int> panel);

    PhaseScar::PhaseScarLookAndFeel lookAndFeelInstance;

    PhaseScarAudioProcessor& processorRef;

    // Panel rectangles, calculated in resized() and painted in paint().
    juce::Rectangle<int> headerBounds, globalBounds, preEqBounds, flangerBounds,
                         centreBounds, notchBounds, distortionBounds, postEqBounds, outputBounds;

    SliderWithLabel inputGain, outputGain, dryWet;
    ButtonWithLabel globalBypass;

    SliderWithLabel preHighPass, preLowPass, preTilt;
    ButtonWithLabel preEqEnabled;

    SliderWithLabel flangerRate, flangerDepth, flangerDelay, flangerFeedback, flangerMix, flangerStereoPhase;
    ButtonWithLabel flangerEnabled;

    SliderWithLabel notchCenter, notchSpread, notchQ, notchDepth, notchMotion;
    ButtonWithLabel notchEnabled;

    SliderWithLabel distortionDrive, distortionBias, distortionTone, distortionTrim, distortionMix;
    ComboWithLabel distortionType;
    ButtonWithLabel distortionEnabled;

    SliderWithLabel postLowGain, postMidFrequency, postMidGain, postMidQ, postHighGain;
    ButtonWithLabel postEqEnabled;

    juce::Label presetDisplay;

    // Enables the ~1s hover tooltips set on every knob/button/combo below.
    juce::TooltipWindow tooltipWindow { this, 700 };

    PhaseScarLevelMeter headerInputMeter { "IN" }, headerOutputMeter { "OUT" };
    PhaseScarLevelMeter outputSectionInputMeter { "IN" }, outputSectionOutputMeter { "OUT" };

    std::unique_ptr<PhaseScar::SpectrumAnalyzerComponent> spectrumAnalyzer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaseScarAudioProcessorEditor)
};
