#pragma once

#include "DistortionProcessor.h"

namespace BassMutator
{
		/**
				TEXTURE band: aggressive top end mangling.

				Chain: drive -> waveshaper -> bit/sample-rate crush -> tone low-pass
				-> mid/side width -> gain.
				The band only contains content above the texture crossover, so widening
				here can never destabilise the low end.
		*/
		class TextureProcessor
		{
		public:
				void prepare (const juce::dsp::ProcessSpec& spec);
				void reset();

				void updateParameters (float drivePercent,
															 float crushPercent,
															 float widthPercent,
															 float toneHz,
															 float gainDb,
															 DistortionMode mode);

				void process (juce::AudioBuffer<float>& band) noexcept;

		private:
				juce::dsp::StateVariableTPTFilter<float> toneFilter;

				juce::SmoothedValue<float> drive { 1.0f };
				juce::SmoothedValue<float> gain { 1.0f };
				juce::SmoothedValue<float> width { 1.0f };
				juce::SmoothedValue<float> crush { 0.0f };

				DistortionMode distortionMode = DistortionMode::tanh;

				// Sample-rate reduction state.
				double downsamplePhase = 0.0;
				std::array<float, 2> heldSample { {} };

				double sampleRate = 44100.0;
		};
}
