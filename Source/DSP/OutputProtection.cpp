#include "OutputProtection.h"

namespace PhaseScar
{
		void OutputProtection::prepare (const juce::dsp::ProcessSpec& spec)
		{
				juce::ignoreUnused (spec);
				reset();
		}

		void OutputProtection::reset()
		{
				dcLastInput.fill (0.0f);
				dcLastOutput.fill (0.0f);
		}

		void OutputProtection::process (juce::AudioBuffer<float>& buffer) noexcept
		{
				const auto numChannels = juce::jmin (buffer.getNumChannels(), maxChannels);
				const auto numSamples  = buffer.getNumSamples();

				for (int channel = 0; channel < numChannels; ++channel)
				{
						auto* data = buffer.getWritePointer (channel);
						auto& lastIn  = dcLastInput[(size_t) channel];
						auto& lastOut = dcLastOutput[(size_t) channel];

						for (int i = 0; i < numSamples; ++i)
						{
								auto sample = data[i];

								if (! std::isfinite (sample))
										sample = 0.0f;

								const auto blocked = sample - lastIn + dcBlockerPole * lastOut;
								lastIn  = sample;
								lastOut = blocked;

								// Gentle tanh ceiling: transparent below roughly -6 dBFS.
								data[i] = juce::jlimit (-1.0f, 1.0f, std::tanh (blocked));
						}

						if (! std::isfinite (lastOut))
						{
								lastIn = 0.0f;
								lastOut = 0.0f;
						}
				}
		}
}
