#include "PhaseScarLookAndFeel.h"

namespace PhaseScar
{
		const juce::Colour PhaseScarLookAndFeel::background     { 0xff0b0b0e };
		const juce::Colour PhaseScarLookAndFeel::panelFill      { 0xff17171c };
		const juce::Colour PhaseScarLookAndFeel::panelFillDark  { 0xff101014 };
		const juce::Colour PhaseScarLookAndFeel::outline        { 0xff2c2c34 };
		const juce::Colour PhaseScarLookAndFeel::innerOutline   { 0x14ffffff };
		const juce::Colour PhaseScarLookAndFeel::textMain       { 0xffd6d9e0 };
		const juce::Colour PhaseScarLookAndFeel::textDim        { 0xff8b8f9a };
		const juce::Colour PhaseScarLookAndFeel::accentPurple   { 0xffb23cff };
		const juce::Colour PhaseScarLookAndFeel::accentRed      { 0xffe0452a };

		const juce::Identifier PhaseScarLookAndFeel::accentPropertyId { "phaseScarAccent" };

		//==============================================================================
		PhaseScarLookAndFeel::PhaseScarLookAndFeel()
		{
				setColour (juce::ResizableWindow::backgroundColourId, background);
				setColour (juce::Label::textColourId,                 textMain);
				setColour (juce::Slider::textBoxTextColourId,         textMain);
				setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
				setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
				setColour (juce::Slider::textBoxHighlightColourId,    accentPurple.withAlpha (0.35f));
				setColour (juce::ComboBox::backgroundColourId,        panelFillDark);
				setColour (juce::ComboBox::textColourId,              textMain);
				setColour (juce::ComboBox::outlineColourId,           outline);
				setColour (juce::ComboBox::arrowColourId,             textDim);
				setColour (juce::PopupMenu::backgroundColourId,       panelFillDark);
				setColour (juce::PopupMenu::textColourId,             textMain);
				setColour (juce::PopupMenu::highlightedBackgroundColourId, accentPurple.withAlpha (0.35f));
				setColour (juce::PopupMenu::highlightedTextColourId,  juce::Colours::white);
				setColour (juce::TextButton::buttonColourId,          panelFill);
				setColour (juce::TextButton::textColourOffId,         textMain);
				setColour (juce::TextButton::textColourOnId,          juce::Colours::white);
				setColour (juce::TooltipWindow::backgroundColourId,   panelFillDark);
				setColour (juce::TooltipWindow::textColourId,         textMain);
				setColour (juce::ScrollBar::thumbColourId,            outline.brighter (0.2f));
				setColour (juce::CaretComponent::caretColourId,       accentPurple);
		}

		void PhaseScarLookAndFeel::setRedAccent (juce::Component& component, bool shouldUseRed)
		{
				component.getProperties().set (accentPropertyId, shouldUseRed ? "red" : "purple");
		}

		juce::Colour PhaseScarLookAndFeel::accentFor (const juce::Component* component)
		{
				for (auto* c = component; c != nullptr; c = c->getParentComponent())
						if (auto* value = c->getProperties().getVarPointer (accentPropertyId))
								return value->toString() == "red" ? accentRed : accentPurple;

				return accentPurple;
		}

		//==============================================================================
		void PhaseScarLookAndFeel::drawPanel (juce::Graphics& g, juce::Rectangle<int> boundsInt,
																					const juce::String& title, juce::Colour accent)
		{
				auto bounds = boundsInt.toFloat().reduced (0.5f);
				constexpr float corner = 4.0f;

				g.setGradientFill (juce::ColourGradient (panelFill, bounds.getCentreX(), bounds.getY(),
																								 panelFillDark, bounds.getCentreX(), bounds.getBottom(), false));
				g.fillRoundedRectangle (bounds, corner);

				g.setColour (outline);
				g.drawRoundedRectangle (bounds, corner, 1.0f);

				g.setColour (innerOutline);
				g.drawRoundedRectangle (bounds.reduced (1.5f), corner - 1.0f, 1.0f);

				if (title.isNotEmpty())
				{
						auto header = bounds.removeFromTop (22.0f).reduced (10.0f, 4.0f);

						g.setColour (accent);
						g.fillRect (juce::Rectangle<float> (header.getX() - 5.0f, header.getCentreY() - 5.0f, 2.0f, 10.0f));

						g.setColour (textMain);
						g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
						g.drawText (title.toUpperCase(), header.toNearestInt(), juce::Justification::centredLeft, false);

						g.setColour (outline);
						g.drawLine (bounds.getX() + 6.0f, header.getBottom() + 3.0f,
												bounds.getRight() - 6.0f, header.getBottom() + 3.0f, 1.0f);
				}
		}

