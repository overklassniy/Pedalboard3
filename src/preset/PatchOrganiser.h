// PatchOrganiser.h - Patch management and navigation component.
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

#ifndef PATCHORGANISER_H_
#define PATCHORGANISER_H_

#include <JuceHeader.h>

class MainPanel;

/// Component for managing patches (collections of filter graph states).
///
/// Displays a list of patches with buttons to add, copy, remove, reorder,
/// and import patches. Patch names can be edited by double-clicking.
class PatchOrganiser : public Component, public ListBoxModel, public Label::Listener, public juce::Button::Listener {
  public:
    /// Creates the patch organiser linked to the given MainPanel and patch array.
    ///
    /// @param panel The MainPanel that owns this organiser.
    /// @param patchArray The shared patch array owned by MainPanel.
    PatchOrganiser(MainPanel* panel, Array<XmlElement*>& patchArray);
    /// Destructor.
    ~PatchOrganiser() override;

    /// Returns the number of patches in the list.
    ///
    /// @return The number of patches.
    int getNumRows() override;
    /// Draws the background for a list row when it is selected or alternating.
    ///
    /// @param rowNumber The index of the row to paint.
    /// @param g The graphics context to draw with.
    /// @param width The width of the row in pixels.
    /// @param height The height of the row in pixels.
    /// @param rowIsSelected Whether the row is currently selected.
    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
    /// Returns the editable label component for a single row, reusing existing ones.
    ///
    /// @param rowNumber The index of the row to refresh.
    /// @param isRowSelected Whether the row is currently selected.
    /// @param existingComponentToUpdate The existing component to reuse or delete.
    ///
    /// @return The label component for the row, or nullptr if the row is out of range.
    Component* refreshComponentForRow(int rowNumber, bool isRowSelected, Component* existingComponentToUpdate) override;
    /// Handles row selection and double-click to edit patch names.
    ///
    /// @param row The index of the clicked row.
    /// @param e The mouse event details.
    void listBoxItemClicked(int row, const MouseEvent& e) override;
    /// Deselects all rows when the background is clicked.
    ///
    /// @param e The mouse event details.
    void backgroundClicked(const MouseEvent& e) override;

    /// Called when the user changes a patch name by editing the label.
    ///
    /// @param labelThatHasChanged The label whose text was changed.
    void labelTextChanged(Label* labelThatHasChanged) override;

    /// Paints the component background.
    ///
    /// @param g The graphics context to draw with.
    void paint(Graphics& g) override;
    /// Lays out the list box and buttons.
    void resized() override;
    /// Handles add, copy, remove, move, and import button clicks.
    ///
    /// @param buttonThatWasClicked The button that was clicked.
    void buttonClicked(Button* buttonThatWasClicked) override;

  private:
    /// The MainPanel that owns this organiser.
    MainPanel* mainPanel;
    /// Reference to the shared patch array owned by MainPanel.
    Array<XmlElement*>& patches;

    std::unique_ptr<ListBox> patchList;
    std::unique_ptr<TextButton> addButton;
    std::unique_ptr<TextButton> copyButton;
    std::unique_ptr<TextButton> removeButton;
    std::unique_ptr<TextButton> moveUpButton;
    std::unique_ptr<TextButton> moveDownButton;
    std::unique_ptr<TextButton> importButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatchOrganiser)
};

#endif
