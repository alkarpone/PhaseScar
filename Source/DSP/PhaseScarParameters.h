#pragma once

#include <JuceHeader.h>

/**
				Centralised parameter identifiers and layout construction for Phase Scar.

				Parameter IDs must stay stable forever - presets and host automation depend
				on them. Visible names may change freely, IDs may not.
*/
namespace PhaseScar
{
				namespace ParamID
				{
								// Global
								inline constexpr auto inputGain          = "inputGain";
								inline constexpr auto outputGain         = "outputGain";
								inline constexpr auto dryWet             = "dryWet";
								inline constexpr auto globalBypass       = "globalBypass";

								// Pre EQ
								inline constexpr auto preEqEnabled       = "preEqEnabled";
								inline constexpr auto preHighPass        = "preHighPass";
								inline constexpr auto preLowPass         = "preLowPass";
								inline constexpr auto preTilt            = "preTilt";

								// Flanger
								inline constexpr auto flangerEnabled     = "flangerEnabled";
								inline constexpr auto flangerRate        = "flangerRate";
								inline constexpr auto flangerDepth       = "flangerDepth";
								inline constexpr auto flangerDelay       = "flangerDelay";
								inline constexpr auto flangerFeedback    = "flangerFeedback";
								inline constexpr auto flangerMix         = "flangerMix";
								inline constexpr auto flangerStereoPhase = "flangerStereoPhase";

								// Notch bank
								inline constexpr auto notchEnabled       = "notchEnabled";
								inline constexpr auto notchCenter        = "notchCenter";
								inline constexpr auto notchSpread        = "notchSpread";
								inline constexpr auto notchQ             = "notchQ";
								inline constexpr auto notchDepth         = "notchDepth";
								inline constexpr auto notchMotion        = "notchMotion";

								// Distortion
								inline constexpr auto distortionEnabled  = "distortionEnabled";
								inline constexpr auto distortionDrive    = "distortionDrive";
								inline constexpr auto distortionType     = "distortionType";
								inline constexpr auto distortionBias     = "distortionBias";
								inline constexpr auto distortionTone     = "distortionTone";
								inline constexpr auto distortionTrim     = "distortionTrim";
								inline constexpr auto distortionMix      = "distortionMix";

								// Post EQ
								inline constexpr auto postEqEnabled      = "postEqEnabled";
								inline constexpr auto postLowGain        = "postLowGain";
								inline constexpr auto postMidFrequency   = "postMidFrequency";
								inline constexpr auto postMidGain        = "postMidGain";
								inline constexpr auto postMidQ           = "postMidQ";
								inline constexpr auto postHighGain       = "postHighGain";
				}

				/** Distortion algorithms available in the distortion stage. */
				enum class DistortionMode
				{
								softClip = 0,
								hardClip,
								tanh,
								wavefold,
								rectify,
								numModes
				};

				inline juce::StringArray getDistortionModeNames()
				{
								return { "Soft Clip", "Hard Clip", "Tanh", "Wavefold", "Rectify" };
				}

				/** Builds the complete APVTS parameter layout. */
				juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
}
