#pragma once

#include <JuceHeader.h>
#include "PhaseScarParameters.h"

namespace PhaseScar
{
				/**
								Single band distortion stage: drive, bias, waveshaper, tone low pass
								and a parallel dry/wet mix. Oversampling is not implemented yet, but
								the interface is ready for it.
				*/
				class DistortionStageProcessor
				{
				public:
								void prepare (const juce::dsp::ProcessSpec& spec);
								void reset();

								void updateParameters (float driveDb, DistortionMode mode, float bias,
																			 float toneHz, float trimDb, float mixPercent) noexcept;

								void process (juce::AudioBuffer<float>& buffer) noexcept;

				private:
								using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
																															juce::dsp::IIR::Coefficients<float>>;

								Filter toneFilter;
								juce::AudioBuffer<float> workBuffer;

								juce::SmoothedValue<float> drive, bias, trim, mix;

								double sampleRate = 44100.0;
								DistortionMode currentMode = DistortionMode::softClip;
								float lastToneHz = -1.0f;
				};
}
