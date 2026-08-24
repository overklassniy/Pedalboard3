// MappingEntryMidi.h - MIDI mapping entry UI component.
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

#ifndef MAPPINGENTRYMIDI_H_
#define MAPPINGENTRYMIDI_H_

#include "MappingSlider.h"

#include <JuceHeader.h>

class MappingsDialog;

/// A single row in the MappingsDialog representing a MIDI CC mapping.
class MappingEntryMidi : public Component,
                         public juce::ComboBox::Listener,
                         public juce::Button::Listener,
                         public MappingSliderListener {
  public:
    /// Creates a MIDI mapping entry row.
    ///
    /// @param dlg The parent MappingsDialog.
    /// @param arrayIndex The row's position in the mappings array.
    /// @param cc The initial MIDI CC.
    /// @param latch Enables latched mode.
    /// @param lowerBound The lower bound of the parameter range.
    /// @param upperBound The upper bound of the parameter range.
    MappingEntryMidi(MappingsDialog* dlg, int arrayIndex, int cc, bool latch, float lowerBound, float upperBound);
    ~MappingEntryMidi() override;

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
    /// Forwards CC selection or MIDI learn activation to the parent dialog.
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
    /// Forwards latch toggle changes to the parent dialog.
    void buttonClicked(Button* buttonThatWasClicked) override;
    /// Forwards lower/upper bound changes to the parent dialog.
    void sliderValueChanged(MappingSlider* sliderThatWasMoved) override;

  private:
    /// The MappingsDialog which holds the mappings array for this plugin.
    MappingsDialog* mappingsDialog;
    /// The index of the mapping within its parent's Array.
    int index;
    /// True if we are currently in MIDI learn mode.
    bool midiLearn;

    std::unique_ptr<ComboBox> ccComboBox;
    std::unique_ptr<ToggleButton> latchButton;
    std::unique_ptr<MappingSlider> slider;
    std::unique_ptr<Label> rangeLabel;
    std::unique_ptr<ComboBox> paramComboBox;
    Path internalPath1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingEntryMidi)
};

#endif
