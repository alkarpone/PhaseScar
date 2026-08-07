#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PhaseScarAssets.h"
#include "DSP/PhaseScarParameters.h"

using LnF = PhaseScar::PhaseScarLookAndFeel;

namespace
{
    constexpr int editorWidth  = 1024;
    constexpr int editorHeight = 768;
    constexpr int margin       = 12;
    constexpr int gap          = 8;

    float levelToNormalised (float linear) noexcept
    {
        if (! std::isfinite (linear) || linear <= 0.0f)
            return 0.0f;

        const auto db = juce::Decibels::gainToDecibels (linear, -60.0f);
        return juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
    }
}

//==============================================================================
void PhaseScarLevelMeter::setLevel (float newLinearLevel)
{
    const auto normalised = levelToNormalised (newLinearLevel);

    if (std::abs (normalised - level) > 0.005f)
    {
        level = normalised;
        repaint();
    }
}

void PhaseScarLevelMeter::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    juce::Rectangle<int> captionArea;

    if (caption.isNotEmpty())
        captionArea = bounds.removeFromBottom (12);

    auto bar = bounds.toFloat().reduced (0.5f);

    g.setColour (LnF::panelFillDark);
    g.fillRoundedRectangle (bar, 2.0f);

    constexpr int segments = 20;
    const auto segmentHeight = bar.getHeight() / (float) segments;
    const auto litSegments = (int) std::round (level * segments);

    for (int i = 0; i < segments; ++i)
    {
        const auto fromTop = segments - 1 - i;
        auto segment = juce::Rectangle<float> (bar.getX() + 1.5f,
                                               bar.getY() + fromTop * segmentHeight + 1.0f,
                                               bar.getWidth() - 3.0f,
                                               segmentHeight - 2.0f);

        const auto isPeakZone = i >= segments - 3;
        const auto colour = isPeakZone ? LnF::accentRed : LnF::accentPurple;

        g.setColour (i < litSegments ? colour : colour.withAlpha (0.08f));
        g.fillRect (segment);
    }

    g.setColour (LnF::outline);
    g.drawRoundedRectangle (bar, 2.0f, 1.0f);

    if (caption.isNotEmpty())
    {
        g.setColour (LnF::textDim);
        g.setFont (juce::Font (juce::FontOptions (9.0f)));
        g.drawText (caption, captionArea, juce::Justification::centred, false);
    }
}

//==============================================================================
PhaseScarAudioProcessorEditor::PhaseScarAudioProcessorEditor (PhaseScarAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&lookAndFeelInstance);

    using namespace PhaseScar;

    addSlider (inputGain,  ParamID::inputGain,  "Input");
    addSlider (outputGain, ParamID::outputGain, "Output");
    addSlider (dryWet,     ParamID::dryWet,     "Dry/Wet");
    addButton (globalBypass, ParamID::globalBypass, "");

    addButton (preEqEnabled, ParamID::preEqEnabled, "");
    addSlider (preHighPass, ParamID::preHighPass, "High Pass");
    addSlider (preLowPass,  ParamID::preLowPass,  "Low Pass");
    addSlider (preTilt,     ParamID::preTilt,     "Tilt");

    addButton (flangerEnabled, ParamID::flangerEnabled, "");
    addSlider (flangerRate,        ParamID::flangerRate,        "Rate");
    addSlider (flangerDepth,       ParamID::flangerDepth,       "Depth");
    addSlider (flangerDelay,       ParamID::flangerDelay,       "Delay");
    addSlider (flangerFeedback,    ParamID::flangerFeedback,    "Feedback");
    addSlider (flangerMix,         ParamID::flangerMix,         "Mix");
    addSlider (flangerStereoPhase, ParamID::flangerStereoPhase, "St Phase");

    addButton (notchEnabled, ParamID::notchEnabled, "");
    addSlider (notchCenter, ParamID::notchCenter, "Center");
    addSlider (notchSpread, ParamID::notchSpread, "Spread");
    addSlider (notchQ,      ParamID::notchQ,      "Q");
    addSlider (notchDepth,  ParamID::notchDepth,  "Depth");
    addSlider (notchMotion, ParamID::notchMotion, "Motion");

    addButton (distortionEnabled, ParamID::distortionEnabled, "");
    addSlider (distortionDrive, ParamID::distortionDrive, "Drive");
    addSlider (distortionBias,  ParamID::distortionBias,  "Bias");
    addSlider (distortionTone,  ParamID::distortionTone,  "Tone");
    addSlider (distortionTrim,  ParamID::distortionTrim,  "Trim");
    addSlider (distortionMix,   ParamID::distortionMix,   "Mix");
    addCombo  (distortionType,  ParamID::distortionType,  "Type");

    addButton (postEqEnabled, ParamID::postEqEnabled, "");
    addSlider (postLowGain,      ParamID::postLowGain,      "Low Gain");
    addSlider (postMidFrequency, ParamID::postMidFrequency, "Mid Freq");
    addSlider (postMidGain,      ParamID::postMidGain,      "Mid Gain");
    addSlider (postMidQ,         ParamID::postMidQ,         "Mid Q");
    addSlider (postHighGain,     ParamID::postHighGain,     "High Gain");

    // The distortion section uses the red accent. Handled purely through a component
    // property that the LookAndFeel reads - no duplicated control classes.
    for (auto* c : std::initializer_list<juce::Component*> { &distortionDrive.slider, &distortionBias.slider,
                                                             &distortionTone.slider, &distortionTrim.slider,
                                                             &distortionMix.slider, &distortionType.combo,
                                                             &distortionEnabled.button })
        LnF::setRedAccent (*c, true);

    presetDisplay.setText ("INIT", juce::dontSendNotification);
    presetDisplay.setJustificationType (juce::Justification::centred);
    presetDisplay.setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold)));
    presetDisplay.setColour (juce::Label::textColourId, LnF::textMain);
    presetDisplay.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetDisplay);

    addAndMakeVisible (headerInputMeter);
    addAndMakeVisible (headerOutputMeter);
    addAndMakeVisible (outputSectionInputMeter);
    addAndMakeVisible (outputSectionOutputMeter);

    setResizable (false, false);
    setSize (editorWidth, editorHeight);

    startTimerHz (30);
}

