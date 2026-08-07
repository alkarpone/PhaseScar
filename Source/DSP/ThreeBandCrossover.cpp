#include "ThreeBandCrossover.h"

namespace BassMutator
{
		void ThreeBandCrossover::prepare (const juce::dsp::ProcessSpec& spec)
		{
				sampleRate = spec.sampleRate;

				subSplit.prepare (spec);
				textureSplit.prepare (spec);
				subAllpass.prepare (spec);

				subSplit.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
				textureSplit.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
				subAllpass.setType (juce::dsp::LinkwitzRileyFilterType::allpass);

				reset();
		}

		void ThreeBandCrossover::reset()
		{
				subSplit.reset();
				textureSplit.reset();
				subAllpass.reset();
		}

		void ThreeBandCrossover::updateParameters (float subCrossoverHz, float textureCrossoverHz)
		{
				const auto nyquistLimit = (float) (sampleRate * 0.45);

				const auto subHz = juce::jlimit (20.0f, nyquistLimit, subCrossoverHz);

				// The texture crossover must always stay above the sub crossover, otherwise
				// the BODY band would collapse and the split would become meaningless.
				const auto textureHz = juce::jlimit (subHz * 1.5f, nyquistLimit,
																						 juce::jmax (textureCrossoverHz, subHz * 1.5f));

				subSplit.setCutoffFrequency (subHz);
				textureSplit.setCutoffFrequency (textureHz);
				subAllpass.setCutoffFrequency (textureHz);
		}

		void ThreeBandCrossover::process (const juce::AudioBuffer<float>& input,
																			juce::AudioBuffer<float>& subBand,
																			juce::AudioBuffer<float>& bodyBand,
																			juce::AudioBuffer<float>& textureBand) noexcept
		{
				const auto numChannels = input.getNumChannels();
				const auto numSamples  = input.getNumSamples();

				for (int channel = 0; channel < numChannels; ++channel)
				{
						const auto* in = input.getReadPointer (channel);
						auto* sub     = subBand.getWritePointer (channel);
						auto* body    = bodyBand.getWritePointer (channel);
						auto* texture = textureBand.getWritePointer (channel);

						for (int i = 0; i < numSamples; ++i)
						{
								float low = 0.0f, high = 0.0f;
								subSplit.processSample (channel, in[i], low, high);

								float mid = 0.0f, top = 0.0f;
								textureSplit.processSample (channel, high, mid, top);

								sub[i]     = subAllpass.processSample (channel, low);
								body[i]    = mid;
								texture[i] = top;
						}
				}
		}
}
