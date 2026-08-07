#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
		/**
				Cyber-industrial look and feel for Phase Scar.

				Accent colour selection: any component (or one of its parents) may carry the
				component property "phaseScarAccent" with the value "red" to switch from the
				default neon purple accent to the restrained distortion red. This avoids
				duplicating control classes only to change a colour.
		*/
		class PhaseScarLookAndFeel : public juce::LookAndFeel_V4
		{
		public:
				PhaseScarLookAndFeel();

				//==============================================================================
				// Shared palette
				static const juce::Colour background;
				static const juce::Colour panelFill;
				static const juce::Colour panelFillDark;
				static const juce::Colour outline;
				static const juce::Colour innerOutline;
				static const juce::Colour textMain;
				static const juce::Colour textDim;
				static const juce::Colour accentPurple;
				static const juce::Colour accentRed;

				static const juce::Identifier accentPropertyId;

				/** Marks a component (and its children, by lookup) as using the red accent. */
				static void setRedAccent (juce::Component& component, bool shouldUseRed);
				static juce::Colour accentFor (const juce::Component* component);

				/** Draws a standard section panel with title. */
				static void drawPanel (juce::Graphics& g, juce::Rectangle<int> bounds,
															 const juce::String& title, juce::Colour accent);

				//==============================================================================
				void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
															 float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
															 juce::Slider&) override;

				void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
															 bool shouldDrawButtonAsHighlighted,
															 bool shouldDrawButtonAsDown) override;

				void drawButtonBackground (juce::Graphics&, juce::Button&,
																	 const juce::Colour& backgroundColour,
																	 bool shouldDrawButtonAsHighlighted,
																	 bool shouldDrawButtonAsDown) override;

				void drawButtonText (juce::Graphics&, juce::TextButton&,
														 bool shouldDrawButtonAsHighlighted,
														 bool shouldDrawButtonAsDown) override;

				void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
													 int buttonX, int buttonY, int buttonW, int buttonH,
													 juce::ComboBox&) override;

				void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

				void drawLabel (juce::Graphics&, juce::Label&) override;

				juce::Label* createSliderTextBox (juce::Slider&) override;

				juce::Font getLabelFont (juce::Label&) override;
				juce::Font getComboBoxFont (juce::ComboBox&) override;
				juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
		};
}
