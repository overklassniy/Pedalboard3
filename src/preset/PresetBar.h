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
class PresetBar : public Component,
                  public juce::Button::Listener,
                  public juce::ComboBox::Listener
{
  public:
    /// Constructor.
    PresetBar(PluginComponent* comp);
    /// Destructor.
    ~PresetBar() override;

    /// So we can do things when the user clicks one of the buttons.
    void buttonClicked(Button* button) override;

    void paint(Graphics& g) override;
    void resized() override;
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

  private:
    /// Helper method. Fills out presetsComboBox correctly.
    void fillOutComboBox();

    /// Helper method to load an SVG file from a binary chunk of data.
    Drawable* loadSVGFromMemory(const void* dataToInitialiseFrom, size_t sizeInBytes);

    /// The 'parent' PluginComponent.
    PluginComponent* component;

    /// Used to figure out which preset the user's just changed the name of.
    int lastComboBox;

    std::unique_ptr<ComboBox> presetsComboBox;
    std::unique_ptr<Label> presetsLabel;
    std::unique_ptr<DrawableButton> importButton;
    std::unique_ptr<DrawableButton> saveButton;
    Path internalPath1;

    JUCE_LEAK_DETECTOR(PresetBar)
};

#endif
