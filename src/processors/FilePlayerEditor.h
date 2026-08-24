// FilePlayerEditor.h - The full editor for FilePlayerProcessor.
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

#ifndef FILEPLAYEREDITOR_H_
#define FILEPLAYEREDITOR_H_

#include "FilePlayerControl.h"

#include <JuceHeader.h>
#include <memory>

class FilePlayerProcessor;

/// The full editor for FilePlayerProcessor.
class FilePlayerEditor : public AudioProcessorEditor, public Timer {
  public:
    /// Creates the editor with the file player controls and restores window bounds.
    ///
    /// @param processor The file player processor to associate with this editor.
    /// @param windowBounds The saved window bounds to restore.
    FilePlayerEditor(FilePlayerProcessor* processor, const Rectangle<int>& windowBounds);
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

#endif
