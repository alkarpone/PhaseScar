#pragma once

#include <JuceHeader.h>

/**
		Centralised parameter identifiers and layout construction for Bass Mutator.

		Parameter IDs must stay stable forever - presets and host automation depend
		on them. Visible names may change freely, IDs may not.
*/
namespace BassMutator
{
		namespace ParamID
		{
				// Global
				inline constexpr auto inputGain             = "inputGain";
				inline constexpr auto outputGain            = "outputGain";
				inline constexpr auto dryWet                = "dryWet";
				inline constexpr auto globalBypass          = "globalBypass";

				// Crossover
				inline constexpr auto subCrossover          = "subCrossover";
				inline constexpr auto textureCrossover      = "textureCrossover";

				// Sub band
				inline constexpr auto subGain               = "subGain";
				inline constexpr auto subSaturation         = "subSaturation";
				inline constexpr auto subMono               = "subMono";

				// Body band
				inline constexpr auto bodyDrive             = "bodyDrive";
				inline constexpr auto bodyTone              = "bodyTone";
				inline constexpr auto bodyGain              = "bodyGain";
				inline constexpr auto bodyDistortionMode    = "bodyDistortionMode";
				inline constexpr auto bodyMotionAmount      = "bodyMotionAmount";

				// Texture band
				inline constexpr auto textureDrive          = "textureDrive";
				inline constexpr auto textureCrush          = "textureCrush";
				inline constexpr auto textureWidth          = "textureWidth";
				inline constexpr auto textureTone           = "textureTone";
				inline constexpr auto textureGain           = "textureGain";
				inline constexpr auto textureDistortionMode = "textureDistortionMode";

				// Motion
				inline constexpr auto motionRate            = "motionRate";
				inline constexpr auto motionDepth           = "motionDepth";
				inline constexpr auto motionShape           = "motionShape";
				inline constexpr auto motionSync            = "motionSync";
				inline constexpr auto motionDivision        = "motionDivision";

				// Envelope follower
				inline constexpr auto envelopeAmount        = "envelopeAmount";
				inline constexpr auto envelopeAttack        = "envelopeAttack";
				inline constexpr auto envelopeRelease       = "envelopeRelease";
		}

		/** Distortion algorithms shared by the BODY and TEXTURE bands. */
		enum class DistortionMode
		{
				softClip = 0,
				hardClip,
				tanh,
				wavefold,
				rectify,
				numModes
		};

		/** LFO waveform shapes. */
		enum class MotionShape
		{
				sine = 0,
				triangle,
				saw,
				square,
				sampleAndHold,
				numShapes
		};

		/** Tempo-synced note divisions, expressed later as a multiple of a whole note. */
		enum class MotionDivision
		{
				whole = 0,
				halfDotted,
				half,
				halfTriplet,
				quarterDotted,
				quarter,
				quarterTriplet,
				eighthDotted,
				eighth,
				eighthTriplet,
				sixteenthDotted,
				sixteenth,
				sixteenthTriplet,
				numDivisions
		};

		inline juce::StringArray getDistortionModeNames()
		{
				return { "Soft Clip", "Hard Clip", "Tanh", "Wavefold", "Rectify" };
		}

		inline juce::StringArray getMotionShapeNames()
		{
				return { "Sine", "Triangle", "Saw", "Square", "Sample & Hold" };
		}

		inline juce::StringArray getMotionDivisionNames()
		{
				return { "1/1", "1/2.", "1/2", "1/2T",
								 "1/4.", "1/4", "1/4T",
								 "1/8.", "1/8", "1/8T",
								 "1/16.", "1/16", "1/16T" };
		}

		/** Length of one LFO cycle in beats (quarter notes) for the given division. */
		inline double getDivisionInBeats (MotionDivision division) noexcept
		{
				switch (division)
				{
						case MotionDivision::whole:            return 4.0;
						case MotionDivision::halfDotted:       return 3.0;
						case MotionDivision::half:             return 2.0;
						case MotionDivision::halfTriplet:      return 4.0 / 3.0;
						case MotionDivision::quarterDotted:    return 1.5;
						case MotionDivision::quarter:          return 1.0;
						case MotionDivision::quarterTriplet:   return 2.0 / 3.0;
						case MotionDivision::eighthDotted:     return 0.75;
						case MotionDivision::eighth:           return 0.5;
						case MotionDivision::eighthTriplet:    return 1.0 / 3.0;
						case MotionDivision::sixteenthDotted:  return 0.375;
						case MotionDivision::sixteenth:        return 0.25;
						case MotionDivision::sixteenthTriplet: return 1.0 / 6.0;
						case MotionDivision::numDivisions:
						default:                               return 1.0;
				}
		}

		/** Builds the complete APVTS parameter layout. */
		juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
}
