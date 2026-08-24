// TapTempoBox.h - Simple component letting the user tap the tempo.
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

#ifndef TAPTEMPOBOX_H_
#define TAPTEMPOBOX_H_

#include "TapTempoHelper.h"

#include <JuceHeader.h>

class PluginField;

/// Simple component letting the user tap the tempo.
class TapTempoBox : public Component, public Timer {
  public:
    /// Creates the tap tempo box.
    ///
    /// @param field The PluginField to send tempo updates to.
    /// @param tempoEd The text editor that displays the current tempo.
    TapTempoBox(PluginField* field, TextEditor* tempoEd);
    ~TapTempoBox() override;

    /// Draws the box.
    void paint(Graphics& g) override;

    /// Used to tap the tempo.
    void mouseDown(const MouseEvent& e) override;

    /// Updates the current tempo.
    void timerCallback() override;

  private:
    /// The current tempo.
    double tempo;

    /// The PluginField we get the tempo from.
    PluginField* pluginField;
    /// The tempo TextEditor to update with the new tempo.
    TextEditor* tempoEditor;

    /// Used to calculate the tempo.
    TapTempoHelper tapHelper;
};

#endif
