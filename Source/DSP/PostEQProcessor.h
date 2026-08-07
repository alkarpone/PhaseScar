#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
				/** Three band tone shaping placed after the distortion stage. */
				class PostEQProcessor
				{
				public:
								void prepare (const juce::dsp::ProcessSpec& spec);
								void reset();

								void updateParameters (float lowGainDb, float midFrequencyHz, float midGainDb,
																			 float midQ, float highGainDb) noexcept;
								void process (juce::AudioBuffer<float>& buffer) noexcept;

				private:
								using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
																															juce::dsp::IIR::Coefficients<float>>;

								static constexpr float lowShelfHz = 150.0f;
								static constexpr float highShelfHz = 6000.0f;

								Filter lowShelf, midPeak, highShelf;

								double sampleRate = 44100.0;
								float lastLowGainDb = -1000.0f;
								float lastMidFrequencyHz = -1.0f;
								float lastMidGainDb = -1000.0f;
								float lastMidQ = -1.0f;
								float lastHighGainDb = -1000.0f;
				};
}
