#include "NotchBankProcessor.h"

namespace PhaseScar
{
				void NotchBankProcessor::prepare (const juce::dsp::ProcessSpec& spec)
				{
								sampleRate = spec.sampleRate;

								for (auto& stage : notchLow)
										stage.prepare (spec);

								for (auto& stage : notchHigh)
										stage.prepare (spec);

								workBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize, false, true, false);

								lastLowHz = -1.0f;
								lastHighHz = -1.0f;
								lastQ = -1.0f;

								reset();
				}

				void NotchBankProcessor::reset()
				{
								for (auto& stage : notchLow)
										stage.reset();

								for (auto& stage : notchHigh)
										stage.reset();

								workBuffer.clear();
				}

				void NotchBankProcessor::updateParameters (float centreHz, float spreadOctaves, float q,
																									 float depthPercent, float motionPercent, float modulation) noexcept
				{
								const auto nyquist = (float) (sampleRate * 0.5);

								depth = juce::jlimit (0.0f, 1.0f, depthPercent / 100.0f);
								q = juce::jlimit (0.2f, 20.0f, q);
								spreadOctaves = juce::jlimit (0.0f, 4.0f, spreadOctaves);

								const auto motion = juce::jlimit (0.0f, 1.0f, motionPercent / 100.0f);
								const auto modOctaves = juce::jlimit (-1.0f, 1.0f, modulation) * motion;

								const auto modulatedCentre = juce::jlimit (20.0f, nyquist * 0.9f,
																													 centreHz * std::pow (2.0f, modOctaves));

								const auto lowHz  = juce::jlimit (20.0f, nyquist * 0.9f, modulatedCentre * std::pow (2.0f, -spreadOctaves * 0.5f));
								const auto highHz = juce::jlimit (20.0f, nyquist * 0.9f, modulatedCentre * std::pow (2.0f,  spreadOctaves * 0.5f));

								// Cascading identical stages widens the notch, so the individual stages
								// are made narrower to keep the requested Q character while cutting deeper.
								const auto stageQ = juce::jlimit (0.2f, 40.0f, q * (float) numStages * 0.6f);

								if (! juce::approximatelyEqual (lowHz, lastLowHz) || ! juce::approximatelyEqual (q, lastQ))
								{
												lastLowHz = lowHz;

												const auto lowCoefficients = juce::dsp::IIR::Coefficients<float>::makeNotch (sampleRate, lowHz, stageQ);

												for (auto& stage : notchLow)
														*stage.state = *lowCoefficients;
								}

								if (! juce::approximatelyEqual (highHz, lastHighHz) || ! juce::approximatelyEqual (q, lastQ))
								{
												lastHighHz = highHz;

												const auto highCoefficients = juce::dsp::IIR::Coefficients<float>::makeNotch (sampleRate, highHz, stageQ);

												for (auto& stage : notchHigh)
														*stage.state = *highCoefficients;
								}

								lastQ = q;
				}

				void NotchBankProcessor::process (juce::AudioBuffer<float>& buffer) noexcept
				{
								const auto numSamples = buffer.getNumSamples();
								const auto numChannels = buffer.getNumChannels();

								if (numSamples <= 0 || numChannels <= 0)
												return;

								if (workBuffer.getNumChannels() < numChannels || workBuffer.getNumSamples() < numSamples)
												return;

								for (int ch = 0; ch < numChannels; ++ch)
												workBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);

								juce::dsp::AudioBlock<float> block (workBuffer.getArrayOfWritePointers(),
																										(size_t) numChannels, (size_t) numSamples);
								juce::dsp::ProcessContextReplacing<float> context (block);

								for (auto& stage : notchLow)
										stage.process (context);

								for (auto& stage : notchHigh)
										stage.process (context);

								for (int ch = 0; ch < numChannels; ++ch)
								{
												auto* dst = buffer.getWritePointer (ch);
												const auto* wet = workBuffer.getReadPointer (ch);

												for (int n = 0; n < numSamples; ++n)
																dst[n] = dst[n] + (wet[n] - dst[n]) * depth;
								}
				}
}