PhaseScarAudioProcessorEditor::~PhaseScarAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void PhaseScarAudioProcessorEditor::addSlider (SliderWithLabel& s, const juce::String& paramID, const juce::String& labelText)
{
    s.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 62, 14);
    s.slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                  juce::MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible (s.slider);

    s.label.setText (labelText.toUpperCase(), juce::dontSendNotification);
    s.label.setJustificationType (juce::Justification::centred);
    s.label.setFont (juce::Font (juce::FontOptions (10.5f)));
    s.label.setColour (juce::Label::textColourId, LnF::textDim);
    s.label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (s.label);

    s.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.apvts, paramID, s.slider);
}

void PhaseScarAudioProcessorEditor::addCombo (ComboWithLabel& c, const juce::String& paramID, const juce::String& labelText)
{
    addAndMakeVisible (c.combo);

    c.label.setText (labelText.toUpperCase(), juce::dontSendNotification);
    c.label.setJustificationType (juce::Justification::centredLeft);
    c.label.setFont (juce::Font (juce::FontOptions (10.5f)));
    c.label.setColour (juce::Label::textColourId, LnF::textDim);
    c.label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (c.label);

    if (auto* param = processorRef.apvts.getParameter (paramID))
        if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*> (param))
            c.combo.addItemList (choiceParam->choices, 1);

    c.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processorRef.apvts, paramID, c.combo);
}

void PhaseScarAudioProcessorEditor::addButton (ButtonWithLabel& b, const juce::String& paramID, const juce::String& labelText)
{
    b.button.setButtonText (labelText);
    addAndMakeVisible (b.button);

    b.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processorRef.apvts, paramID, b.button);
}

//==============================================================================
juce::Rectangle<int> PhaseScarAudioProcessorEditor::panelContent (juce::Rectangle<int> panel)
{
    auto content = panel.reduced (8);
    content.removeFromTop (22);
    return content;
}

void PhaseScarAudioProcessorEditor::layoutKnobs (juce::Rectangle<int> area, int columns,
                                                 const std::vector<SliderWithLabel*>& knobs)
{
    if (knobs.empty() || columns <= 0)
        return;

    const auto rows = ((int) knobs.size() + columns - 1) / columns;
    const auto cellWidth = area.getWidth() / columns;
    const auto cellHeight = area.getHeight() / rows;

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const auto row = (int) i / columns;
        const auto column = (int) i % columns;

        auto cell = juce::Rectangle<int> (area.getX() + column * cellWidth,
                                          area.getY() + row * cellHeight,
                                          cellWidth, cellHeight).reduced (3, 2);

        auto labelArea = cell.removeFromTop (13);
        knobs[i]->label.setBounds (labelArea);
        knobs[i]->slider.setBounds (cell);
    }
}

