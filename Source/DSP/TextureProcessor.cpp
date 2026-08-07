#include "TextureProcessor.h"

namespace BassMutator
{
		namespace
		{
				constexpr double smoothingSeconds = 0.02;
				constexpr float  maxDriveLinear   = 32.0f;
				constexpr float  minBitDepth      = 3.0f;
				constexpr float  maxBitDepth      = 16.0f;
				constexpr float  maxDownsample    = 24.0f;
				constexpr float  maxSideGain      = 2.0f;
		}

		void TextureProcessor::prepare (const juce::dsp::ProcessSpec& spec)
		{
				sampleRate = spec.sampleRate;

				toneFilter.prepare (spec);
				toneFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
				toneFilter.setResonance (0.707f);
				toneFilter.setCutoffFrequency (12000.0f);

				drive.reset (spec.sampleRate, smoothingSeconds);
				gain.reset (spec.sampleRate, smoothingSeconds);
				width.reset (spec.sampleRate, smoothingSeconds);
				crush.reset (spec.sampleRate, smoothingSeconds);

				reset();
		}

		void TextureProcessor::reset()
		{
				toneFilter.reset();
				downsamplePhase = 0.0;
				heldSample.fill (0.0f);
		}

		void TextureProcessor::updateParameters (float drivePercent,
																						 float crushPercent,
																						 float widthPercent,
																						 float toneHz,
																						 float gainDb,
																						 DistortionMode mode)
		{
				const auto normalisedDrive = juce::jlimit (0.0f, 1.0f, drivePercent * 0.01f);
				drive.setTargetValue (1.0f + normalisedDrive * (maxDriveLinear - 1.0f));

				crush.setTargetValue (juce::jlimit (0.0f, 1.0f, crushPercent * 0.01f));
				width.setTargetValue (juce::jlimit (0.0f, maxSideGain, widthPercent * 0.01f));
				gain.setTargetValue (juce::Decibels::decibelsToGain (gainDb));

				distortionMode = mode;

				const auto nyquistLimit = (float) (sampleRate * 0.45);
				toneFilter.setCutoffFrequency (juce::jlimit (500.0f, nyquistLimit, toneHz));
		}

		void TextureProcessor::process (juce::AudioBuffer<float>& band) noexcept
		{
				const auto numChannels = band.getNumChannels();
				const auto numSamples  = band.getNumSamples();

				for (int i = 0; i < numSamples; ++i)
				{
						const auto currentDrive = drive.getNextValue();
						const auto currentCrush = crush.getNextValue();
						const auto currentGain  = gain.getNextValue();
						const auto currentWidth = width.getNextValue();

						const auto makeUp = Distortion::gainCompensation (distortionMode, currentDrive);

						// Sample-rate reduction: hold the previous value while the phase
						// accumulator has not wrapped around.
						const auto downsampleFactor = 1.0 + (double) currentCrush * (maxDownsample - 1.0);
						downsamplePhase += 1.0;
						const bool takeNewSample = downsamplePhase >= downsampleFactor;

						if (takeNewSample)
								downsamplePhase -= downsampleFactor;

						const auto bitDepth = maxBitDepth - currentCrush * (maxBitDepth - minBitDepth);
						const auto levels = std::pow (2.0f, bitDepth) - 1.0f;

						for (int channel = 0; channel < numChannels; ++channel)
						{
								auto sample = band.getSample (channel, i) * currentDrive;
								sample = Distortion::process (distortionMode, sample) * makeUp;

								if (currentCrush > 0.0f)
								{
										const auto index = (size_t) juce::jmin (channel, (int) heldSample.size() - 1);

										if (takeNewSample || heldSample[index] == 0.0f)
										{
												const auto quantised = std::round (juce::jlimit (-1.0f, 1.0f, sample) * levels) / levels;
												heldSample[index] = sample + currentCrush * (quantised - sample);
										}

										sample = heldSample[index];
								}

								band.setSample (channel, i, sample * currentGain);
						}

						// Mid/side widening, only meaningful for stereo material.
						if (numChannels >= 2)
						{
								const auto left  = band.getSample (0, i);
								const auto right = band.getSample (1, i);

								const auto mid  = 0.5f * (left + right);
								const auto side = 0.5f * (left - right) * currentWidth;

								band.setSample (0, i, mid + side);
								band.setSample (1, i, mid - side);
						}
				}

				juce::dsp::AudioBlock<float> block (band);
				juce::dsp::ProcessContextReplacing<float> context (block);
				toneFilter.process (context);
		}
}
