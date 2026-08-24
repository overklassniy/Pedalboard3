// AudioRecorderEditor.h - The full editor for RecorderProcessor.
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

#ifndef AUDIORECORDEREDITOR_H_
#define AUDIORECORDEREDITOR_H_

#include <JuceHeader.h>
#include <memory>

#include "AudioRecorderControl.h"

class RecorderProcessor;

/// The full editor for RecorderProcessor.
class AudioRecorderEditor : public AudioProcessorEditor,
                            public Timer
{
  public:
    AudioRecorderEditor(RecorderProcessor* processor,
                        const Rectangle<int>& windowBounds,
                        AudioThumbnail& thumbnail);
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

#endif