//==============================================================================
void PhaseScarAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (margin);

    headerBounds = bounds.removeFromTop (64);
    bounds.removeFromTop (gap);

    auto bottomRow = bounds.removeFromBottom (150);
    bounds.removeFromBottom (gap);

    // Header content
    {
        auto header = headerBounds.reduced (10, 8);
        header.removeFromLeft (390);                     // logo drawing area (see paint())

        auto meterArea = header.removeFromRight (110);
        meterArea = meterArea.reduced (4, 0);
        headerInputMeter.setBounds (meterArea.removeFromLeft (26));
        meterArea.removeFromLeft (8);
        headerOutputMeter.setBounds (meterArea.removeFromLeft (26));

        presetDisplay.setBounds (header.reduced (20, 10));
    }

    // Columns
    auto leftColumn  = bounds.removeFromLeft (296);
    bounds.removeFromLeft (10);
    auto rightColumn = bounds.removeFromRight (296);
    bounds.removeFromRight (10);
    centreBounds = bounds;

    globalBounds  = leftColumn.removeFromTop (140);
    leftColumn.removeFromTop (gap);
    preEqBounds   = leftColumn.removeFromTop (140);
    leftColumn.removeFromTop (gap);
    flangerBounds = leftColumn;

    notchBounds = rightColumn.removeFromTop (250);
    rightColumn.removeFromTop (gap);
    distortionBounds = rightColumn;

    postEqBounds = bottomRow.removeFromLeft (700);
    bottomRow.removeFromLeft (gap + 4);
    outputBounds = bottomRow;

    const auto placeEnable = [] (juce::ToggleButton& button, juce::Rectangle<int> panel)
    {
        button.setBounds (panel.getRight() - 28, panel.getY() + 5, 20, 18);
    };

    placeEnable (globalBypass.button,      globalBounds);
    placeEnable (preEqEnabled.button,      preEqBounds);
    placeEnable (flangerEnabled.button,    flangerBounds);
    placeEnable (notchEnabled.button,      notchBounds);
    placeEnable (distortionEnabled.button, distortionBounds);
    placeEnable (postEqEnabled.button,     postEqBounds);

    layoutKnobs (panelContent (globalBounds),  3, { &inputGain, &outputGain, &dryWet });
    layoutKnobs (panelContent (preEqBounds),   3, { &preHighPass, &preLowPass, &preTilt });
    layoutKnobs (panelContent (flangerBounds), 3, { &flangerRate, &flangerDepth, &flangerDelay,
                                                    &flangerFeedback, &flangerMix, &flangerStereoPhase });
    layoutKnobs (panelContent (notchBounds),   3, { &notchCenter, &notchSpread, &notchQ,
                                                    &notchDepth, &notchMotion });

    {
        auto content = panelContent (distortionBounds);
        auto typeRow = content.removeFromBottom (46).reduced (6, 4);
        distortionType.label.setBounds (typeRow.removeFromTop (14));
        distortionType.combo.setBounds (typeRow);

        layoutKnobs (content, 3, { &distortionDrive, &distortionBias, &distortionTone,
                                   &distortionTrim, &distortionMix });
    }

    layoutKnobs (panelContent (postEqBounds), 5, { &postLowGain, &postMidFrequency, &postMidGain,
                                                   &postMidQ, &postHighGain });

    {
        auto content = panelContent (outputBounds).reduced (16, 4);
        outputSectionInputMeter.setBounds (content.removeFromLeft (40).reduced (8, 0));
        content.removeFromLeft (10);
        outputSectionOutputMeter.setBounds (content.removeFromLeft (40).reduced (8, 0));
    }
}

