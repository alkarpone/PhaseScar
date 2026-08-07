/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   phase_scar_logo_png;
    const int            phase_scar_logo_pngSize = 2151353;

    extern const char*   phase_scar_center_panel_png;
    const int            phase_scar_center_panel_pngSize = 2242971;

    extern const char*   phase_scar_background_png;
    const int            phase_scar_background_pngSize = 1991696;

    extern const char*   phase_scar_symbol_png;
    const int            phase_scar_symbol_pngSize = 2452390;

    extern const char*   phase_scar_icons_png;
    const int            phase_scar_icons_pngSize = 2213254;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
