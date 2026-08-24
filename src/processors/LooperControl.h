// LooperControl.h - UI control for the looper processor.
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

#ifndef LOOPERCONTROL_H_
#define LOOPERCONTROL_H_

#include "WaveformDisplay.h"

#include <JuceHeader.h>
#include <memory>

class LooperProcessor;

/// UI control for the looper processor.
class LooperControl : public Component,
                      public FilenameComponentListener,
                      public Timer,
                      public ChangeListener,
                      public juce::Button::Listener {
  public:
    /// Creates the control with the associated processor and waveform thumbnail.
    ///
    /// @param proc The looper processor to associate with this control.
    /// @param thumbnail The audio thumbnail to display in the waveform view.
    LooperControl(LooperProcessor* proc, AudioThumbnail* thumbnail);
    /// Removes change listeners and child components.
    ~LooperControl() override;

    /// Called when the user selects a sound file.
    ///
    /// @param filenameComp The filename component whose selected file changed.
    void filenameComponentChanged(FilenameComponent* filenameComp) override;
    /// Called on each timer tick to update the read position.
    void timerCallback() override;
    /// Called when the waveform display or processor broadcasts a change.
    ///
    /// @param source The change broadcaster that triggered the callback.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Changes the waveform display background colour.
    ///
    /// @param col The new background colour for the waveform display.
    void setWaveformBackground(const Colour& col);
    /// Clears the waveform display.
    void clearDisplay();

    /// Last directory used for file browsing.
    static File lastDir;

    /// Empty paint override.
    void paint(Graphics& g) override;
    /// Lays out the waveform display, filename selector and buttons.
    void resized() override;
    /// Handles sync, play/pause, return-to-zero and record button clicks.
    ///
    /// @param buttonThatWasClicked The button that was clicked by the user.
    void buttonClicked(Button* buttonThatWasClicked) override;

  private:
    /// Associated looper processor.
    LooperProcessor* processor;

    /// SVG images used for the play/pause button states.
    std::unique_ptr<Drawable> playImage;
    std::unique_ptr<Drawable> pauseImage;
    /// Whether the play/pause button is showing the play icon.
    bool playing;

    /// SVG images used for the record button states.
    std::unique_ptr<Drawable> recordImage;
    std::unique_ptr<Drawable> stopImage;
    /// Whether the record button is showing the record icon.
    bool recording;

    /// Waveform display for the loop.
    std::unique_ptr<WaveformDisplay> fileDisplay;
    /// Filename selector.
    std::unique_ptr<FilenameComponent> filename;
    /// Sync to main transport toggle.
    std::unique_ptr<ToggleButton> syncButton;
    /// Stop recording after a bar toggle.
    std::unique_ptr<ToggleButton> stopAfterBarButton;
    /// Play/pause button.
    std::unique_ptr<DrawableButton> playPauseButton;
    /// Return-to-zero button.
    std::unique_ptr<DrawableButton> rtzButton;
    /// Record button.
    std::unique_ptr<DrawableButton> recordButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LooperControl)
};

#endif
