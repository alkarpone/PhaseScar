#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
				/**
								Basic modulated delay line flanger with a sine LFO.

								Tempo sync, through-zero operation and alternative LFO shapes are
								intentionally out of scope for the first skeleton.
				*/
				class FlangerProcessor
				{
				public:
								void prepare (const juce::dsp::ProcessSpec& spec);
								void reset();

								void updateParameters (float rateHz, float depthPercent, float baseDelayMs,
																			 float feedbackPercent, float mixPercent, float stereoPhaseDegrees) noexcept;

								void process (juce::AudioBuffer<float>& buffer) noexcept;

								/** Last LFO value of the left channel, in [-1, 1]. Used as a modulation source. */
								float getLfoValue() const noexcept { return lastLfoValue; }

				private:
								float readInterpolated (int channel, float delayInSamples) const noexcept;

								static constexpr float maxDelayMs = 25.0f;

								juce::AudioBuffer<float> delayBuffer;
								std::array<int, 2> writePosition { { 0, 0 } };

								double sampleRate = 44100.0;
								int delayBufferLength = 0;

								double lfoPhase = 0.0;
								double lfoIncrement = 0.0;
								float stereoPhaseOffset = 0.25f;
								float lastLfoValue = 0.0f;

								juce::SmoothedValue<float> baseDelaySamples, depth, feedback, mix;
				};
}
