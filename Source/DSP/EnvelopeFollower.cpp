#include "EnvelopeFollower.h"

namespace BassMutator
{
		namespace
		{
				float timeToCoefficient (float milliseconds, double sampleRate) noexcept
				{
						const auto seconds = juce::jmax (1.0e-5f, milliseconds * 0.001f);
						return (float) std::exp (-1.0 / (seconds * sampleRate));
				}
		}

		void EnvelopeFollower::prepare (const juce::dsp::ProcessSpec& spec)
		{
				sampleRate = spec.sampleRate;
				updateParameters (10.0f, 150.0f, 0.0f);
				reset();
		}

		void EnvelopeFollower::reset()
		{
				envelope = 0.0f;
		}

		void EnvelopeFollower::updateParameters (float attackMs, float releaseMs, float amountPercent)
		{
				attackCoefficient  = timeToCoefficient (attackMs, sampleRate);
				releaseCoefficient = timeToCoefficient (releaseMs, sampleRate);
				amount = juce::jlimit (-1.0f, 1.0f, amountPercent * 0.01f);
		}

		float EnvelopeFollower::process (const juce::AudioBuffer<float>& input) noexcept
		{
				const auto numChannels = input.getNumChannels();
				const auto numSamples  = input.getNumSamples();

				for (int i = 0; i < numSamples; ++i)
				{
						float peak = 0.0f;

						for (int channel = 0; channel < numChannels; ++channel)
								peak = juce::jmax (peak, std::abs (input.getSample (channel, i)));

						const auto coefficient = peak > envelope ? attackCoefficient : releaseCoefficient;
						envelope = peak + coefficient * (envelope - peak);
				}

				if (! std::isfinite (envelope))
						envelope = 0.0f;

				return juce::jlimit (0.0f, 1.0f, envelope) * amount;
		}
}
