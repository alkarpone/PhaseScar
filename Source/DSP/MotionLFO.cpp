#include "MotionLFO.h"

namespace BassMutator
{
		void MotionLFO::prepare (const juce::dsp::ProcessSpec& spec)
		{
				sampleRate = spec.sampleRate;
				depth.reset (spec.sampleRate, 0.05);
				reset();
		}

		void MotionLFO::reset()
		{
				phase = 0.0;
				sampleAndHoldValue = 0.0f;
				currentValue = 0.0f;
		}

		void MotionLFO::updateParameters (float rateHz,
																			float depthPercent,
																			MotionShape shape,
																			bool sync,
																			MotionDivision division,
																			double hostBpm)
		{
				depth.setTargetValue (juce::jlimit (0.0f, 1.0f, depthPercent * 0.01f));
				currentShape = shape;

				double frequencyHz = juce::jlimit (0.01, 100.0, (double) rateHz);

				if (sync && hostBpm > 0.0)
				{
						const auto beats = getDivisionInBeats (division);
						const auto secondsPerBeat = 60.0 / hostBpm;
						const auto cycleSeconds = juce::jmax (1.0e-4, beats * secondsPerBeat);
						frequencyHz = 1.0 / cycleSeconds;
				}

				phaseIncrement = frequencyHz / sampleRate;
		}

		float MotionLFO::renderShape (double p) noexcept
		{
				switch (currentShape)
				{
						case MotionShape::sine:
								return (float) std::sin (p * juce::MathConstants<double>::twoPi);

						case MotionShape::triangle:
								return (float) (4.0 * std::abs (p - 0.5) - 1.0);

						case MotionShape::saw:
								return (float) (2.0 * p - 1.0);

						case MotionShape::square:
								return p < 0.5 ? 1.0f : -1.0f;

						case MotionShape::sampleAndHold:
								return sampleAndHoldValue;

						case MotionShape::numShapes:
						default:
								return 0.0f;
				}
		}

		float MotionLFO::advance (int numSamples) noexcept
		{
				const auto previousPhase = phase;
				phase += phaseIncrement * (double) numSamples;

				if (phase >= 1.0)
				{
						phase -= std::floor (phase);

						// New random value on each cycle wrap for sample & hold.
						sampleAndHoldValue = random.nextFloat() * 2.0f - 1.0f;
				}

				juce::ignoreUnused (previousPhase);

				const auto depthValue = depth.skip (numSamples);
				currentValue = renderShape (phase) * depthValue;
				return currentValue;
		}
}
