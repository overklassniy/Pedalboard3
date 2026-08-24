// PresetBar.h - Preset bar component for individual plugins.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2012 Niall Moody.
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

#ifndef PRESETBAR_H_
#define PRESETBAR_H_

#include <JuceHeader.h>

class PluginComponent;

/// Preset bar component for individual plugins.
///
/// Displays a combo box with plugin presets and user-saved presets,
/// along with import and save buttons.
class PresetBar : public Component, public juce::Button::Listener, public juce::ComboBox::Listener {
  public:
    /// Creates the preset bar for the given plugin component.
    ///
    /// @param comp The parent PluginComponent this bar belongs to.
    PresetBar(PluginComponent* comp);
    /// Destructor.
    ~PresetBar() override;

    /// Handles import and save button clicks.
    ///
    /// @param button The button that was clicked.
    void buttonClicked(Button* button) override;

    /// Paints the bar background.
    ///
    /// @param g The graphics context to draw with.
    void paint(Graphics& g) override;
    /// Lays out the combo box, label, and buttons.
    void resized() override;
    /// Handles preset selection, user preset loading, and preset renaming.
    ///
    /// @param comboBoxThatHasChanged The combo box whose selection changed.
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

  private:
    /// Populates the combo box with plugin presets followed by user-saved presets.
    void fillOutComboBox();

    /// Loads an SVG drawable from a binary data chunk.
    ///
    /// @param dataToInitialiseFrom Pointer to the raw SVG data in memory.
    /// @param sizeInBytes The size of the data in bytes.
    ///
    /// @return The loaded Drawable, or nullptr if parsing failed.
    Drawable* loadSVGFromMemory(const void* dataToInitialiseFrom, size_t sizeInBytes);

    /// The parent PluginComponent this bar belongs to.
    PluginComponent* component;

    /// Tracks the last selected combo box ID to detect preset name edits.
    int lastComboBox;

    std::unique_ptr<ComboBox> presetsComboBox;
    std::unique_ptr<Label> presetsLabel;
    std::unique_ptr<DrawableButton> importButton;
    std::unique_ptr<DrawableButton> saveButton;
    Path internalPath1;

    JUCE_LEAK_DETECTOR(PresetBar)
};

#endif
