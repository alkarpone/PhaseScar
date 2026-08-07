#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
	/**
				Real-time-safe single-producer/single-consumer ring buffer that carries
				a mono (L+R) mixdown of the post-processing signal from the audio thread
				to the GUI thread.

				processBlock() only ever calls push(), which performs no allocations,
				no locks and no GUI calls - it is safe to call from the audio thread.
				The GUI reads the data out with pop() on a Timer callback.
	*/
	class SpectrumFifo
	{
	public:
		void prepare (int newCapacity)
		{
			capacity = juce::jmax (1024, newCapacity);
			buffer.setSize (1, capacity);
			buffer.clear();
			abstractFifo.setTotalSize (capacity);
		}

		void reset() noexcept
		{
			abstractFifo.reset();
			buffer.clear();
		}

		/** Real-time-safe: push a mono mixdown of the buffer. No allocations, no locks, no GUI calls. */
		void push (const juce::AudioBuffer<float>& audio) noexcept
		{
			const auto numSamples = audio.getNumSamples();
			const auto numChannels = audio.getNumChannels();

			if (numSamples <= 0 || numChannels <= 0)
				return;

			int start1, size1, start2, size2;
			abstractFifo.prepareToWrite (numSamples, start1, size1, start2, size2);

			auto* dest = buffer.getWritePointer (0);

			auto writeRange = [&] (int destStart, int srcStart, int count)
			{
				for (int n = 0; n < count; ++n)
				{
					const auto srcIndex = srcStart + n;
					float mono = audio.getSample (0, srcIndex);

					if (numChannels > 1)
						mono = 0.5f * (mono + audio.getSample (1, srcIndex));

					dest[destStart + n] = mono;
				}
			};

			if (size1 > 0)
				writeRange (start1, 0, size1);

			if (size2 > 0)
				writeRange (start2, size1, size2);

			abstractFifo.finishedWrite (size1 + size2);
		}

		/** GUI thread only: pop up to maxSamples into destination, returns number popped. */
		int pop (float* destination, int maxSamples) noexcept
		{
			int start1, size1, start2, size2;
			abstractFifo.prepareToRead (maxSamples, start1, size1, start2, size2);

			const auto* src = buffer.getReadPointer (0);

			if (size1 > 0)
				juce::FloatVectorOperations::copy (destination, src + start1, size1);

			if (size2 > 0)
				juce::FloatVectorOperations::copy (destination + size1, src + start2, size2);

			abstractFifo.finishedRead (size1 + size2);

			return size1 + size2;
		}

		int getNumReady() const noexcept { return abstractFifo.getNumReady(); }

	private:
		juce::AbstractFifo abstractFifo { 1024 };
		juce::AudioBuffer<float> buffer;
		int capacity = 1024;
	};
}
