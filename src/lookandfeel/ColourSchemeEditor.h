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
                           public juce::ComboBox::Listener
{
  public:
    /// Constructor.
    ColourSchemeEditor();
    /// Destructor.
    ~ColourSchemeEditor() override;

    /// Returns the number of colours which can be edited.
    int getNumRows() override;
    /// Draws the colour selector list.
    void paintListBoxItem(int rowNumber,
                          Graphics& g,
                          int width,
                          int height,
                          bool rowIsSelected) override;
    /// So we know when the user selects a new colour.
    void listBoxItemClicked(int row, const MouseEvent& e) override;

    /// Handles the user clicking the save or delete preset buttons.
    void buttonClicked(Button* button) override;

    /// Called when the colour changes.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    void paint(Graphics& g) override;
    void resized() override;
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

  private:
    /// Helper method to load an SVG file from a binary chunk of data.
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
