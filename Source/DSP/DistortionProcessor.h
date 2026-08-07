#pragma once

#include "BassMutatorParameters.h"

namespace BassMutator
{
		/**
				Stateless waveshaping helpers shared by the BODY and TEXTURE bands.

				All functions expect a pre-driven sample and return a roughly level
				matched result, so switching modes does not cause huge jumps.
				Exact loudness matching is intentionally out of scope for v1.
		*/
		struct Distortion
		{
				static float softClip (float x) noexcept
				{
						// Cubic soft clipper: smooth knee, hard limit at +/-1.
						x = juce::jlimit (-1.5f, 1.5f, x);
						return x - (x * x * x) / 3.0f;
				}

				static float hardClip (float x) noexcept
				{
						return juce::jlimit (-1.0f, 1.0f, x);
				}

				static float tanhShape (float x) noexcept
				{
						return std::tanh (x);
				}

				static float wavefold (float x) noexcept
				{
						// Triangle folding: maps any input into [-1, 1] without blowing up.
						constexpr float period = 4.0f;
						float folded = std::fmod (x + 1.0f, period);

						if (folded < 0.0f)
								folded += period;

						folded = std::fabs (folded - 2.0f) - 1.0f;
						return -folded;
				}

				static float rectify (float x) noexcept
				{
						// Blend of the raw signal and its full-wave rectified version, with the
						// DC term removed so the fundamental is not destroyed.
						const auto rectified = std::fabs (x) - 0.5f;
						return juce::jlimit (-1.0f, 1.0f, 0.5f * x + rectified);
				}

				static float process (DistortionMode mode, float x) noexcept
				{
						switch (mode)
						{
								case DistortionMode::softClip: return softClip (x);
								case DistortionMode::hardClip: return hardClip (x);
								case DistortionMode::tanh:     return tanhShape (x);
								case DistortionMode::wavefold: return wavefold (x);
								case DistortionMode::rectify:  return rectify (x);
								case DistortionMode::numModes:
								default:                       return x;
						}
				}

				/** Approximate make-up so higher drive settings do not run away in level. */
				static float gainCompensation (DistortionMode mode, float driveLinear) noexcept
				{
						const auto base = 1.0f / juce::jmax (1.0f, std::sqrt (driveLinear));

						switch (mode)
						{
								case DistortionMode::softClip: return base * 1.5f;
								case DistortionMode::hardClip: return base * 0.9f;
								case DistortionMode::tanh:     return base * 1.1f;
								case DistortionMode::wavefold: return base * 0.8f;
								case DistortionMode::rectify:  return base * 1.0f;
								case DistortionMode::numModes:
								default:                       return 1.0f;
						}
				}
		};
}
