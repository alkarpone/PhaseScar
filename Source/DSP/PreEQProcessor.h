#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
				/**
								Simple pre-processing EQ: high pass, low pass and a broad tilt.
								Deliberately minimal - a full graphical EQ comes later.
				*/
				class PreEQProcessor
				{
				public:
								void prepare (const juce::dsp::ProcessSpec& spec);
								void reset();

								void updateParameters (float highPassHz, float lowPassHz, float tiltDb) noexcept;
								void process (juce::AudioBuffer<float>& buffer) noexcept;

				private:
								using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
																															juce::dsp::IIR::Coefficients<float>>;

								Filter highPass, lowPass, lowTilt, highTilt;

								double sampleRate = 44100.0;
								float lastHighPassHz = -1.0f;
								float lastLowPassHz = -1.0f;
								float lastTiltDb = -1000.0f;
				};
}
