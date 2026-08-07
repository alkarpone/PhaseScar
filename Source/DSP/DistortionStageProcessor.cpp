#include "DistortionStageProcessor.h"
#include "Waveshapers.h"

namespace PhaseScar
{
				void DistortionStageProcessor::prepare (const juce::dsp::ProcessSpec& spec)
				{
								sampleRate = spec.sampleRate;

								toneFilter.prepare (spec);
								workBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize, false, true, false);

								const auto rampSeconds = 0.05;
								drive.reset (sampleRate, rampSeconds);
								bias.reset (sampleRate, rampSeconds);
								trim.reset (sampleRate, rampSeconds);
								mix.reset (sampleRate, rampSeconds);

								lastToneHz = -1.0f;

								reset();
				}

				void DistortionStageProcessor::reset()
				{
								toneFilter.reset();
								workBuffer.clear();
				}

				void DistortionStageProcessor::updateParameters (float driveDb, DistortionMode mode, float biasValue,
																												 float toneHz, float trimDb, float mixPercent) noexcept
				{
								currentMode = mode;

								drive.setTargetValue (juce::Decibels::decibelsToGain (juce::jlimit (0.0f, 36.0f, driveDb)));
								bias.setTargetValue (juce::jlimit (-1.0f, 1.0f, biasValue));
								trim.setTargetValue (juce::Decibels::decibelsToGain (juce::jlimit (-24.0f, 24.0f, trimDb)));
								mix.setTargetValue (juce::jlimit (0.0f, 1.0f, mixPercent / 100.0f));

								const auto nyquist = (float) (sampleRate * 0.5);
								toneHz = juce::jlimit (100.0f, juce::jmin (20000.0f, nyquist * 0.95f), toneHz);

								if (! juce::approximatelyEqual (toneHz, lastToneHz))
								{
												lastToneHz = toneHz;
												*toneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, toneHz);
								}
				}

				void DistortionStageProcessor::process (juce::AudioBuffer<float>& buffer) noexcept
				{
								const auto numSamples = buffer.getNumSamples();
								const auto numChannels = buffer.getNumChannels();

								if (numSamples <= 0 || numChannels <= 0)
												return;

								if (workBuffer.getNumChannels() < numChannels || workBuffer.getNumSamples() < numSamples)
												return;

								for (int ch = 0; ch < numChannels; ++ch)
												workBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);

								for (int n = 0; n < numSamples; ++n)
								{
												const auto currentDrive = drive.getNextValue();
												const auto currentBias = bias.getNextValue();
												const auto compensation = Waveshapers::gainCompensation (currentMode, currentDrive);

												for (int ch = 0; ch < numChannels; ++ch)
												{
																auto* data = workBuffer.getWritePointer (ch);
																auto shaped = Waveshapers::process (currentMode, data[n] * currentDrive + currentBias);
																shaped = (shaped - currentBias * 0.5f) * compensation;

																if (! std::isfinite (shaped))
																				shaped = 0.0f;

																data[n] = juce::jlimit (-4.0f, 4.0f, shaped);
												}
								}

								juce::dsp::AudioBlock<float> block (workBuffer.getArrayOfWritePointers(),
																										(size_t) numChannels, (size_t) numSamples);
								juce::dsp::ProcessContextReplacing<float> context (block);
								toneFilter.process (context);

								const auto trimStart = trim.getCurrentValue();
								const auto trimEnd = trim.skip (juce::jmax (1, numSamples));

								for (int ch = 0; ch < numChannels; ++ch)
										workBuffer.applyGainRamp (ch, 0, numSamples, trimStart, trimEnd);

								const auto currentMix = mix.getNextValue();
								mix.skip (juce::jmax (0, numSamples - 1));

								for (int ch = 0; ch < numChannels; ++ch)
								{
												auto* dst = buffer.getWritePointer (ch);
												const auto* wet = workBuffer.getReadPointer (ch);

												for (int n = 0; n < numSamples; ++n)
																dst[n] = dst[n] + (wet[n] - dst[n]) * currentMix;
								}
				}
}
