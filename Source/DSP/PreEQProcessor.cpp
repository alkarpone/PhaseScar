#include "PreEQProcessor.h"

namespace PhaseScar
{
				void PreEQProcessor::prepare (const juce::dsp::ProcessSpec& spec)
				{
								sampleRate = spec.sampleRate;

								highPass.prepare (spec);
								lowPass.prepare (spec);
								lowTilt.prepare (spec);
								highTilt.prepare (spec);

								lastHighPassHz = -1.0f;
								lastLowPassHz = -1.0f;
								lastTiltDb = -1000.0f;

								reset();
				}

				void PreEQProcessor::reset()
				{
								highPass.reset();
								lowPass.reset();
								lowTilt.reset();
								highTilt.reset();
				}

				void PreEQProcessor::updateParameters (float highPassHz, float lowPassHz, float tiltDb) noexcept
				{
								const auto nyquist = (float) (sampleRate * 0.5);
								highPassHz = juce::jlimit (20.0f, juce::jmin (1000.0f, nyquist * 0.9f), highPassHz);
								lowPassHz  = juce::jlimit (1000.0f, juce::jmin (22000.0f, nyquist * 0.95f), lowPassHz);
								tiltDb     = juce::jlimit (-12.0f, 12.0f, tiltDb);

								if (! juce::approximatelyEqual (highPassHz, lastHighPassHz))
								{
												lastHighPassHz = highPassHz;
												*highPass.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, highPassHz);
								}

								if (! juce::approximatelyEqual (lowPassHz, lastLowPassHz))
								{
												lastLowPassHz = lowPassHz;
												*lowPass.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, lowPassHz);
								}

								if (! juce::approximatelyEqual (tiltDb, lastTiltDb))
								{
												lastTiltDb = tiltDb;

												const auto lowGain  = juce::Decibels::decibelsToGain (-tiltDb * 0.5f);
												const auto highGain = juce::Decibels::decibelsToGain (tiltDb * 0.5f);

												*lowTilt.state  = *juce::dsp::IIR::Coefficients<float>::makeLowShelf  (sampleRate, 250.0f, 0.7071f, lowGain);
												*highTilt.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, 3000.0f, 0.7071f, highGain);
								}
				}

				void PreEQProcessor::process (juce::AudioBuffer<float>& buffer) noexcept
				{
								juce::dsp::AudioBlock<float> block (buffer);
								juce::dsp::ProcessContextReplacing<float> context (block);

								highPass.process (context);
								lowPass.process (context);
								lowTilt.process (context);
								highTilt.process (context);
				}
}
