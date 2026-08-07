#pragma once

#include <JuceHeader.h>

namespace BassMutator
{
		/**
				Phase-consistent three band split (SUB / BODY / TEXTURE).

				Implemented as two cascaded Linkwitz-Riley 4th order crossovers.
				Because the second crossover only sees the upper part of the spectrum,
				the SUB band is passed through an all-pass tuned to the texture crossover
				frequency. That keeps the phase relationship intact, so summing the three
				bands reconstructs the input without deep cancellation notches.
		*/
		class ThreeBandCrossover
		{
		public:
				void prepare (const juce::dsp::ProcessSpec& spec);
				void reset();

				/** Coefficient updates are block rate, never per sample. */
				void updateParameters (float subCrossoverHz, float textureCrossoverHz);

				/** Splits input into three pre-allocated band buffers of matching size. */
				void process (const juce::AudioBuffer<float>& input,
											juce::AudioBuffer<float>& subBand,
											juce::AudioBuffer<float>& bodyBand,
											juce::AudioBuffer<float>& textureBand) noexcept;

		private:
				juce::dsp::LinkwitzRileyFilter<float> subSplit;
				juce::dsp::LinkwitzRileyFilter<float> textureSplit;
				juce::dsp::LinkwitzRileyFilter<float> subAllpass;

				double sampleRate = 44100.0;
		};
}
