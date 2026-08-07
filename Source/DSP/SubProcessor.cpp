#include "SubProcessor.h"

namespace BassMutator
{
		namespace
		{
				constexpr float safetyHighPassHz = 25.0f;
				constexpr float dcBlockerPole    = 0.9995f;
				constexpr double smoothingSeconds = 0.02;
		}

		void SubProcessor::prepare (const juce::dsp::ProcessSpec& spec)
		{
				safetyHighPass.prepare (spec);
				safetyHighPass.setType (juce::dsp::StateVariableTPTFilterType::highpass);
				safetyHighPass.setCutoffFrequency (safetyHighPassHz);
				safetyHighPass.setResonance (0.707f);

				gain.reset (spec.sampleRate, smoothingSeconds);
				saturation.reset (spec.sampleRate, smoothingSeconds);
				monoAmount.reset (spec.sampleRate, smoothingSeconds);

				reset();
		}

		void SubProcessor::reset()
		{
				safetyHighPass.reset();
				dcLastInput.fill (0.0f);
				dcLastOutput.fill (0.0f);
		}

		void SubProcessor::updateParameters (float gainDb, float saturationPercent, bool mono)
		{
				gain.setTargetValue (juce::Decibels::decibelsToGain (gainDb));
				saturation.setTargetValue (juce::jlimit (0.0f, 1.0f, saturationPercent * 0.01f));
				monoAmount.setTargetValue (mono ? 1.0f : 0.0f);
		}

		void SubProcessor::process (juce::AudioBuffer<float>& band) noexcept
		{
				const auto numChannels = band.getNumChannels();
				const auto numSamples  = band.getNumSamples();

				juce::dsp::AudioBlock<float> block (band);
				juce::dsp::ProcessContextReplacing<float> context (block);
				safetyHighPass.process (context);

				for (int i = 0; i < numSamples; ++i)
				{
						const auto currentGain  = gain.getNextValue();
						const auto currentSat   = saturation.getNextValue();
						const auto currentMono  = monoAmount.getNextValue();

						// Mono fold first so saturation acts on the final mono content.
						if (numChannels > 1 && currentMono > 0.0f)
						{
								float sum = 0.0f;

								for (int channel = 0; channel < numChannels; ++channel)
										sum += band.getSample (channel, i);

								const auto monoSample = sum / (float) numChannels;

								for (int channel = 0; channel < numChannels; ++channel)
								{
										const auto original = band.getSample (channel, i);
										band.setSample (channel, i, original + currentMono * (monoSample - original));
								}
						}

						for (int channel = 0; channel < numChannels; ++channel)
						{
								auto sample = band.getSample (channel, i);

								if (currentSat > 0.0f)
								{
										// Very moderate drive range: the fundamental has to survive.
										const auto drive = 1.0f + currentSat * 3.0f;
										const auto shaped = Distortion::tanhShape (sample * drive) / std::sqrt (drive);
										sample += currentSat * (shaped - sample);
								}

								sample *= currentGain;

								if (channel < (int) dcLastInput.size())
								{
										const auto blocked = sample - dcLastInput[(size_t) channel]
																				 + dcBlockerPole * dcLastOutput[(size_t) channel];
										dcLastInput[(size_t) channel]  = sample;
										dcLastOutput[(size_t) channel] = blocked;
										sample = blocked;
								}

								band.setSample (channel, i, sample);
						}
				}
		}
}
