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
class VuMeterControl : public Component, public Timer {
  public:
    /// Constructs the control, starts a 60 ms repaint timer, and sets the fixed size.
    ///
    /// @param proc The VU meter processor to associate with this control.
    VuMeterControl(VuMeterProcessor* proc);
    /// Destructor; stops the repaint timer.
    ~VuMeterControl() override;

    /// Draws the dual-channel VU meter bars, scale lines, and dB labels.
    void paint(Graphics& g) override;
    /// No layout logic; the meter fills the component bounds set at construction.
    void resized() override;

    /// Polls the processor for current left/right levels, converts to dB, and triggers a repaint.
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
class VuMeterEditor : public AudioProcessorEditor {
  public:
    /// Constructs the editor, creates the VU meter control, and sets the window size.
    ///
    /// @param processor The audio processor to associate with this editor.
    /// @param windowBounds The saved window bounds to restore.
    VuMeterEditor(AudioProcessor* processor, const Rectangle<int>& windowBounds);
    /// Saves the current window bounds back to the processor before deletion.
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
