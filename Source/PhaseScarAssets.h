#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
		/**
				Central, safe access point for the embedded binary image assets.

				Every getter returns an image that may be invalid (null) if the resource
				was not embedded. Callers must always check isValid() and draw a JUCE
				fallback instead - nothing here ever throws or asserts.
		*/
		class Assets
		{
		public:
				static const juce::Image& logo();
				static const juce::Image& centrePanel();
				static const juce::Image& background();
				static const juce::Image& symbol();
				static const juce::Image& icons();

				/** Bounding box of the visible artwork inside the logo image.

						Detected once at runtime by scanning for pixels that differ from the
						image's empty background (transparent or flat border colour). Lets the
						editor draw only the artwork without ever modifying the source PNG.
				*/
				static juce::Rectangle<int> logoContentArea();

				/** Names of assets that failed to load, for diagnostics. */
				static juce::StringArray getMissingAssetNames();

		private:
				static juce::Image loadByName (const char* resourceName);
				static juce::Rectangle<int> detectContentArea (const juce::Image& image);
		};
}
