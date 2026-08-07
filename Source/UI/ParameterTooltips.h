#pragma once

#include <JuceHeader.h>

namespace PhaseScar
{
		/**
				Central place for all knob/control tooltip texts.

				Edit the strings below to change what appears in the small hover
				tooltip (shown after ~1 second of hovering over a control). Keys must
				match the APVTS parameter IDs defined in DSP/PhaseScarParameters.h.
		*/
		inline const std::map<juce::String, juce::String>& getParameterTooltips()
		{
				static const std::map<juce::String, juce::String> tooltips
				{
						// Global
						{ "inputGain",          "Nastavuje vstupni uroven signalu pred zpracovanim." },
						{ "outputGain",         "Nastavuje vystupni uroven signalu po zpracovani." },
						{ "dryWet",             "Pomer mezi puvodnim (dry) a zpracovanym (wet) signalem." },
						{ "globalBypass",       "Obejde cely efekt a propusti puvodni signal beze zmeny." },

						// Pre EQ
						{ "preEqEnabled",       "Zapne/vypne predni ekvalizer pred flangerem a notchem." },
						{ "preHighPass",        "Horni propust - odrizne nizke frekvence pod nastavenou hranici." },
						{ "preLowPass",         "Dolni propust - odrizne vysoke frekvence nad nastavenou hranici." },
						{ "preTilt",            "Naklani tonalni vyvazeni mezi basy a vyskami (tilt EQ)." },

						// Flanger
						{ "flangerEnabled",     "Zapne/vypne flanger efekt." },
						{ "flangerRate",        "Rychlost modulace flangeru (jak rychle se meni zpozdeni)." },
						{ "flangerDepth",       "Hloubka modulace flangeru - jak silne se zpozdeni meni." },
						{ "flangerDelay",       "Zakladni doba zpozdeni flangeru." },
						{ "flangerFeedback",    "Mnozstvi signalu vraceneho zpet do flangeru (zvyrazni rezonanci)." },
						{ "flangerMix",         "Pomer mezi suchym a flangerem zpracovanym signalem." },
						{ "flangerStereoPhase", "Fazovy posun modulace mezi levym a pravym kanalem (sirka stereo obrazu)." },

						// Notch
						{ "notchEnabled",       "Zapne/vypne dvojici pohyblivych notch filtru." },
						{ "notchCenter",        "Stredni frekvence, kolem ktere jsou notch filtry umisteny." },
						{ "notchSpread",        "Rozestup (v oktavach) mezi dvema notch filtry kolem stredu." },
						{ "notchQ",             "Sirka/ostrost notch filtru - vyssi hodnota = uzsi a hlubsi zarez." },
						{ "notchDepth",         "Hloubka potlaceni frekvence v notch filtrech (0-100 %)." },
						{ "notchMotion",        "Rychlost/mnozstvi automatickeho pohybu notch filtru v case." },

						// Distortion
						{ "distortionEnabled",  "Zapne/vypne stupen zkresleni (distortion)." },
						{ "distortionDrive",    "Mnozstvi zesileni signalu pred zkreslenim (intenzita zkresleni)." },
						{ "distortionBias",     "Posouva pracovni bod zkresleni - meni charakter/asymetrii zvuku." },
						{ "distortionTone",     "Tonalni korekce zkresleneho signalu (jas/tmavost)." },
						{ "distortionTrim",     "Dorovnani vystupni urovne po zkresleni." },
						{ "distortionMix",      "Pomer mezi suchym a zkreslenym signalem." },
						{ "distortionType",     "Vyber typu/algoritmu zkresleni." },

						// Post EQ
						{ "postEqEnabled",      "Zapne/vypne vystupni ekvalizer." },
						{ "postLowGain",        "Zesileni/zeslabeni nizkych frekvenci na vystupu." },
						{ "postMidFrequency",   "Stredni frekvence pasmoveho (mid) filtru na vystupu." },
						{ "postMidGain",        "Zesileni/zeslabeni stredni frekvence na vystupu." },
						{ "postMidQ",           "Sirka pasmoveho (mid) filtru na vystupu." },
						{ "postHighGain",       "Zesileni/zeslabeni vysokych frekvenci na vystupu." },
				};

				return tooltips;
		}

		/** Returns the tooltip text for a given parameter ID, or an empty string if none is set. */
		inline juce::String getParameterTooltip (const juce::String& paramID)
		{
				const auto& tooltips = getParameterTooltips();
				const auto it = tooltips.find (paramID);
				return it != tooltips.end() ? it->second : juce::String();
		}
}
