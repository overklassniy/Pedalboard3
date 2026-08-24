// OutputToggleEditor.h - The control and editor for OutputToggleProcessor.
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

#ifndef OUTPUTTOGGLEEDITOR_H_
#define OUTPUTTOGGLEEDITOR_H_

#include <JuceHeader.h>
#include <memory>

class OutputToggleProcessor;

/// The PluginComponent control for OutputToggleProcessor.
class OutputToggleControl : public Component,
                            public Timer,
                            public juce::Button::Listener
{
  public:
    OutputToggleControl(OutputToggleProcessor* proc);
    ~OutputToggleControl() override;

    /// Updates the button to reflect the processor's state.
    void timerCallback() override;
    /// Toggles between outputs.
    void buttonClicked(Button* button) override;

  private:
    /// Associated OutputToggleProcessor.
    OutputToggleProcessor* processor;

    /// The toggle button.
    std::unique_ptr<DrawableButton> toggleButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputToggleControl)
};

/// The full editor for OutputToggleProcessor.
class OutputToggleEditor : public AudioProcessorEditor,
                           public Timer,
                           public juce::Button::Listener
{
  public:
    OutputToggleEditor(AudioProcessor* processor,
                       const Rectangle<int>& windowBounds);
    ~OutputToggleEditor() override;

    /// Resizes the button to fill the window.
    void resized() override;
    /// Fills the background with the window colour.
    void paint(Graphics& g) override;

    /// Updates the button and restores the editor window bounds.
    void timerCallback() override;
    /// Toggles between outputs.
    void buttonClicked(Button* button) override;

  private:
    /// The toggle button.
    std::unique_ptr<DrawableButton> toggleButton;

    /// Bounds of the parent window.
    Rectangle<int> parentBounds;

    /// True once the window position has been restored.
    bool setPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputToggleEditor)
};

#endif
