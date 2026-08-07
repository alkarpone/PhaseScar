#include "SpectrumAnalyzerComponent.h"
#include "PhaseScarLookAndFeel.h"
#include "DSP/PhaseScarParameters.h"

using LnF = PhaseScar::PhaseScarLookAndFeel;

namespace PhaseScar
{
	namespace
	{
		constexpr float logMin = 1.30103f;   // log10(20)
		constexpr float logMax = 4.30103f;   // log10(20000)

		constexpr int handleRadius = 5;

		// Frequency label marks shown along the bottom of the analyzer.
		const std::array<std::pair<float, const char*>, 10> frequencyMarks {{
			{ 20.0f,    "20" },
			{ 50.0f,    "50" },
			{ 100.0f,   "100" },
			{ 200.0f,   "200" },
			{ 500.0f,   "500" },
			{ 1000.0f,  "1k" },
			{ 2000.0f,  "2k" },
			{ 5000.0f,  "5k" },
			{ 10000.0f, "10k" },
			{ 20000.0f, "20k" }
		}};

		const std::array<float, 5> dbMarks {{ 12.0f, 0.0f, -12.0f, -24.0f, -36.0f }};
	}

	//==========================================================================
	SpectrumAnalyzerComponent::SpectrumAnalyzerComponent (PhaseScarAudioProcessor& processor)
		: processorRef (processor)
	{
		smoothedMagnitudesDb.fill (minDb);
		magnitudesDb.fill (minDb);
		fifoAccumulator.fill (0.0f);

		fifoReadBuffer.resize (8192);

		centerParameter = processorRef.apvts.getParameter (ParamID::notchCenter);
		spreadParameter = processorRef.apvts.getParameter (ParamID::notchSpread);

		setInterceptsMouseClicks (true, false);
		startTimerHz (30);
	}

	SpectrumAnalyzerComponent::~SpectrumAnalyzerComponent()
	{
		stopTimer();
	}

	//==========================================================================
	void SpectrumAnalyzerComponent::resized() {}

	//==========================================================================
	float SpectrumAnalyzerComponent::frequencyToX (float frequencyHz) const noexcept
	{
		const auto bounds = getLocalBounds().toFloat();
		const auto clamped = juce::jlimit (minFrequency, maxFrequency, frequencyHz);
		const auto logF = std::log10 (clamped);
		const auto proportion = (logF - logMin) / (logMax - logMin);
		return bounds.getX() + proportion * bounds.getWidth();
	}

	float SpectrumAnalyzerComponent::xToFrequency (float x) const noexcept
	{
		const auto bounds = getLocalBounds().toFloat();
		const auto proportion = juce::jlimit (0.0f, 1.0f, (x - bounds.getX()) / juce::jmax (1.0f, bounds.getWidth()));
		const auto logF = logMin + proportion * (logMax - logMin);
		return std::pow (10.0f, logF);
	}

	float SpectrumAnalyzerComponent::dbToY (float db) const noexcept
	{
		const auto bounds = getLocalBounds().toFloat();
		const auto clamped = juce::jlimit (minDb, maxDb, db);
		const auto proportion = (maxDb - clamped) / (maxDb - minDb);
		return bounds.getY() + proportion * bounds.getHeight();
	}

	//==========================================================================
	SpectrumAnalyzerComponent::NotchFrequencies SpectrumAnalyzerComponent::computeNotchFrequencies() const noexcept
	{
		NotchFrequencies result;

		auto& apvts = processorRef.apvts;

		result.enabled = apvts.getRawParameterValue (ParamID::notchEnabled)->load() > 0.5f;

		const auto centreHz = apvts.getRawParameterValue (ParamID::notchCenter)->load();
		auto spreadOctaves = apvts.getRawParameterValue (ParamID::notchSpread)->load();
		auto q = apvts.getRawParameterValue (ParamID::notchQ)->load();
		const auto depthPercent = apvts.getRawParameterValue (ParamID::notchDepth)->load();

		const auto sampleRate = processorRef.getSampleRate() > 0.0 ? processorRef.getSampleRate() : 44100.0;
		const auto nyquist = (float) (sampleRate * 0.5);

		q = juce::jlimit (0.2f, 20.0f, q);
		spreadOctaves = juce::jlimit (0.0f, 4.0f, spreadOctaves);

		// GUI preview intentionally ignores Motion/LFO modulation - it shows the
		// filter's resting position, exactly mirroring NotchBankProcessor::updateParameters
		// with modOctaves == 0.
		const auto modulatedCentre = juce::jlimit (20.0f, nyquist * 0.9f, centreHz);

		result.lowHz  = juce::jlimit (20.0f, nyquist * 0.9f, modulatedCentre * std::pow (2.0f, -spreadOctaves * 0.5f));
		result.highHz = juce::jlimit (20.0f, nyquist * 0.9f, modulatedCentre * std::pow (2.0f,  spreadOctaves * 0.5f));
		result.stageQ = juce::jlimit (0.2f, 40.0f, q * (float) 3 * 0.6f);   // NotchBankProcessor::numStages == 3
		result.depth  = juce::jlimit (0.0f, 1.0f, depthPercent / 100.0f);

		return result;
	}

