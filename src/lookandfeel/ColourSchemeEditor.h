// ColourSchemeEditor.h - Editor for the colour scheme.
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

#ifndef COLOURSCHEMEEDITOR_H_
#define COLOURSCHEMEEDITOR_H_

#include <JuceHeader.h>

/// Editor for the colour scheme.
///
/// Displays a list of named colours with a colour selector, and a combo
/// box for choosing, saving, and deleting colour scheme presets.
/// Notifies listeners via ChangeBroadcaster when a colour changes.
class ColourSchemeEditor : public Component,
                           public ListBoxModel,
                           public juce::Button::Listener,
                           public ChangeListener,
                           public ChangeBroadcaster,
                           public juce::ComboBox::Listener {
  public:
    /// Builds the editor with a colour selector, colour list, and preset controls.
    ColourSchemeEditor();
    /// Prompts to save unsaved changes and persists the selected preset name.
    ~ColourSchemeEditor() override;

    /// Returns the number of colours which can be edited.
    ///
    /// @return The number of colour entries in the colour scheme.
    int getNumRows() override;
    /// Draws the colour selector list.
    ///
    /// @param rowNumber The index of the row to draw.
    /// @param g The graphics context to draw with.
    /// @param width The width of the row.
    /// @param height The height of the row.
    /// @param rowIsSelected Whether the row is currently selected.
    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
    /// So we know when the user selects a new colour.
    ///
    /// @param row The index of the clicked row.
    /// @param e The mouse event for the click.
    void listBoxItemClicked(int row, const MouseEvent& e) override;

    /// Handles the user clicking the save or delete preset buttons.
    void buttonClicked(Button* button) override;

    /// Called when the colour changes.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Fills the background with the window colour.
    void paint(Graphics& g) override;
    /// Lays out the colour editor, list, preset combo, and buttons.
    void resized() override;
    /// Reloads the selected preset when the combo box changes.
    ///
    /// @param comboBoxThatHasChanged The combo box whose selection changed.
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

  private:
    /// Helper method to load an SVG file from a binary chunk of data.
    ///
    /// @param dataToInitialiseFrom Pointer to the binary data containing the SVG.
    /// @param sizeInBytes The size of the binary data in bytes.
    /// @return A Drawable created from the SVG data, or nullptr on failure.
    Drawable* loadSVGFromMemory(const void* dataToInitialiseFrom, size_t sizeInBytes);

    std::unique_ptr<ColourSelector> colourEditor;
    std::unique_ptr<ListBox> colourSelector;
    std::unique_ptr<ComboBox> presetSelector;
    std::unique_ptr<DrawableButton> deleteButton;
    std::unique_ptr<DrawableButton> saveButton;
    std::unique_ptr<DrawableButton> newButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColourSchemeEditor)
};

#endif
