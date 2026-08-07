#pragma once

#include <JuceHeader.h>

namespace BassMutator
{
		/**
				Peak based envelope follower with separate attack and release times.
				Used as a second modulation source alongside the motion LFO.
		*/
		class EnvelopeFollower
		{
		public:
				void prepare (const juce::dsp::ProcessSpec& spec);
				void reset();

				void updateParameters (float attackMs, float releaseMs, float amountPercent);

				/** Processes the block and returns the resulting modulation in [-1, 1]. */
				float process (const juce::AudioBuffer<float>& input) noexcept;

				float getCurrentEnvelope() const noexcept { return envelope; }

		private:
				float attackCoefficient = 0.0f;
				float releaseCoefficient = 0.0f;
				float amount = 0.0f;
				float envelope = 0.0f;
				double sampleRate = 44100.0;
		};
}
