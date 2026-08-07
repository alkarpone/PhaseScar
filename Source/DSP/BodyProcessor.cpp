#include "BodyProcessor.h"

namespace BassMutator
{
		namespace
		{
				constexpr double smoothingSeconds = 0.02;
				constexpr float  maxDriveLinear   = 24.0f;   // ~+27 dB at full drive
				constexpr float  maxToneOctaves   = 2.0f;    // modulation span of the tone filter
		}

		void BodyProcessor::prepare (const juce::dsp::ProcessSpec& spec)
		{
				sampleRate = spec.sampleRate;

				toneFilter.prepare (spec);
				toneFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
				toneFilter.setResonance (0.707f);
				toneFilter.setCutoffFrequency (baseToneHz);

				drive.reset (spec.sampleRate, smoothingSeconds);
				gain.reset (spec.sampleRate, smoothingSeconds);
				motionAmount.reset (spec.sampleRate, smoothingSeconds);

				reset();
		}

		void BodyProcessor::reset()
		{
				toneFilter.reset();
		}

		void BodyProcessor::updateParameters (float drivePercent,
																					float toneHz,
																					float gainDb,
																					DistortionMode mode,
																					float motionAmountPercent)
		{
				const auto normalisedDrive = juce::jlimit (0.0f, 1.0f, drivePercent * 0.01f);
				drive.setTargetValue (1.0f + normalisedDrive * (maxDriveLinear - 1.0f));

				gain.setTargetValue (juce::Decibels::decibelsToGain (gainDb));
				motionAmount.setTargetValue (juce::jlimit (0.0f, 1.0f, motionAmountPercent * 0.01f));

				distortionMode = mode;
				baseToneHz = toneHz;
		}

		void BodyProcessor::process (juce::AudioBuffer<float>& band, float modulation) noexcept
		{
				const auto numChannels = band.getNumChannels();
				const auto numSamples  = band.getNumSamples();

				// Block rate coefficient update: cheap and free of zipper artefacts at
				// typical block sizes.
				const auto amount = motionAmount.getTargetValue();
				const auto octaveShift = juce::jlimit (-1.0f, 1.0f, modulation) * amount * maxToneOctaves;
				const auto nyquistLimit = (float) (sampleRate * 0.45);
				const auto modulatedTone = juce::jlimit (40.0f, nyquistLimit,
																								 baseToneHz * std::pow (2.0f, octaveShift));
				toneFilter.setCutoffFrequency (modulatedTone);

				for (int i = 0; i < numSamples; ++i)
				{
						const auto currentDrive = drive.getNextValue();
						const auto currentGain  = gain.getNextValue();
						motionAmount.getNextValue();

						const auto makeUp = Distortion::gainCompensation (distortionMode, currentDrive);

						for (int channel = 0; channel < numChannels; ++channel)
						{
								auto sample = band.getSample (channel, i) * currentDrive;
								sample = Distortion::process (distortionMode, sample) * makeUp;
								band.setSample (channel, i, sample * currentGain);
						}
				}

				juce::dsp::AudioBlock<float> block (band);
				juce::dsp::ProcessContextReplacing<float> context (block);
				toneFilter.process (context);
		}
}
