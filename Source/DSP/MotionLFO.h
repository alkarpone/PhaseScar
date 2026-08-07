#pragma once

#include "BassMutatorParameters.h"

namespace BassMutator
{
		/**
				Modulation source. Runs entirely inside the DSP domain - it never writes
				back into the APVTS, so the audio thread stays lock free.

				Output range is bipolar [-1, 1] scaled by depth.
		*/
		class MotionLFO
		{
		public:
				void prepare (const juce::dsp::ProcessSpec& spec);
				void reset();

				void updateParameters (float rateHz,
															 float depthPercent,
															 MotionShape shape,
															 bool sync,
															 MotionDivision division,
															 double hostBpm);

				/** Advances the LFO by the given number of samples and returns the block value. */
				float advance (int numSamples) noexcept;

				float getCurrentValue() const noexcept { return currentValue; }

		private:
				float renderShape (double phase) noexcept;

				juce::SmoothedValue<float> depth { 0.0f };

				MotionShape currentShape = MotionShape::sine;
				double phase = 0.0;
				double phaseIncrement = 0.0;
				double sampleRate = 44100.0;

				float sampleAndHoldValue = 0.0f;
				juce::Random random;

				float currentValue = 0.0f;
		};
}
