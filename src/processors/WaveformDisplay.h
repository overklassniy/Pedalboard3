// WaveformDisplay.h - A component that displays a waveform.
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

#ifndef WAVEFORMDISPLAY_H_
#define WAVEFORMDISPLAY_H_

#include <JuceHeader.h>

/// A component that displays a waveform.
class WaveformDisplay : public Component, public ChangeListener, public ChangeBroadcaster {
  public:
    /// Constructs the display with an optional external thumbnail; creates one if none is provided.
    ///
    /// @param thumb External audio thumbnail to use, or nullptr to create one internally.
    /// @param deleteThumb Whether to delete the thumbnail in the destructor.
    WaveformDisplay(AudioThumbnail* thumb = nullptr, bool deleteThumb = true);
    /// Removes the change listener and deletes the thumbnail if owned.
    ~WaveformDisplay() override;

    /// Draws the background, waveform channels, read pointer line, and file length label.
    void paint(Graphics& g) override;
    /// Repaints when the thumbnail source changes during loading.
    ///
    /// @param source The change broadcaster that triggered the callback.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Sets the read pointer to the clicked position and notifies listeners.
    ///
    /// @param e The mouse event containing the click position.
    void mouseDown(const MouseEvent& e) override;
    /// Updates the read pointer as the user drags and notifies listeners.
    ///
    /// @param e The mouse event containing the drag position.
    void mouseDrag(const MouseEvent& e) override;

    /// Loads an audio file into the thumbnail and resets the read pointer.
    ///
    /// @param file The audio file to load; pass an empty File to clear.
    void setFile(const File& file);
    /// Updates the read pointer position (0.0 to 1.0) and repaints.
    ///
    /// @param val The new read pointer position, normalized from 0.0 to 1.0.
    void setReadPointer(float val);
    /// Returns the last user-requested read pointer position (0.0 to 1.0).
    float getReadPointer() const { return newReadPointer; }

    /// Sets the background colour.
    ///
    /// @param col The new background colour.
    void setBackgroundColour(const Colour& col);

  private:
    /// The current thumbnail.
    AudioThumbnail* thumbnail;

    /// The current position of the read pointer.
    float readPointer;
    /// Where the user clicked to move the read pointer.
    float newReadPointer;

    /// The background colour.
    Colour backgroundColour;

    /// True if we should delete the thumbnail in our destructor.
    bool deleteThumbnail;
};

/// A simplified component that displays a waveform.
class WaveformDisplayLite : public Component, public ChangeListener {
  public:
    /// Constructs the lite display and registers as a change listener on the thumbnail.
    ///
    /// @param thumb The external audio thumbnail to display.
    WaveformDisplayLite(AudioThumbnail& thumb);
    /// Unregisters the change listener from the thumbnail.
    ~WaveformDisplayLite() override;

    /// Draws the background, waveform channels, and file length label.
    void paint(Graphics& g) override;
    /// Repaints when the thumbnail source changes during loading.
    ///
    /// @param source The change broadcaster that triggered the callback.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Sets the background colour.
    ///
    /// @param col The new background colour.
    void setBackgroundColour(const Colour& col);

  private:
    /// The current thumbnail.
    AudioThumbnail& thumbnail;

    /// The background colour.
    Colour backgroundColour;
};

#endif