//==============================================================================
void PhaseScarAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (LnF::background);

    // Subtle background texture, kept at low intensity so it never hurts readability.
    const auto& backgroundImage = PhaseScar::Assets::background();

    if (backgroundImage.isValid())
    {
        g.setColour (juce::Colours::white);
        g.setOpacity (0.16f);
        g.drawImage (backgroundImage, getLocalBounds().toFloat(),
                     juce::RectanglePlacement::fillDestination);
        g.setOpacity (1.0f);
    }

    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillRect (getLocalBounds());

    // Header
    LnF::drawPanel (g, headerBounds, {}, LnF::accentPurple);

    auto logoArea = headerBounds.reduced (20, 3).removeFromLeft (360);
    const auto& logoImage = PhaseScar::Assets::logo();

    if (logoImage.isValid())
    {
        // Only the detected artwork area of the PNG is drawn - the source file is
        // never modified, the empty canvas around the logo is simply skipped.
        const auto source = PhaseScar::Assets::logoContentArea();

        const auto destination = juce::RectanglePlacement (juce::RectanglePlacement::centred)
                                   .appliedTo (source.toFloat(), logoArea.toFloat())
                                   .toNearestInt();

        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.setColour (juce::Colours::white);   // drawImage() uses the current colour's alpha as opacity
        g.drawImage (logoImage,
                     destination.getX(), destination.getY(),
                     destination.getWidth(), destination.getHeight(),
                     source.getX(), source.getY(),
                     source.getWidth(), source.getHeight(),
                     false);
        g.setImageResamplingQuality (juce::Graphics::mediumResamplingQuality);
    }
    else
    {
        g.setColour (LnF::textMain);
        g.setFont (juce::Font (juce::FontOptions (22.0f, juce::Font::bold)));
        g.drawText ("PHASE SCAR", logoArea, juce::Justification::centredLeft, false);
    }

    {
        auto presetFrame = presetDisplay.getBounds().expanded (6, 4).toFloat();
        g.setColour (LnF::panelFillDark);
        g.fillRoundedRectangle (presetFrame, 3.0f);
        g.setColour (LnF::outline);
        g.drawRoundedRectangle (presetFrame, 3.0f, 1.0f);

        g.setColour (LnF::textDim);
        g.setFont (juce::Font (juce::FontOptions (9.0f)));
        g.drawText ("PRESET", presetFrame.removeFromTop (12.0f).toNearestInt(),
                    juce::Justification::centred, false);
    }

    // Section panels
    LnF::drawPanel (g, globalBounds,     "GLOBAL",     LnF::accentPurple);
    LnF::drawPanel (g, preEqBounds,      "PRE EQ",     LnF::accentPurple);
    LnF::drawPanel (g, flangerBounds,    "FLANGER",    LnF::accentPurple);
    LnF::drawPanel (g, notchBounds,      "NOTCH",      LnF::accentPurple);
    LnF::drawPanel (g, distortionBounds, "DISTORTION", LnF::accentRed);
    LnF::drawPanel (g, postEqBounds,     "POST EQ",    LnF::accentPurple);
    LnF::drawPanel (g, outputBounds,     "OUTPUT",     LnF::accentPurple);

    // Centre decorative panel (static in this phase)
    LnF::drawPanel (g, centreBounds, "CORE", LnF::accentPurple);

    auto centreContent = panelContent (centreBounds).reduced (6);
    const auto& centreImage = PhaseScar::Assets::centrePanel();

    g.setColour (juce::Colours::white);   // full opacity for the image drawing below

    if (centreImage.isValid())
    {
        g.drawImageWithin (centreImage, centreContent.getX(), centreContent.getY(),
                           centreContent.getWidth(), centreContent.getHeight() - 20,
                           juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize,
                           false);
    }
    else
    {
        const auto& symbolImage = PhaseScar::Assets::symbol();

        if (symbolImage.isValid())
        {
            g.drawImageWithin (symbolImage, centreContent.getX(), centreContent.getY(),
                               centreContent.getWidth(), centreContent.getHeight() - 20,
                               juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize,
                               false);
        }
        else
        {
            auto fallback = centreContent.withTrimmedBottom (20).toFloat().reduced (30.0f);
            g.setColour (LnF::accentPurple.withAlpha (0.35f));
            g.drawEllipse (fallback, 1.5f);
            g.drawLine (fallback.getX(), fallback.getCentreY(), fallback.getRight(), fallback.getCentreY(), 1.0f);
        }
    }

    g.setColour (LnF::textDim);
    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    g.drawText ("SIGNAL CHAIN", centreContent.removeFromBottom (16),
                juce::Justification::centred, false);
}

//==============================================================================
void PhaseScarAudioProcessorEditor::timerCallback()
{
    const auto in = processorRef.meteringData.getInputPeak();
    const auto out = processorRef.meteringData.getOutputPeak();

    headerInputMeter.setLevel (in);
    headerOutputMeter.setLevel (out);
    outputSectionInputMeter.setLevel (in);
    outputSectionOutputMeter.setLevel (out);
}
