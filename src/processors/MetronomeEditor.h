// MetronomeEditor.h - The full editor for MetronomeProcessor.
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

#ifndef METRONOMEEDITOR_H_
#define METRONOMEEDITOR_H_

#include <JuceHeader.h>
#include <memory>

#include "MetronomeControl.h"

class MetronomeProcessor;

/// The full editor for MetronomeProcessor.
class MetronomeEditor : public AudioProcessorEditor,
                        public Timer
{
  public:
    MetronomeEditor(MetronomeProcessor* processor,
                    const Rectangle<int>& windowBounds);
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
