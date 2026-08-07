#pragma once

#include "DistortionProcessor.h"

namespace BassMutator
{
		/**
				BODY band: the main character stage.

				Chain: drive -> waveshaper -> tone low-pass (movement destination) -> gain.
				The modulation value is supplied by the host processor and applied at block
				rate, so filter coefficients are not recalculated per sample.
		*/
		class BodyProcessor
		{
		public:
				void prepare (const juce::dsp::ProcessSpec& spec);
				void reset();

				void updateParameters (float drivePercent,
															 float toneHz,
															 float gainDb,
															 DistortionMode mode,
															 float motionAmountPercent);

				/** modulation is expected in [-1, 1] and moves the tone cutoff. */
				void process (juce::AudioBuffer<float>& band, float modulation) noexcept;

		private:
				juce::dsp::StateVariableTPTFilter<float> toneFilter;

				juce::SmoothedValue<float> drive { 1.0f };
				juce::SmoothedValue<float> gain { 1.0f };
				juce::SmoothedValue<float> motionAmount { 0.0f };

				DistortionMode distortionMode = DistortionMode::softClip;
				float baseToneHz = 3000.0f;
				double sampleRate = 44100.0;
		};
}
