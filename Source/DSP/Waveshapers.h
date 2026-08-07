#pragma once

#include "PhaseScarParameters.h"

namespace PhaseScar
{
				/**
								Stateless waveshaping helpers used by the distortion stage.

								All functions expect a pre-driven sample and return a roughly level
								matched result, so switching modes does not cause huge jumps.
				*/
				struct Waveshapers
				{
								static float softClip (float x) noexcept
								{
												// Cubic soft clipper. The input must be limited to +/-1 before the
												// polynomial, otherwise the curve folds back down and louder input
												// produces a quieter output.
												x = juce::jlimit (-1.0f, 1.0f, x);
												return 1.5f * (x - (x * x * x) / 3.0f);
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
												constexpr float period = 4.0f;
												float folded = std::fmod (x + 1.0f, period);

												if (folded < 0.0f)
																folded += period;

												folded = std::fabs (folded - 2.0f) - 1.0f;
												return -folded;
								}

								static float rectify (float x) noexcept
								{
												const auto rectified = std::fabs (x) - 0.5f;
												return juce::jlimit (-1.0f, 1.0f, 0.5f * x + rectified);
								}

								static float process (DistortionMode mode, float x) noexcept
								{
												if (! std::isfinite (x))
																return 0.0f;

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

								/**
												Mild make-up so higher drive settings stay in the same loudness
												ballpark. The shapers already limit the peak level, so only a gentle
												correction is applied - a strong 1/sqrt(drive) law would simply make
												the sound quieter the harder it is driven.
								*/
								static float gainCompensation (DistortionMode mode, float driveLinear) noexcept
								{
												const auto safeDrive = juce::jmax (1.0f, driveLinear);
												const auto base = 1.0f / std::pow (safeDrive, 0.25f);

												switch (mode)
												{
																case DistortionMode::softClip: return base * 1.6f;
																case DistortionMode::hardClip: return base * 1.2f;
																case DistortionMode::tanh:     return base * 1.4f;
																case DistortionMode::wavefold: return base * 1.1f;
																case DistortionMode::rectify:  return base * 1.3f;
																case DistortionMode::numModes:
																default:                       return 1.0f;
												}
								}
				};
}