		//==============================================================================
		void PhaseScarLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
																								 float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
																								 juce::Slider& slider)
		{
				const auto accent = accentFor (&slider);
				auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2.0f);
				const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
				const auto centre = bounds.getCentre();
				const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

				const auto arcRadius = radius - 2.0f;
				const auto bodyRadius = radius * 0.72f;

				// Track
				juce::Path track;
				track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
														 rotaryStartAngle, rotaryEndAngle, true);
				g.setColour (outline);
				g.strokePath (track, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

				// Value arc
				if (sliderPos > 0.001f)
				{
						juce::Path value;
						value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
																 rotaryStartAngle, angle, true);
						g.setColour (accent.withAlpha (slider.isEnabled() ? 1.0f : 0.4f));
						g.strokePath (value, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
				}

				// Metallic outer ring
				g.setGradientFill (juce::ColourGradient (juce::Colour (0xff45454f), centre.x, centre.y - bodyRadius,
																								 juce::Colour (0xff1b1b21), centre.x, centre.y + bodyRadius, false));
				g.fillEllipse (juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f).withCentre (centre));

				// Body with inner shadow
				const auto innerRadius = bodyRadius - 2.0f;
				g.setGradientFill (juce::ColourGradient (juce::Colour (0xff101014), centre.x, centre.y - innerRadius,
																								 juce::Colour (0xff222229), centre.x, centre.y + innerRadius, false));
				g.fillEllipse (juce::Rectangle<float> (innerRadius * 2.0f, innerRadius * 2.0f).withCentre (centre));

				g.setColour (juce::Colours::black.withAlpha (0.45f));
				g.drawEllipse (juce::Rectangle<float> (innerRadius * 2.0f, innerRadius * 2.0f).withCentre (centre), 1.4f);

				// Pointer
				juce::Path pointer;
				const auto pointerLength = innerRadius * 0.62f;
				pointer.addRoundedRectangle (-1.0f, -innerRadius + 2.5f, 2.0f, pointerLength, 1.0f);
				pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));

				g.setColour (slider.isEnabled() ? textMain : textDim);
				g.fillPath (pointer);

				g.setColour (accent.withAlpha (0.55f));
				g.fillEllipse (juce::Rectangle<float> (3.0f, 3.0f).withCentre (centre));
		}

		//==============================================================================
		void PhaseScarLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
																								 bool shouldDrawButtonAsHighlighted, bool)
		{
				const auto accent = accentFor (&button);
				const auto on = button.getToggleState();

				auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
				const auto boxSize = juce::jmin (14.0f, bounds.getHeight());
				auto box = juce::Rectangle<float> (boxSize, boxSize).withCentre ({ bounds.getX() + boxSize * 0.5f, bounds.getCentreY() });

				g.setColour (on ? accent.withAlpha (0.85f) : juce::Colour (0xff1c1c22));
				g.fillRoundedRectangle (box, 2.5f);

				g.setColour (on ? accent : outline);
				g.drawRoundedRectangle (box, 2.5f, 1.0f);

				if (on)
				{
						g.setColour (juce::Colours::white.withAlpha (0.85f));
						g.fillRoundedRectangle (box.reduced (boxSize * 0.32f), 1.0f);
				}

				if (shouldDrawButtonAsHighlighted)
				{
						g.setColour (accent.withAlpha (0.25f));
						g.drawRoundedRectangle (box.expanded (1.5f), 3.5f, 1.0f);
				}

				const auto text = button.getButtonText();

				if (text.isNotEmpty())
				{
						g.setColour (on ? textMain : textDim);
						g.setFont (juce::Font (juce::FontOptions (11.0f)));
						g.drawText (text, bounds.withTrimmedLeft (boxSize + 6.0f).toNearestInt(),
												juce::Justification::centredLeft, true);
				}
		}

		void PhaseScarLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
																										 const juce::Colour&,
																										 bool shouldDrawButtonAsHighlighted,
																										 bool shouldDrawButtonAsDown)
		{
				const auto accent = accentFor (&button);
				auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);

				g.setGradientFill (juce::ColourGradient (juce::Colour (0xff23232a), bounds.getCentreX(), bounds.getY(),
																								 juce::Colour (0xff141418), bounds.getCentreX(), bounds.getBottom(), false));
				g.fillRoundedRectangle (bounds, 3.0f);

				if (shouldDrawButtonAsDown || button.getToggleState())
				{
						g.setColour (accent.withAlpha (0.28f));
						g.fillRoundedRectangle (bounds, 3.0f);
				}

				g.setColour (shouldDrawButtonAsHighlighted ? accent.withAlpha (0.7f) : outline);
				g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
		}

		void PhaseScarLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool)
		{
				g.setFont (getTextButtonFont (button, button.getHeight()));
				g.setColour (button.isEnabled() ? textMain : textDim);
				g.drawText (button.getButtonText().toUpperCase(), button.getLocalBounds(),
										juce::Justification::centred, false);
		}

		//==============================================================================
		void PhaseScarLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
																						 int, int, int, int, juce::ComboBox& box)
		{
				const auto accent = accentFor (&box);
				auto bounds = juce::Rectangle<int> (width, height).toFloat().reduced (0.5f);

				g.setColour (panelFillDark);
				g.fillRoundedRectangle (bounds, 3.0f);

				const auto focused = isButtonDown || box.hasKeyboardFocus (false);
				g.setColour (focused ? accent.withAlpha (0.8f) : outline);
				g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

				juce::Path arrow;
				const auto cx = bounds.getRight() - 12.0f;
				const auto cy = bounds.getCentreY();
				arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
				arrow.lineTo (cx, cy + 3.0f);
				arrow.lineTo (cx + 4.0f, cy - 2.0f);

				g.setColour (box.isEnabled() ? accent : textDim);
				g.strokePath (arrow, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
		}

		void PhaseScarLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
		{
				label.setBounds (8, 1, box.getWidth() - 26, box.getHeight() - 2);
				label.setFont (getComboBoxFont (box));
				label.setJustificationType (juce::Justification::centredLeft);
		}

		//==============================================================================
		void PhaseScarLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
		{
				if (! label.isBeingEdited())
				{
						g.setFont (getLabelFont (label));
						g.setColour (label.findColour (juce::Label::textColourId)
															.withMultipliedAlpha (label.isEnabled() ? 1.0f : 0.5f));
						g.drawFittedText (label.getText(), label.getLocalBounds(),
															label.getJustificationType(), 1, 1.0f);
				}
				else if (auto* editor = label.getCurrentTextEditor())
				{
						juce::ignoreUnused (editor);
				}
		}

		juce::Label* PhaseScarLookAndFeel::createSliderTextBox (juce::Slider& slider)
		{
				auto* label = juce::LookAndFeel_V4::createSliderTextBox (slider);
				label->setColour (juce::Label::textColourId, textDim);
				label->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
				label->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
				label->setColour (juce::TextEditor::backgroundColourId, panelFillDark);
				label->setColour (juce::TextEditor::textColourId, textMain);
				label->setColour (juce::TextEditor::highlightColourId, accentFor (&slider).withAlpha (0.35f));
				label->setFont (juce::Font (juce::FontOptions (11.0f)));
				return label;
		}

		juce::Font PhaseScarLookAndFeel::getLabelFont (juce::Label& label)
		{
				return juce::Font (juce::FontOptions (label.getFont().getHeight()));
		}

		juce::Font PhaseScarLookAndFeel::getComboBoxFont (juce::ComboBox&)
		{
				return juce::Font (juce::FontOptions (12.0f));
		}

		juce::Font PhaseScarLookAndFeel::getTextButtonFont (juce::TextButton&, int)
		{
				return juce::Font (juce::FontOptions (11.0f, juce::Font::bold));
		}
}
