#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
				/**
								Two moving notch filters placed symmetrically around a centre frequency.
								Coefficients are refreshed once per processing block, not per sample.
				*/
				class NotchBankProcessor
				{
				public:
								void prepare (const juce::dsp::ProcessSpec& spec);
								void reset();

								void updateParameters (float centreHz, float spreadOctaves, float q,
																			 float depthPercent, float motionPercent, float modulation) noexcept;

								void process (juce::AudioBuffer<float>& buffer) noexcept;

				private:
								using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
																															juce::dsp::IIR::Coefficients<float>>;

								/** Number of identical notch stages in series - more stages, deeper cut. */
								static constexpr int numStages = 3;

								std::array<Filter, numStages> notchLow, notchHigh;
								juce::AudioBuffer<float> workBuffer;

								double sampleRate = 44100.0;
								float depth = 0.0f;
								float lastLowHz = -1.0f;
								float lastHighHz = -1.0f;
								float lastQ = -1.0f;
				};
}