	float SpectrumAnalyzerComponent::notchResponseDb (const NotchFrequencies& freqs, float frequencyHz) const noexcept
	{
		const auto sampleRate = processorRef.getSampleRate() > 0.0 ? processorRef.getSampleRate() : 44100.0;

		const auto lowCoeffs  = juce::dsp::IIR::Coefficients<float>::makeNotch (sampleRate, freqs.lowHz,  freqs.stageQ);
		const auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeNotch (sampleRate, freqs.highHz, freqs.stageQ);

		const auto lowMag  = lowCoeffs->getMagnitudeForFrequency  ((double) frequencyHz, sampleRate);
		const auto highMag = highCoeffs->getMagnitudeForFrequency ((double) frequencyHz, sampleRate);

		// Cascaded stages (numStages == 3) multiply magnitudes; the two notch banks
		// are in series, so their combined magnitude also multiplies.
		const auto totalMag = std::pow (lowMag, 3.0) * std::pow (highMag, 3.0);

		// Depth linearly mixes wet into dry (see NotchBankProcessor::process),
		// which is equivalent to blending the magnitude towards 1.0 (unity).
		const auto mixedMag = 1.0 + (totalMag - 1.0) * (double) freqs.depth;

		return (float) juce::Decibels::gainToDecibels (juce::jmax (1.0e-6, mixedMag));
	}

	//==========================================================================
	void SpectrumAnalyzerComponent::pushSampleIntoFft (float sample) noexcept
	{
		fifoAccumulator[(size_t) fifoIndex++] = sample;

		if (fifoIndex == fftSize)
		{
			fifoIndex = 0;

			std::copy (fifoAccumulator.begin(), fifoAccumulator.end(), fftData.begin());
			std::fill (fftData.begin() + fftSize, fftData.end(), 0.0f);

			window.multiplyWithWindowingTable (fftData.data(), (size_t) fftSize);
			fft.performFrequencyOnlyForwardTransform (fftData.data());

			const auto sampleRate = processorRef.getSampleRate() > 0.0 ? processorRef.getSampleRate() : 44100.0;
			const auto binWidth = (float) (sampleRate / (double) fftSize);

			for (int bin = 0; bin < numBins; ++bin)
			{
				const auto frequency = bin * binWidth;
				juce::ignoreUnused (frequency);

				const auto magnitude = fftData[(size_t) bin] / (float) fftSize;
				const auto db = juce::Decibels::gainToDecibels (magnitude, minDb - 12.0f);
				magnitudesDb[(size_t) bin] = db;
			}

			constexpr float smoothingCoeff = 0.35f;

			if (! smoothingInitialised)
			{
				smoothedMagnitudesDb = magnitudesDb;
				smoothingInitialised = true;
			}
			else
			{
				for (int bin = 0; bin < numBins; ++bin)
				{
					auto& smoothed = smoothedMagnitudesDb[(size_t) bin];
					smoothed += (magnitudesDb[(size_t) bin] - smoothed) * smoothingCoeff;
				}
			}
		}
	}

	void SpectrumAnalyzerComponent::pullFifoAndUpdateFft()
	{
		auto& fifo = processorRef.spectrumFifo;

		int numRead;

		do
		{
			const auto toRead = juce::jmin ((int) fifoReadBuffer.size(), fifo.getNumReady());

			if (toRead <= 0)
				break;

			numRead = fifo.pop (fifoReadBuffer.data(), toRead);

			for (int i = 0; i < numRead; ++i)
				pushSampleIntoFft (fifoReadBuffer[(size_t) i]);
		}
		while (numRead > 0 && fifo.getNumReady() > 0);
	}

	void SpectrumAnalyzerComponent::timerCallback()
	{
		pullFifoAndUpdateFft();
		repaint();
	}

