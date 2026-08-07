#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
		/**
				Safety stage, not a mastering limiter.

				Removes NaN/Inf values, blocks DC and applies a gentle soft clip so an
				accidental extreme setting can never produce a dangerous output level.
		*/
		class OutputProtection
		{
		public:
				void prepare (const juce::dsp::ProcessSpec& spec);
				void reset();

				void process (juce::AudioBuffer<float>& buffer) noexcept;

		private:
				static constexpr float dcBlockerPole = 0.9995f;
				static constexpr int maxChannels = 8;

				std::array<float, maxChannels> dcLastInput { {} };
				std::array<float, maxChannels> dcLastOutput { {} };
		};
}
