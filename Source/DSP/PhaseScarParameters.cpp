#include "PhaseScarParameters.h"

namespace PhaseScar
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
								layout.add (makeFloat (ParamID::inputGain,  "Input Gain",  { -24.0f, 24.0f, 0.01f }, 0.0f, "dB", 1));
								layout.add (makeFloat (ParamID::outputGain, "Output Gain", { -24.0f, 12.0f, 0.01f }, 0.0f, "dB", 1));
								layout.add (makeFloat (ParamID::dryWet,     "Dry/Wet",     { 0.0f, 100.0f, 0.1f }, 100.0f, "%", 1));
								layout.add (makeBool  (ParamID::globalBypass, "Bypass", false));

								// ---------------------------------------------------------------- Pre EQ
								layout.add (makeBool  (ParamID::preEqEnabled, "Pre EQ Enable", true));
								layout.add (makeFloat (ParamID::preHighPass, "Pre High Pass", frequencyRange (20.0f, 1000.0f), 20.0f, "Hz", 0));
								layout.add (makeFloat (ParamID::preLowPass,  "Pre Low Pass",  frequencyRange (1000.0f, 22000.0f), 22000.0f, "Hz", 0));
								layout.add (makeFloat (ParamID::preTilt,     "Pre Tilt",      { -12.0f, 12.0f, 0.01f }, 0.0f, "dB", 1));

								// --------------------------------------------------------------- Flanger
								layout.add (makeBool  (ParamID::flangerEnabled, "Flanger Enable", true));
								layout.add (makeFloat (ParamID::flangerRate,    "Flanger Rate",   frequencyRange (0.05f, 10.0f), 0.35f, "Hz", 2));
								layout.add (makeFloat (ParamID::flangerDepth,   "Flanger Depth",  { 0.0f, 100.0f, 0.1f }, 60.0f, "%", 1));
								layout.add (makeFloat (ParamID::flangerDelay,   "Flanger Delay",  frequencyRange (0.1f, 10.0f), 1.5f, "ms", 2));
								layout.add (makeFloat (ParamID::flangerFeedback, "Flanger Feedback", { -95.0f, 95.0f, 0.1f }, 40.0f, "%", 1));
								layout.add (makeFloat (ParamID::flangerMix,     "Flanger Mix",    { 0.0f, 100.0f, 0.1f }, 50.0f, "%", 1));
								layout.add (makeFloat (ParamID::flangerStereoPhase, "Flanger Stereo Phase", { 0.0f, 180.0f, 0.1f }, 90.0f, "deg", 0));

								// ----------------------------------------------------------------- Notch
								layout.add (makeBool  (ParamID::notchEnabled, "Notch Enable", true));
								layout.add (makeFloat (ParamID::notchCenter, "Notch Center", frequencyRange (100.0f, 12000.0f), 900.0f, "Hz", 0));
								layout.add (makeFloat (ParamID::notchSpread, "Notch Spread", { 0.0f, 4.0f, 0.01f }, 1.0f, "oct", 2));
								layout.add (makeFloat (ParamID::notchQ,      "Notch Q",      { 0.2f, 20.0f, 0.01f }, 4.0f, "", 2));
								layout.add (makeFloat (ParamID::notchDepth,  "Notch Depth",  { 0.0f, 100.0f, 0.1f }, 70.0f, "%", 1));
								layout.add (makeFloat (ParamID::notchMotion, "Notch Motion", { 0.0f, 100.0f, 0.1f }, 25.0f, "%", 1));

								// ------------------------------------------------------------ Distortion
								layout.add (makeBool   (ParamID::distortionEnabled, "Distortion Enable", true));
								layout.add (makeFloat  (ParamID::distortionDrive, "Distortion Drive", { 0.0f, 36.0f, 0.01f }, 6.0f, "dB", 1));
								layout.add (makeChoice (ParamID::distortionType,  "Distortion Type", getDistortionModeNames(), 0));
								layout.add (makeFloat  (ParamID::distortionBias,  "Distortion Bias", { -1.0f, 1.0f, 0.001f }, 0.0f, "", 2));
								layout.add (makeFloat  (ParamID::distortionTone,  "Distortion Tone", frequencyRange (100.0f, 20000.0f), 8000.0f, "Hz", 0));
								layout.add (makeFloat  (ParamID::distortionTrim,  "Distortion Trim", { -24.0f, 24.0f, 0.01f }, 0.0f, "dB", 1));
								layout.add (makeFloat  (ParamID::distortionMix,   "Distortion Mix",  { 0.0f, 100.0f, 0.1f }, 100.0f, "%", 1));

								// --------------------------------------------------------------- Post EQ
								layout.add (makeBool  (ParamID::postEqEnabled, "Post EQ Enable", true));
								layout.add (makeFloat (ParamID::postLowGain,  "Post Low Gain",  { -18.0f, 18.0f, 0.01f }, 0.0f, "dB", 1));
								layout.add (makeFloat (ParamID::postMidFrequency, "Post Mid Freq", frequencyRange (100.0f, 10000.0f), 1000.0f, "Hz", 0));
								layout.add (makeFloat (ParamID::postMidGain,  "Post Mid Gain",  { -18.0f, 18.0f, 0.01f }, 0.0f, "dB", 1));
								layout.add (makeFloat (ParamID::postMidQ,     "Post Mid Q",     { 0.2f, 10.0f, 0.01f }, 0.9f, "", 2));
								layout.add (makeFloat (ParamID::postHighGain, "Post High Gain", { -18.0f, 18.0f, 0.01f }, 0.0f, "dB", 1));

								return layout;
				}
}