	//==========================================================================
	void SpectrumAnalyzerComponent::paint (juce::Graphics& g)
	{
		auto bounds = getLocalBounds().toFloat();

		g.saveState();
		g.reduceClipRegion (getLocalBounds());

		drawBackground (g, bounds);
		drawGrid (g, bounds);
		drawSpectrum (g, bounds);
		drawZeroDbReference (g, bounds);

		const auto freqs = computeNotchFrequencies();
		drawNotchResponse (g, bounds, freqs);
		drawHandles (g, bounds, freqs);

		g.restoreState();
	}

	void SpectrumAnalyzerComponent::drawBackground (juce::Graphics& g, juce::Rectangle<float> bounds) const
	{
		g.setColour (LnF::panelFillDark);
		g.fillRect (bounds);
	}

	void SpectrumAnalyzerComponent::drawGrid (juce::Graphics& g, juce::Rectangle<float> bounds) const
	{
		g.setColour (LnF::outline.withAlpha (0.25f));

		for (auto& mark : frequencyMarks)
		{
			const auto x = frequencyToX (mark.first);
			g.drawVerticalLine ((int) x, bounds.getY(), bounds.getBottom());
		}

		for (auto db : dbMarks)
		{
			const auto y = dbToY (db);
			g.drawHorizontalLine ((int) y, bounds.getX(), bounds.getRight());
		}

		g.setFont (juce::Font (juce::FontOptions (9.0f)));
		g.setColour (LnF::textDim.withAlpha (0.7f));

		for (auto& mark : frequencyMarks)
		{
			const auto x = frequencyToX (mark.first);
			g.drawText (mark.second, juce::Rectangle<float> (x - 14.0f, bounds.getBottom() - 12.0f, 28.0f, 11.0f),
						juce::Justification::centred, false);
		}

		for (auto db : dbMarks)
		{
			const auto y = dbToY (db);
			g.drawText (juce::String (db, 0), juce::Rectangle<float> (bounds.getX() + 2.0f, y - 10.0f, 26.0f, 10.0f),
						juce::Justification::centredLeft, false);
		}
	}

	void SpectrumAnalyzerComponent::drawSpectrum (juce::Graphics& g, juce::Rectangle<float> bounds) const
	{
		if (! smoothingInitialised)
			return;

		const auto sampleRate = processorRef.getSampleRate() > 0.0 ? processorRef.getSampleRate() : 44100.0;
		const auto binWidth = (float) (sampleRate / (double) fftSize);

		juce::Path path;
		bool started = false;

		const auto pixelWidth = juce::jmax (1, (int) bounds.getWidth());

		for (int px = 0; px < pixelWidth; ++px)
		{
			const auto x = bounds.getX() + (float) px;
			const auto frequency = xToFrequency (x);

			auto bin = (int) (frequency / binWidth);
			bin = juce::jlimit (0, numBins - 1, bin);

			const auto db = smoothedMagnitudesDb[(size_t) bin];
			const auto y = dbToY (db);

			if (! started)
			{
				path.startNewSubPath (x, y);
				started = true;
			}
			else
			{
				path.lineTo (x, y);
			}
		}

		g.setColour (juce::Colour (0xffe0306a));   // red/magenta
		g.strokePath (path, juce::PathStrokeType (1.4f));
	}

	void SpectrumAnalyzerComponent::drawZeroDbReference (juce::Graphics& g, juce::Rectangle<float> bounds) const
	{
		const auto y = dbToY (0.0f);
		g.setColour (LnF::textDim.withAlpha (0.35f));
		g.drawHorizontalLine ((int) y, bounds.getX(), bounds.getRight());
	}

	void SpectrumAnalyzerComponent::drawNotchResponse (juce::Graphics& g, juce::Rectangle<float> bounds, const NotchFrequencies& freqs) const
	{
		juce::Path path;
		bool started = false;

		const auto pixelWidth = juce::jmax (1, (int) bounds.getWidth());

		for (int px = 0; px < pixelWidth; ++px)
		{
			const auto x = bounds.getX() + (float) px;
			const auto frequency = xToFrequency (x);

			const auto db = freqs.enabled ? notchResponseDb (freqs, frequency) : 0.0f;
			const auto y = dbToY (db);

			if (! started)
			{
				path.startNewSubPath (x, y);
				started = true;
			}
			else
			{
				path.lineTo (x, y);
			}
		}

		const auto colour = juce::Colour (0xffb040ff);   // purple
		g.setColour (freqs.enabled ? colour : colour.withAlpha (0.25f));
		g.strokePath (path, juce::PathStrokeType (1.8f));
	}

