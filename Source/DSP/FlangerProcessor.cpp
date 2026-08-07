#include "FlangerProcessor.h"

namespace PhaseScar
{
				void FlangerProcessor::prepare (const juce::dsp::ProcessSpec& spec)
				{
								sampleRate = spec.sampleRate;

								delayBufferLength = juce::jmax (16, (int) std::ceil (sampleRate * (maxDelayMs / 1000.0)) + 4);
								delayBuffer.setSize (2, delayBufferLength, false, true, false);

								const auto rampSeconds = 0.05;
								baseDelaySamples.reset (sampleRate, rampSeconds);
								depth.reset (sampleRate, rampSeconds);
								feedback.reset (sampleRate, rampSeconds);
								mix.reset (sampleRate, rampSeconds);

								reset();
				}

				void FlangerProcessor::reset()
				{
								delayBuffer.clear();
								writePosition.fill (0);
								lfoPhase = 0.0;
								lastLfoValue = 0.0f;
				}

				void FlangerProcessor::updateParameters (float rateHz, float depthPercent, float baseDelayMs,
																								 float feedbackPercent, float mixPercent, float stereoPhaseDegrees) noexcept
				{
								rateHz = juce::jlimit (0.05f, 10.0f, rateHz);
								lfoIncrement = rateHz / sampleRate;

								const auto maxDelaySamples = (float) (delayBufferLength - 4);
								const auto targetDelay = juce::jlimit (1.0f, maxDelaySamples * 0.5f,
																											 (float) (juce::jlimit (0.1f, maxDelayMs, baseDelayMs) * sampleRate / 1000.0));

								baseDelaySamples.setTargetValue (targetDelay);
								depth.setTargetValue (juce::jlimit (0.0f, 1.0f, depthPercent / 100.0f));
								feedback.setTargetValue (juce::jlimit (-0.95f, 0.95f, feedbackPercent / 100.0f));
								mix.setTargetValue (juce::jlimit (0.0f, 1.0f, mixPercent / 100.0f));

								stereoPhaseOffset = juce::jlimit (0.0f, 180.0f, stereoPhaseDegrees) / 360.0f;
				}

				float FlangerProcessor::readInterpolated (int channel, float delayInSamples) const noexcept
				{
								const auto* data = delayBuffer.getReadPointer (channel);

								float readPosition = (float) writePosition[(size_t) channel] - delayInSamples;
								while (readPosition < 0.0f)
												readPosition += (float) delayBufferLength;

								const auto index0 = (int) readPosition;
								const auto fraction = readPosition - (float) index0;
								const auto i0 = index0 % delayBufferLength;
								const auto i1 = (i0 + 1) % delayBufferLength;

								return data[i0] + fraction * (data[i1] - data[i0]);
				}

				void FlangerProcessor::process (juce::AudioBuffer<float>& buffer) noexcept
				{
								const auto numSamples = buffer.getNumSamples();
								const auto numChannels = juce::jmin (buffer.getNumChannels(), delayBuffer.getNumChannels());

								if (numSamples <= 0 || numChannels <= 0 || delayBufferLength <= 0)
												return;

								const auto maxDelaySamples = (float) (delayBufferLength - 4);

								for (int n = 0; n < numSamples; ++n)
								{
												const auto currentBase = baseDelaySamples.getNextValue();
												const auto currentDepth = depth.getNextValue();
												const auto currentFeedback = feedback.getNextValue();
												const auto currentMix = mix.getNextValue();

												for (int ch = 0; ch < numChannels; ++ch)
												{
																const auto phase = lfoPhase + (ch == 1 ? (double) stereoPhaseOffset : 0.0);
																const auto lfo = (float) std::sin (juce::MathConstants<double>::twoPi * phase);

																if (ch == 0)
																				lastLfoValue = lfo;

																const auto delaySamples = juce::jlimit (1.0f, maxDelaySamples,
																																				currentBase * (1.0f + 0.9f * currentDepth * lfo));

																auto* channelData = buffer.getWritePointer (ch);
																const auto input = channelData[n];
																auto delayed = readInterpolated (ch, delaySamples);

																if (! std::isfinite (delayed))
																				delayed = 0.0f;

																const auto toWrite = juce::jlimit (-4.0f, 4.0f, input + delayed * currentFeedback);
																delayBuffer.getWritePointer (ch)[writePosition[(size_t) ch]] = toWrite;

																channelData[n] = input + (delayed - input) * currentMix;
												}

												for (int ch = 0; ch < numChannels; ++ch)
																writePosition[(size_t) ch] = (writePosition[(size_t) ch] + 1) % delayBufferLength;

												lfoPhase += lfoIncrement;
												if (lfoPhase >= 1.0)
																lfoPhase -= 1.0;
								}
				}
}
