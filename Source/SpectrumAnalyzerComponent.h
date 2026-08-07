#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace PhaseScar
{
	/**
				Central spectrum analyzer overlay.

				Shows a real-time FFT spectrum of the post-processing mono (L+R) signal,
				an analytic overlay of the two notch filters' response (using exactly the
				same Center/Spread/Q/Depth maths as NotchBankProcessor), and two draggable
				handles that control the notchCenter / notchSpread APVTS parameters.

				All FFT work and repaint calls happen on the message thread (Timer
				callback). The audio thread only ever pushes samples into a lock-free
				ring buffer (see SpectrumFifo).
	*/
	class SpectrumAnalyzerComponent : public juce::Component,
										 private juce::Timer
	{
	public:
		explicit SpectrumAnalyzerComponent (PhaseScarAudioProcessor& processor);
		~SpectrumAnalyzerComponent() override;

		void paint (juce::Graphics& g) override;
		void resized() override;

		void mouseDown (const juce::MouseEvent& event) override;
		void mouseDrag (const juce::MouseEvent& event) override;
		void mouseUp (const juce::MouseEvent& event) override;

	private:
		void timerCallback() override;

		void pullFifoAndUpdateFft();
		void pushSampleIntoFft (float sample) noexcept;

		//======================================================================
		// Screen <-> frequency mapping helpers
		float frequencyToX (float frequencyHz) const noexcept;
		float xToFrequency (float x) const noexcept;
		float dbToY (float db) const noexcept;

		//======================================================================
		// Notch response, mirrors NotchBankProcessor's maths exactly.
		struct NotchFrequencies
		{
			float lowHz = 0.0f;
			float highHz = 0.0f;
			float stageQ = 1.0f;
			float depth = 0.0f;
			bool enabled = false;
		};

		NotchFrequencies computeNotchFrequencies() const noexcept;
		float notchResponseDb (const NotchFrequencies& freqs, float frequencyHz) const noexcept;

		//======================================================================
		// Drawing layers
		void drawBackground (juce::Graphics& g, juce::Rectangle<float> bounds) const;
		void drawGrid (juce::Graphics& g, juce::Rectangle<float> bounds) const;
		void drawSpectrum (juce::Graphics& g, juce::Rectangle<float> bounds) const;
		void drawZeroDbReference (juce::Graphics& g, juce::Rectangle<float> bounds) const;
		void drawNotchResponse (juce::Graphics& g, juce::Rectangle<float> bounds, const NotchFrequencies& freqs) const;
		void drawHandles (juce::Graphics& g, juce::Rectangle<float> bounds, const NotchFrequencies& freqs) const;

		// Builds a smoothly-curved path (Catmull-Rom interpolated) through the
		// supplied per-pixel points, rather than connecting them with straight
		// line segments - this removes the "jagged"/staircase look.
		static juce::Path buildSmoothPath (const std::vector<juce::Point<float>>& points);

		float interpolatedMagnitudeDb (float frequencyHz) const noexcept;

		//======================================================================
		PhaseScarAudioProcessor& processorRef;

		static constexpr int fftOrder = 11;                 // 2^11 = 2048
		static constexpr int fftSize = 1 << fftOrder;

		juce::dsp::FFT fft { fftOrder };
		juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann };

		std::array<float, fftSize * 2> fftData {};
		std::array<float, fftSize> fifoAccumulator {};
		int fifoIndex = 0;

		static constexpr int numBins = fftSize / 2;
		std::array<float, numBins> magnitudesDb {};
		std::array<float, numBins> smoothedMagnitudesDb {};
		bool smoothingInitialised = false;

		std::vector<float> fifoReadBuffer;

		static constexpr float minFrequency = 20.0f;
		static constexpr float maxFrequency = 20000.0f;
		static constexpr float maxDb = 12.0f;
		static constexpr float minDb = -48.0f;

		//======================================================================
		// Drag state for the two notch handles
		enum class DraggedHandle { none, low, high };
		DraggedHandle draggedHandle = DraggedHandle::none;
		juce::RangedAudioParameter* centerParameter = nullptr;
		juce::RangedAudioParameter* spreadParameter = nullptr;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzerComponent)
	};
}
