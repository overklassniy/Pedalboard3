// MappingEntryOsc.h - OSC mapping entry UI component.
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

#ifndef MAPPINGENTRYOSC_H_
#define MAPPINGENTRYOSC_H_

#include <JuceHeader.h>

class MappingsDialog;

/// A single row in the MappingsDialog representing an OSC mapping.
class MappingEntryOsc : public Component, public juce::ComboBox::Listener, public juce::Slider::Listener {
  public:
    /// Creates an OSC mapping entry row.
    ///
    /// @param dlg The parent MappingsDialog.
    /// @param arrayIndex The row's position in the mappings array.
    /// @param oscAddress The initial OSC address.
    /// @param oscParam The initial OSC parameter index.
    /// @param possibleAddresses Populates the address combo box with
    ///        previously received addresses.
    MappingEntryOsc(MappingsDialog* dlg, int arrayIndex, const String& oscAddress, int oscParam,
                    const StringArray& possibleAddresses);
    ~MappingEntryOsc() override;

    /// Updates the mapping's OSC address when the text changes.
    ///
    /// @param editor The text editor whose text changed.
    void textEditorTextChanged(TextEditor& editor);
    /// Updates the mapping's OSC address when the user presses Return.
    ///
    /// @param editor The text editor in which Return was pressed.
    void textEditorReturnKeyPressed(TextEditor& editor);
    /// Reverts any address changes the user made when Escape is pressed.
    ///
    /// @param editor The text editor in which Escape was pressed.
    void textEditorEscapeKeyPressed(TextEditor& editor);
    /// Updates the mapping's OSC address when the editor loses focus.
    ///
    /// @param editor The text editor that lost focus.
    void textEditorFocusLost(TextEditor& editor);

    /// Adds a parameter name to the parameter combo box.
    ///
    /// @param param The parameter name to add.
    void addParameter(const String& param);
    /// Selects the parameter at the given index in the combo box.
    ///
    /// @param index The zero-based index of the parameter to select.
    void selectParameter(int index);

    void paint(Graphics& g) override;
    void resized() override;
    /// Forwards parameter or address changes to the parent dialog.
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
    /// Forwards OSC parameter index changes to the parent dialog.
    void sliderValueChanged(Slider* sliderThatWasMoved) override;

  private:
    /// The MappingsDialog which holds the mappings array for this plugin.
    MappingsDialog* mappingsDialog;
    /// The index of the mapping within its parent's Array.
    int index;

    std::unique_ptr<ComboBox> paramComboBox;
    std::unique_ptr<Label> addressLabel;
    std::unique_ptr<ComboBox> addressEditor;
    std::unique_ptr<Label> oscParamLabel;
    std::unique_ptr<Slider> oscParamSlider;
    Path internalPath1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingEntryOsc)
};

#endif
