#include "PostEQProcessor.h"

namespace PhaseScar
{
				void PostEQProcessor::prepare (const juce::dsp::ProcessSpec& spec)
				{
								sampleRate = spec.sampleRate;

								lowShelf.prepare (spec);
								midPeak.prepare (spec);
								highShelf.prepare (spec);

								lastLowGainDb = -1000.0f;
								lastMidFrequencyHz = -1.0f;
								lastMidGainDb = -1000.0f;
								lastMidQ = -1.0f;
								lastHighGainDb = -1000.0f;

								reset();
				}

				void PostEQProcessor::reset()
				{
								lowShelf.reset();
								midPeak.reset();
								highShelf.reset();
				}

				void PostEQProcessor::updateParameters (float lowGainDb, float midFrequencyHz, float midGainDb,
																								float midQ, float highGainDb) noexcept
				{
								const auto nyquist = (float) (sampleRate * 0.5);

								lowGainDb  = juce::jlimit (-18.0f, 18.0f, lowGainDb);
								highGainDb = juce::jlimit (-18.0f, 18.0f, highGainDb);
								midGainDb  = juce::jlimit (-18.0f, 18.0f, midGainDb);
								midQ       = juce::jlimit (0.2f, 10.0f, midQ);
								midFrequencyHz = juce::jlimit (100.0f, juce::jmin (10000.0f, nyquist * 0.9f), midFrequencyHz);

								if (! juce::approximatelyEqual (lowGainDb, lastLowGainDb))
								{
												lastLowGainDb = lowGainDb;
												*lowShelf.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
																sampleRate, juce::jmin (lowShelfHz, nyquist * 0.9f), 0.7071f,
																juce::Decibels::decibelsToGain (lowGainDb));
								}

								if (! juce::approximatelyEqual (midFrequencyHz, lastMidFrequencyHz)
										|| ! juce::approximatelyEqual (midGainDb, lastMidGainDb)
										|| ! juce::approximatelyEqual (midQ, lastMidQ))
								{
												lastMidFrequencyHz = midFrequencyHz;
												lastMidGainDb = midGainDb;
												lastMidQ = midQ;

												*midPeak.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
																sampleRate, midFrequencyHz, midQ, juce::Decibels::decibelsToGain (midGainDb));
								}

								if (! juce::approximatelyEqual (highGainDb, lastHighGainDb))
								{
												lastHighGainDb = highGainDb;
												*highShelf.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
																sampleRate, juce::jmin (highShelfHz, nyquist * 0.9f), 0.7071f,
																juce::Decibels::decibelsToGain (highGainDb));
								}
				}

				void PostEQProcessor::process (juce::AudioBuffer<float>& buffer) noexcept
				{
								juce::dsp::AudioBlock<float> block (buffer);
								juce::dsp::ProcessContextReplacing<float> context (block);

								lowShelf.process (context);
								midPeak.process (context);
								highShelf.process (context);
				}
}
