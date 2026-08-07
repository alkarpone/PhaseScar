#include "PhaseScarAssets.h"

namespace PhaseScar
{
		namespace
		{
				juce::StringArray missingAssets;

				void reportMissing (const juce::String& name)
				{
						if (! missingAssets.contains (name))
								missingAssets.add (name);
				}
		}

		juce::Image Assets::loadByName (const char* originalFileName)
		{
				// Resolve through the generated originalFilenames table so we never have to
				// hard-code the mangled BinaryData identifiers.
				for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
				{
						if (juce::String (BinaryData::originalFilenames[i]).equalsIgnoreCase (originalFileName))
						{
								int size = 0;

								if (auto* data = BinaryData::getNamedResource (BinaryData::namedResourceList[i], size))
								{
										auto image = juce::ImageFileFormat::loadFrom (data, (size_t) size);

										if (image.isValid())
												return image;
								}

								break;
						}
				}

				reportMissing (originalFileName);
				return {};
		}

		const juce::Image& Assets::logo()
		{
				static const juce::Image image = loadByName ("phase_scar_logo.png");
				return image;
		}

		const juce::Image& Assets::centrePanel()
		{
				static const juce::Image image = loadByName ("phase_scar_center_panel.png");
				return image;
		}

		const juce::Image& Assets::background()
		{
				static const juce::Image image = loadByName ("phase_scar_background.png");
				return image;
		}

		const juce::Image& Assets::symbol()
		{
				static const juce::Image image = loadByName ("phase_scar_symbol.png");
				return image;
		}

		const juce::Image& Assets::icons()
		{
				static const juce::Image image = loadByName ("phase_scar_icons.png");
				return image;
		}

		juce::StringArray Assets::getMissingAssetNames()
		{
				return missingAssets;
		}

		juce::Rectangle<int> Assets::detectContentArea (const juce::Image& image)
		{
				if (! image.isValid())
						return {};

				const juce::Image::BitmapData data (image, juce::Image::BitmapData::readOnly);
				const auto hasAlpha = image.hasAlphaChannel();

				// Reference "empty" colour taken from the top left corner for opaque images.
				const auto reference = data.getPixelColour (0, 0);

				const auto isContent = [&] (int x, int y)
				{
						const auto pixel = data.getPixelColour (x, y);

						if (hasAlpha && pixel.getFloatAlpha() < 0.08f)
								return false;

						if (! hasAlpha)
						{
								const auto difference = std::abs (pixel.getFloatRed()   - reference.getFloatRed())
																			+ std::abs (pixel.getFloatGreen() - reference.getFloatGreen())
																			+ std::abs (pixel.getFloatBlue()  - reference.getFloatBlue());
								return difference > 0.12f;
						}

						return true;
				};

				// A coarse step is plenty for a bounding box and keeps this cheap.
				constexpr int step = 2;

				int minX = image.getWidth(), minY = image.getHeight(), maxX = -1, maxY = -1;

				for (int y = 0; y < image.getHeight(); y += step)
				{
						for (int x = 0; x < image.getWidth(); x += step)
						{
								if (isContent (x, y))
								{
										minX = juce::jmin (minX, x);
										minY = juce::jmin (minY, y);
										maxX = juce::jmax (maxX, x);
										maxY = juce::jmax (maxY, y);
								}
						}
				}

				if (maxX < minX || maxY < minY)
						return image.getBounds();

				return juce::Rectangle<int> (minX, minY, maxX - minX + 1, maxY - minY + 1)
								 .expanded (step)
								 .getIntersection (image.getBounds());
		}

		juce::Rectangle<int> Assets::logoContentArea()
		{
				static const juce::Rectangle<int> area = detectContentArea (logo());
				return area;
		}
}
