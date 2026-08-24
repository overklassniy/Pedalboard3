// VuMeterEditor.h - The control and editor for VuMeterProcessor.
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

#ifndef VUMETEREDITOR_H_
#define VUMETEREDITOR_H_

#include <JuceHeader.h>
#include <memory>

class VuMeterProcessor;

/// The PluginComponent control for the VU meter processor.
class VuMeterControl : public Component,
                       public Timer
{
  public:
    VuMeterControl(VuMeterProcessor* proc);
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

/// The full editor for VuMeterProcessor.
class VuMeterEditor : public AudioProcessorEditor
{
  public:
    VuMeterEditor(AudioProcessor* processor,
                  const Rectangle<int>& windowBounds);
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

#endif
