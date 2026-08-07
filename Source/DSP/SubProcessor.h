#pragma once

#include "DistortionProcessor.h"

namespace BassMutator
{
		/**
				SUB band: kept clean, controlled and mono compatible.

				Chain: safety high-pass -> optional mono fold -> gentle saturation -> gain.
		*/
		class SubProcessor
		{
		public:
				void prepare (const juce::dsp::ProcessSpec& spec);
				void reset();

				void updateParameters (float gainDb, float saturationPercent, bool mono);

				void process (juce::AudioBuffer<float>& band) noexcept;

		private:
				juce::dsp::StateVariableTPTFilter<float> safetyHighPass;

				juce::SmoothedValue<float> gain { 1.0f };
				juce::SmoothedValue<float> saturation { 0.0f };
				juce::SmoothedValue<float> monoAmount { 1.0f };

				// Per-channel DC blocker state.
				std::array<float, 2> dcLastInput { {} };
				std::array<float, 2> dcLastOutput { {} };
		};
}
