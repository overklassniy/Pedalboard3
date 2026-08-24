// LevelEditor.h - The control and editor for LevelProcessor.
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

#ifndef LEVELEDITOR_H_
#define LEVELEDITOR_H_

#include <JuceHeader.h>
#include <memory>

class LevelProcessor;

/// The PluginComponent control for LevelProcessor.
class LevelControl : public Component, public Timer, public juce::Slider::Listener {
  public:
    /// Creates the compact rotary slider control for the given processor.
    ///
    /// @param proc The level processor to associate with this control.
    LevelControl(LevelProcessor* proc);
    ~LevelControl() override;

    /// Updates the slider to reflect the processor's level.
    void timerCallback() override;
    /// Sets the processor's level from the slider.
    ///
    /// @param slider The slider whose value changed (unused; reads the member slider).
    void sliderValueChanged(Slider* slider) override;

  private:
    /// Associated LevelProcessor.
    LevelProcessor* processor;

    /// Rotary level slider.
    std::unique_ptr<Slider> slider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelControl)
};

/// The full editor for LevelProcessor.
class LevelEditor : public AudioProcessorEditor, public Timer, public juce::Slider::Listener {
  public:
    /// Creates the full editor with a large rotary slider and restores window bounds.
    ///
    /// @param processor The audio processor to associate with this editor.
    /// @param windowBounds The saved window bounds to restore.
    LevelEditor(AudioProcessor* processor, const Rectangle<int>& windowBounds);
    ~LevelEditor() override;

    /// Resizes the slider to fill the window.
    void resized() override;
    /// Fills the background with the window colour.
    void paint(Graphics& g) override;

    /// Updates the slider to reflect the processor's level.
    void timerCallback() override;
    /// Sets the processor's level from the slider.
    ///
    /// @param slider The slider whose value changed (unused; reads the member slider).
    void sliderValueChanged(Slider* slider) override;

  private:
    /// Rotary level slider.
    std::unique_ptr<Slider> slider;

    /// Bounds of the parent window.
    Rectangle<int> parentBounds;

    /// True once the window position has been restored.
    bool setPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelEditor)
};

#endif
