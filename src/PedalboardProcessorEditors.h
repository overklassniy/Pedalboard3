// PedalboardProcessorEditors.h - The various editors for the app's internal processors.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2026 Niall Moody.
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
//

#ifndef PEDALBOARDPROCESSOREDITORS_H_
#define PEDALBOARDPROCESSOREDITORS_H_

#include <JuceHeader.h>
#include <memory>

#include "AudioRecorderControl.h"
#include "FilePlayerControl.h"
#include "MetronomeControl.h"

class LevelProcessor;
class FilePlayerProcessor;
class OutputToggleProcessor;
class VuMeterProcessor;
class RecorderProcessor;
class MetronomeProcessor;
class LooperProcessor;

//------------------------------------------------------------------------------
/// The PluginComponent control for LevelProcessor.
class LevelControl : public Component,
                     public Timer,
                     public juce::Slider::Listener
{
  public:
    /// Constructor.
    LevelControl(LevelProcessor* proc);
    /// Destructor.
    ~LevelControl() override;

    /// Updates the slider to reflect the processor's level.
    void timerCallback() override;
    /// Sets the processor's level from the slider.
    void sliderValueChanged(Slider* slider) override;

  private:
    /// Associated LevelProcessor.
    LevelProcessor* processor;

    /// Rotary level slider.
    std::unique_ptr<Slider> slider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelControl)
};

//------------------------------------------------------------------------------
/// The full editor for LevelProcessor.
class LevelEditor : public AudioProcessorEditor,
                    public Timer,
                    public juce::Slider::Listener
{
  public:
    /// Constructor.
    LevelEditor(AudioProcessor* processor, const Rectangle<int>& windowBounds);
    /// Destructor.
    ~LevelEditor() override;

    /// Resizes the slider to fill the window.
    void resized() override;
    /// Fills the background with the window colour.
    void paint(Graphics& g) override;

    /// Updates the slider to reflect the processor's level.
    void timerCallback() override;
    /// Sets the processor's level from the slider.
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

//------------------------------------------------------------------------------
/// The full editor for FilePlayerProcessor.
class FilePlayerEditor : public AudioProcessorEditor,
                         public Timer
{
  public:
    /// Constructor.
    FilePlayerEditor(FilePlayerProcessor* processor,
                     const Rectangle<int>& windowBounds);
    /// Destructor.
    ~FilePlayerEditor() override;

    /// Resizes the controls to fill the window.
    void resized() override;
    /// Fills the background with the window colour.
    void paint(Graphics& g) override;

    /// Restores the editor window bounds.
    void timerCallback() override;

  private:
    /// The actual file player controls.
    std::unique_ptr<FilePlayerControl> controls;

    /// Bounds of the parent window.
    Rectangle<int> parentBounds;

    /// True once the window position has been restored.
    bool setPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilePlayerEditor)
};

//------------------------------------------------------------------------------
/// The PluginComponent control for OutputToggleProcessor.
class OutputToggleControl : public Component,
                            public Timer,
                            public juce::Button::Listener
{
  public:
    /// Constructor.
    OutputToggleControl(OutputToggleProcessor* proc);
    /// Destructor.
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

//------------------------------------------------------------------------------
/// The full editor for OutputToggleProcessor.
class OutputToggleEditor : public AudioProcessorEditor,
                           public Timer,
                           public juce::Button::Listener
{
  public:
    /// Constructor.
    OutputToggleEditor(AudioProcessor* processor,
                       const Rectangle<int>& windowBounds);
    /// Destructor.
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

//------------------------------------------------------------------------------
/// The PluginComponent control for the VU meter processor.
class VuMeterControl : public Component,
                       public Timer
{
  public:
    /// Constructor.
    VuMeterControl(VuMeterProcessor* proc);
    /// Destructor.
    ~VuMeterControl() override;

    /// Draws the meter.
    void paint(Graphics& g) override;
    /// Resizes the meter.
    void resized() override;

    /// Updates the meter.
    void timerCallback() override;

  private:
    /// Associated VuMeterProcessor.
    VuMeterProcessor* processor;

    /// Current left channel level.
    float levelLeft;
    /// Current right channel level.
    float levelRight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VuMeterControl)
};

//------------------------------------------------------------------------------
/// The full editor for VuMeterProcessor.
class VuMeterEditor : public AudioProcessorEditor
{
  public:
    /// Constructor.
    VuMeterEditor(AudioProcessor* processor,
                  const Rectangle<int>& windowBounds);
    /// Destructor.
    ~VuMeterEditor() override;

    /// Resizes the meter to fill the window.
    void resized() override;
    /// Fills the background with the window colour.
    void paint(Graphics& g) override;

  private:
    /// The VU meter control.
    std::unique_ptr<VuMeterControl> meter;

    /// Bounds of the parent window.
    Rectangle<int> parentBounds;

    /// True once the window position has been restored.
    bool setPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VuMeterEditor)
};

//------------------------------------------------------------------------------
/// The full editor for RecorderProcessor.
class AudioRecorderEditor : public AudioProcessorEditor,
                            public Timer
{
  public:
    /// Constructor.
    AudioRecorderEditor(RecorderProcessor* processor,
                        const Rectangle<int>& windowBounds,
                        AudioThumbnail& thumbnail);
    /// Destructor.
    ~AudioRecorderEditor() override;

    /// Resizes the controls to fill the window.
    void resized() override;
    /// Fills the background with the window colour.
    void paint(Graphics& g) override;

    /// Restores the editor window bounds.
    void timerCallback() override;

  private:
    /// The actual audio recorder controls.
    std::unique_ptr<AudioRecorderControl> controls;

    /// Bounds of the parent window.
    Rectangle<int> parentBounds;

    /// True once the window position has been restored.
    bool setPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRecorderEditor)
};

//------------------------------------------------------------------------------
/// The full editor for MetronomeProcessor.
class MetronomeEditor : public AudioProcessorEditor,
                        public Timer
{
  public:
    /// Constructor.
    MetronomeEditor(MetronomeProcessor* processor,
                    const Rectangle<int>& windowBounds);
    /// Destructor.
    ~MetronomeEditor() override;

    /// Resizes the controls to fill the window.
    void resized() override;
    /// Fills the background with the window colour.
    void paint(Graphics& g) override;

    /// Restores the editor window bounds.
    void timerCallback() override;

  private:
    /// The actual metronome controls.
    std::unique_ptr<MetronomeControl> controls;

    /// Bounds of the parent window.
    Rectangle<int> parentBounds;

    /// True once the window position has been restored.
    bool setPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetronomeEditor)
};

#endif
