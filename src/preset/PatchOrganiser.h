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
class PatchOrganiser : public Component,
                       public ListBoxModel,
                       public Label::Listener,
                       public juce::Button::Listener
{
  public:
    /// Constructor.
    PatchOrganiser(MainPanel* panel, Array<XmlElement*>& patchArray);
    /// Destructor.
    ~PatchOrganiser() override;

    /// Returns the number of active mappings.
    int getNumRows() override;
    /// Draws a row.
    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
    /// Returns the component for a single row.
    Component* refreshComponentForRow(int rowNumber, bool isRowSelected, Component* existingComponentToUpdate) override;
    /// So the user can select rows.
    void listBoxItemClicked(int row, const MouseEvent& e) override;
    /// So the user can deselect rows.
    void backgroundClicked(const MouseEvent& e) override;

    /// Called when the user changes a patch name by editing the label.
    void labelTextChanged(Label* labelThatHasChanged) override;

    void paint(Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* buttonThatWasClicked) override;

  private:
    /// The MainPanel.
    MainPanel* mainPanel;
    /// Our copy of the available patches.
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
