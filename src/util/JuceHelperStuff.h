// JuceHelperStuff.h - Some useful helper functions.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
//
// Modified for Pedalboard3 from the original Pedalboard2 source.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef JUCEHELPERSTUFF_H_
#define JUCEHELPERSTUFF_H_

#include <JuceHeader.h>

/// Collection of helper functions for common JUCE tasks.
namespace JuceHelperStuff {

/// Loads an SVG image from memory into a Drawable.
///
/// @param dataToInitialiseFrom Pointer to the raw SVG data.
/// @param sizeInBytes Length of the SVG data in bytes.
/// @return A heap-allocated Drawable, or nullptr if the data could not be parsed as valid SVG.
Drawable* loadSVGFromMemory(const void* dataToInitialiseFrom, size_t sizeInBytes);

/// Shows a modal dialog window with the application icon set.
///
/// The dialog is centred around componentToCentreAround and uses a native
/// title bar.
///
/// @param dialogTitle Title of the dialog window.
/// @param contentComponent Content component to display in the dialog.
/// @param componentToCentreAround Component to centre the dialog around.
/// @param backgroundColour Background colour of the dialog.
/// @param escapeKeyTriggersCloseButton Whether the escape key closes the dialog.
/// @param shouldBeResizable Whether the dialog should be resizable.
/// @param useBottomRightCornerResizer Whether to show a bottom-right resizer.
/// @return The modal loop's exit code.
int showModalDialog(const String& dialogTitle, Component* contentComponent, Component* componentToCentreAround,
                    const Colour& backgroundColour, bool escapeKeyTriggersCloseButton, bool shouldBeResizable = false,
                    bool useBottomRightCornerResizer = false);

/// Shows a non-modal dialog window with the application icon set.
///
/// Unlike showModalDialog this does not block.
///
/// @param dialogTitle Title of the dialog window.
/// @param contentComponent Content component to display in the dialog.
/// @param componentToCentreAround Component to centre the dialog around.
/// @param backgroundColour Background colour of the dialog.
/// @param escapeKeyTriggersCloseButton Whether the escape key closes the dialog.
/// @param shouldBeResizable Whether the dialog should be resizable.
/// @param useBottomRightCornerResizer Whether to show a bottom-right resizer.
/// @param stayOnTop When true the dialog is kept above other windows.
void showNonModalDialog(const String& dialogTitle, Component* contentComponent, Component* componentToCentreAround,
                        const Colour& backgroundColour, bool escapeKeyTriggersCloseButton,
                        bool shouldBeResizable = false, bool useBottomRightCornerResizer = false,
                        bool stayOnTop = false);

/// Returns the application data folder for Pedalboard3.
///
/// @return The application data folder File.
File getAppDataFolder();

} // namespace JuceHelperStuff

#endif