	void SpectrumAnalyzerComponent::drawHandles (juce::Graphics& g, juce::Rectangle<float> bounds, const NotchFrequencies& freqs) const
	{
		juce::ignoreUnused (bounds);

		const auto drawHandle = [&] (float frequencyHz, const char* label)
		{
			const auto x = frequencyToX (frequencyHz);
			const auto db = freqs.enabled ? notchResponseDb (freqs, frequencyHz) : 0.0f;
			const auto y = dbToY (db);

			const auto colour = juce::Colour (0xffb040ff);
			g.setColour (freqs.enabled ? colour : colour.withAlpha (0.35f));
			g.fillEllipse (x - (float) handleRadius, y - (float) handleRadius,
							 (float) handleRadius * 2.0f, (float) handleRadius * 2.0f);

			g.setColour (juce::Colours::white.withAlpha (freqs.enabled ? 0.9f : 0.4f));
			g.setFont (juce::Font (juce::FontOptions (9.0f, juce::Font::bold)));
			g.drawText (label, juce::Rectangle<float> (x - 10.0f, y - (float) handleRadius - 14.0f, 20.0f, 12.0f),
						juce::Justification::centred, false);
		};

		drawHandle (freqs.lowHz,  "1");
		drawHandle (freqs.highHz, "2");
	}

	//==========================================================================
	void SpectrumAnalyzerComponent::mouseDown (const juce::MouseEvent& event)
	{
		const auto freqs = computeNotchFrequencies();

		const auto lowX  = frequencyToX (freqs.lowHz);
		const auto highX = frequencyToX (freqs.highHz);

		const auto distanceToLow  = std::abs (event.position.x - lowX);
		const auto distanceToHigh = std::abs (event.position.x - highX);

		constexpr float grabRadius = 12.0f;

		if (distanceToLow > grabRadius && distanceToHigh > grabRadius)
		{
			draggedHandle = DraggedHandle::none;
			return;
		}

		draggedHandle = (distanceToLow <= distanceToHigh) ? DraggedHandle::low : DraggedHandle::high;

		if (centerParameter != nullptr)
			centerParameter->beginChangeGesture();

		if (spreadParameter != nullptr)
			spreadParameter->beginChangeGesture();
	}

	void SpectrumAnalyzerComponent::mouseDrag (const juce::MouseEvent& event)
	{
		if (draggedHandle == DraggedHandle::none || centerParameter == nullptr || spreadParameter == nullptr)
			return;

		const auto freqs = computeNotchFrequencies();

		auto lowHz  = freqs.lowHz;
		auto highHz = freqs.highHz;

		const auto newFrequency = juce::jlimit (minFrequency, maxFrequency, xToFrequency (event.position.x));

		if (draggedHandle == DraggedHandle::low)
			lowHz = juce::jmin (newFrequency, highHz * 0.98f);
		else
			highHz = juce::jmax (newFrequency, lowHz * 1.02f);

		// Exact inverse of NotchBankProcessor's symmetric octave-spacing maths:
		//   lowHz  = centre * 2^(-spread/2)
		//   highHz = centre * 2^( spread/2)
		// => centre = sqrt(lowHz * highHz), spread = log2(highHz / lowHz)
		const auto newCentre = std::sqrt (lowHz * highHz);
		const auto newSpread = std::log2 (highHz / lowHz);

		const auto& centerRange = centerParameter->getNormalisableRange();
		const auto& spreadRange = spreadParameter->getNormalisableRange();

		const auto centreNorm = centerRange.convertTo0to1 (juce::jlimit (centerRange.start, centerRange.end, newCentre));
		const auto spreadNorm = spreadRange.convertTo0to1 (juce::jlimit (spreadRange.start, spreadRange.end, newSpread));

		centerParameter->setValueNotifyingHost (centreNorm);
		spreadParameter->setValueNotifyingHost (spreadNorm);
	}

	void SpectrumAnalyzerComponent::mouseUp (const juce::MouseEvent&)
	{
		if (draggedHandle == DraggedHandle::none)
			return;

		if (centerParameter != nullptr)
			centerParameter->endChangeGesture();

		if (spreadParameter != nullptr)
			spreadParameter->endChangeGesture();

		draggedHandle = DraggedHandle::none;
	}
}
