#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
		/**
				Lock free transport of meter values from the audio thread to the UI.
				The editor polls this with a timer; nothing here ever blocks.
		*/
		class MeteringData
		{
		public:
				void reset() noexcept
				{
						inputPeak.store (0.0f, std::memory_order_relaxed);
						outputPeak.store (0.0f, std::memory_order_relaxed);
						motionValue.store (0.0f, std::memory_order_relaxed);
						envelopeValue.store (0.0f, std::memory_order_relaxed);
				}

				void pushInputPeak (float value) noexcept   { pushPeak (inputPeak, value); }
				void pushOutputPeak (float value) noexcept  { pushPeak (outputPeak, value); }

				void setMotionValue (float value) noexcept   { motionValue.store (value, std::memory_order_relaxed); }
				void setEnvelopeValue (float value) noexcept { envelopeValue.store (value, std::memory_order_relaxed); }

				float getInputPeak() const noexcept    { return inputPeak.load (std::memory_order_relaxed); }
				float getOutputPeak() const noexcept   { return outputPeak.load (std::memory_order_relaxed); }
				float getMotionValue() const noexcept  { return motionValue.load (std::memory_order_relaxed); }
				float getEnvelopeValue() const noexcept { return envelopeValue.load (std::memory_order_relaxed); }

		private:
				static void pushPeak (std::atomic<float>& target, float value) noexcept
				{
						if (! std::isfinite (value))
								value = 0.0f;

						// Simple peak hold with decay handled on the UI side.
						const auto previous = target.load (std::memory_order_relaxed);
						target.store (juce::jmax (previous * 0.85f, value), std::memory_order_relaxed);
				}

				std::atomic<float> inputPeak { 0.0f };
				std::atomic<float> outputPeak { 0.0f };
				std::atomic<float> motionValue { 0.0f };
				std::atomic<float> envelopeValue { 0.0f };
		};
}
