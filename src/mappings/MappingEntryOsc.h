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
class MappingEntryOsc : public Component,
                        public juce::ComboBox::Listener,
                        public juce::Slider::Listener
{
public:
    MappingEntryOsc(MappingsDialog* dlg, int arrayIndex, const String& oscAddress,
                    int oscParam, const StringArray& possibleAddresses);
    ~MappingEntryOsc() override;

    /// Used to update the mapping's OSC address.
    void textEditorTextChanged(TextEditor& editor);
    /// Used to update the mapping's OSC address.
    void textEditorReturnKeyPressed(TextEditor& editor);
    /// Used to revert any changes the user made.
    void textEditorEscapeKeyPressed(TextEditor& editor);
    /// Used to update the mapping's OSC address.
    void textEditorFocusLost(TextEditor& editor);

    /// Fills out the parameter combo box.
    void addParameter(const String& param);
    /// Selects the currently-selected parameter in the combo box.
    void selectParameter(int index);

    void paint(Graphics& g) override;
    void resized() override;
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
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
