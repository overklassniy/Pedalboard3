// AudioRecorderControl.h - UI control for the audio recorder processor.
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

#ifndef AUDIORECORDERCONTROL_H_
#define AUDIORECORDERCONTROL_H_

#include <JuceHeader.h>
#include <memory>

class RecorderProcessor;
class WaveformDisplayLite;

/// UI control for the audio recorder processor.
class AudioRecorderControl : public Component,
                             public FilenameComponentListener,
                             public ChangeListener,
                             public juce::Button::Listener {
  public:
    /// Creates the control panel for the given recorder processor.
    ///
    /// @param proc The recorder processor to associate with this control.
    /// @param thumbnail The audio thumbnail to display in the waveform view.
    AudioRecorderControl(RecorderProcessor* proc, AudioThumbnail& thumbnail);
    ~AudioRecorderControl() override;

    /// Called when the user selects a sound file.
    ///
    /// @param filenameComp The filename component whose selected file changed.
    void filenameComponentChanged(FilenameComponent* filenameComp) override;
    /// Called when the processor broadcasts a change.
    ///
    /// @param source The change broadcaster that triggered the callback.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Changes the waveform display background colour.
    ///
    /// @param col The new background colour for the waveform display.
    void setWaveformBackground(const Colour& col);

    /// Empty paint override; the component has no custom painting.
    void paint(Graphics& g) override;
    /// Lays out the waveform display, filename selector, and buttons.
    void resized() override;
    /// Handles sync toggle and record button clicks.
    ///
    /// @param buttonThatWasClicked The button that was clicked by the user.
    void buttonClicked(Button* buttonThatWasClicked) override;

  private:
    /// Associated audio recorder processor.
    RecorderProcessor* processor;

    /// SVG images used for the record button states.
    std::unique_ptr<Drawable> recordImage;
    std::unique_ptr<Drawable> stopImage;
    /// Whether the record button is currently showing the record icon.
    bool recording;

    /// Waveform display for the recorded file.
    std::unique_ptr<WaveformDisplayLite> fileDisplay;
    /// Filename selector.
    std::unique_ptr<FilenameComponent> filename;
    /// Sync to main transport toggle.
    std::unique_ptr<ToggleButton> syncButton;
    /// Record toggle button.
    std::unique_ptr<DrawableButton> recordButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRecorderControl)
};

#endif
