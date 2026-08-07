#include "BassMutatorParameters.h"

namespace BassMutator
{
		namespace
		{
				constexpr int paramVersion = 1;

				juce::NormalisableRange<float> frequencyRange (float minHz, float maxHz)
				{
						// Logarithmic feel so low frequencies get sensible knob resolution.
						juce::NormalisableRange<float> range { minHz, maxHz };
						range.setSkewForCentre (std::sqrt (minHz * maxHz));
						return range;
				}

				std::unique_ptr<juce::AudioParameterFloat> makeFloat (const char* id,
																															const juce::String& name,
																															juce::NormalisableRange<float> range,
																															float defaultValue,
																															const juce::String& unit,
																															int decimals = 2)
				{
						return std::make_unique<juce::AudioParameterFloat> (
								juce::ParameterID { id, paramVersion },
								name,
								range,
								defaultValue,
								juce::AudioParameterFloatAttributes()
										.withLabel (unit)
										.withStringFromValueFunction ([unit, decimals] (float value, int)
										{
												return juce::String (value, decimals) + (unit.isEmpty() ? juce::String() : " " + unit);
										})
										.withValueFromStringFunction ([] (const juce::String& text)
										{
												return text.getFloatValue();
										}));
				}

				std::unique_ptr<juce::AudioParameterChoice> makeChoice (const char* id,
																																const juce::String& name,
																																const juce::StringArray& choices,
																																int defaultIndex)
				{
						return std::make_unique<juce::AudioParameterChoice> (
								juce::ParameterID { id, paramVersion }, name, choices, defaultIndex);
				}

				std::unique_ptr<juce::AudioParameterBool> makeBool (const char* id,
																														const juce::String& name,
																														bool defaultValue)
				{
						return std::make_unique<juce::AudioParameterBool> (
								juce::ParameterID { id, paramVersion }, name, defaultValue);
				}
		}

		juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
		{
				juce::AudioProcessorValueTreeState::ParameterLayout layout;

				// ---------------------------------------------------------------- Global
				layout.add (makeFloat (ParamID::inputGain,  "Input Gain",  { -24.0f, 24.0f, 0.01f },  0.0f, "dB", 1));
				layout.add (makeFloat (ParamID::outputGain, "Output Gain", { -24.0f, 12.0f, 0.01f },  0.0f, "dB", 1));
				layout.add (makeFloat (ParamID::dryWet,     "Dry/Wet",     {   0.0f, 100.0f, 0.1f }, 100.0f, "%",  1));
				layout.add (makeBool  (ParamID::globalBypass, "Bypass", false));

				// ------------------------------------------------------------- Crossover
				layout.add (makeFloat (ParamID::subCrossover,     "Sub Crossover",     frequencyRange (50.0f, 250.0f),   110.0f, "Hz", 1));
				layout.add (makeFloat (ParamID::textureCrossover, "Texture Crossover", frequencyRange (500.0f, 8000.0f), 2500.0f, "Hz", 0));

				// ------------------------------------------------------------------- Sub
				layout.add (makeFloat (ParamID::subGain,       "Sub Gain",       { -24.0f, 12.0f, 0.01f }, 0.0f,  "dB", 1));
				layout.add (makeFloat (ParamID::subSaturation, "Sub Saturation", {   0.0f, 100.0f, 0.1f }, 20.0f, "%",  0));
				layout.add (makeBool  (ParamID::subMono,       "Sub Mono", true));

				// ------------------------------------------------------------------ Body
				layout.add (makeFloat  (ParamID::bodyDrive,        "Body Drive",  {   0.0f, 100.0f, 0.1f }, 25.0f,   "%",  0));
				layout.add (makeFloat  (ParamID::bodyTone,         "Body Tone",   frequencyRange (200.0f, 12000.0f), 3000.0f, "Hz", 0));
				layout.add (makeFloat  (ParamID::bodyGain,         "Body Gain",   { -24.0f, 12.0f, 0.01f }, 0.0f,    "dB", 1));
				layout.add (makeChoice (ParamID::bodyDistortionMode, "Body Mode", getDistortionModeNames(), (int) DistortionMode::softClip));
				layout.add (makeFloat  (ParamID::bodyMotionAmount, "Body Motion", {   0.0f, 100.0f, 0.1f },  0.0f,   "%",  0));

				// --------------------------------------------------------------- Texture
				layout.add (makeFloat  (ParamID::textureDrive, "Texture Drive", {   0.0f, 100.0f, 0.1f },  25.0f, "%",  0));
				layout.add (makeFloat  (ParamID::textureCrush, "Texture Crush", {   0.0f, 100.0f, 0.1f },   0.0f, "%",  0));
				layout.add (makeFloat  (ParamID::textureWidth, "Texture Width", {   0.0f, 200.0f, 0.1f }, 100.0f, "%",  0));
				layout.add (makeFloat  (ParamID::textureTone,  "Texture Tone",  frequencyRange (1000.0f, 18000.0f), 12000.0f, "Hz", 0));
				layout.add (makeFloat  (ParamID::textureGain,  "Texture Gain",  { -24.0f, 12.0f, 0.01f },  0.0f,  "dB", 1));
				layout.add (makeChoice (ParamID::textureDistortionMode, "Texture Mode", getDistortionModeNames(), (int) DistortionMode::tanh));

				// ---------------------------------------------------------------- Motion
				{
						juce::NormalisableRange<float> rateRange { 0.05f, 20.0f };
						rateRange.setSkewForCentre (1.0f);
						layout.add (makeFloat (ParamID::motionRate, "Motion Rate", rateRange, 1.0f, "Hz", 2));
				}
				layout.add (makeFloat  (ParamID::motionDepth, "Motion Depth", { 0.0f, 100.0f, 0.1f }, 50.0f, "%", 0));
				layout.add (makeChoice (ParamID::motionShape, "Motion Shape", getMotionShapeNames(), (int) MotionShape::sine));
				layout.add (makeBool   (ParamID::motionSync,  "Motion Sync", false));
				layout.add (makeChoice (ParamID::motionDivision, "Motion Division", getMotionDivisionNames(), (int) MotionDivision::quarter));

				// -------------------------------------------------------------- Envelope
				layout.add (makeFloat (ParamID::envelopeAmount, "Envelope Amount", { -100.0f, 100.0f, 0.1f },   0.0f, "%",  0));
				{
						juce::NormalisableRange<float> attackRange { 0.1f, 100.0f };
						attackRange.setSkewForCentre (10.0f);
						layout.add (makeFloat (ParamID::envelopeAttack, "Envelope Attack", attackRange, 10.0f, "ms", 2));

						juce::NormalisableRange<float> releaseRange { 10.0f, 1000.0f };
						releaseRange.setSkewForCentre (150.0f);
						layout.add (makeFloat (ParamID::envelopeRelease, "Envelope Release", releaseRange, 150.0f, "ms", 1));
				}

				return layout;
		}
}
