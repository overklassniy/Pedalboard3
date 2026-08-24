// FilePlayerProcessor.h - Processor which plays back an audio file.
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

#ifndef FILEPLAYERPROCESSOR_H_
#define FILEPLAYERPROCESSOR_H_

#include "PedalboardProcessor.h"

/// Processor which plays back an audio file.
class FilePlayerProcessor : public PedalboardProcessor, public ChangeListener, public ChangeBroadcaster {
  public:
    /// Constructs a file player with no file loaded.
    FilePlayerProcessor();
    /// Constructor which also sets the processor's sound file.
    ///
    /// @param phil The sound file to load for playback.
    FilePlayerProcessor(const File& phil);
    ~FilePlayerProcessor() override;

    /// Sets the sound file to play.
    ///
    /// @param phil The sound file to load; unloads any previous file.
    void setFile(const File& phil);
    /// Returns the sound file.
    const File& getFile() const { return soundFile; }
    /// Returns the current read position within the file (0->1).
    double getReadPosition() const {
        return transportSource.getCurrentPosition() / transportSource.getLengthInSeconds();
    }
    /// Returns whether the file is currently playing.
    bool isPlaying() const { return transportSource.isPlaying(); }

    /// Returns the component which is added to the instance's PluginComponent.
    ///
    /// @return A new FilePlayerControl component; deleted by the caller.
    Component* getControls();
    /// Returns the size of the controls component.
    Point<int> getSize() override { return Point<int>(300, 100); }

    /// Updates the bounds of our editor window.
    ///
    /// @param bounds The new editor window bounds to store.
    void updateEditorBounds(const Rectangle<int>& bounds);

    /// So we can reset the play position when we reach the end of the file.
    ///
    /// @param source The change broadcaster that triggered the callback.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Provides a description of the processor to the filter graph.
    ///
    /// @param description The plugin description to fill in.
    void fillInPluginDescription(PluginDescription& description) const override;

    /// Writes the transport source's audio into the output buffer.
    ///
    /// @param buffer The audio buffer to write the transport source's output into.
    /// @param midiMessages The MIDI buffer (unused).
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    /// Parameter constants.
    enum {
        Play = 0,
        ReturnToZero,
        Looping,
        ReadPosition,
        SyncToMainTransport,
        Trigger,

        NumParameters
    };

    /// Returns the name of the processor.
    const String getName() const override { return "File Player"; }
    /// Passed to transportSource.
    ///
    /// @param sampleRate The current sample rate in Hz.
    /// @param estimatedSamplesPerBlock The maximum number of samples per block.
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override;
    /// Passed to transportSource.
    void releaseResources() override;
    /// Returns the length of the plugin's tail.
    double getTailLengthSeconds() const override { return 0.0; }
    /// This processor does not accept MIDI input.
    bool acceptsMidi() const override { return false; }
    /// This processor does not produce MIDI output.
    bool producesMidi() const override { return false; }
    /// Creates the full editor window for this processor.
    AudioProcessorEditor* createEditor() override;
    /// Returns true; this processor has a custom editor.
    bool hasEditor() const override { return true; }

    /// JUCE 8: deprecated parameter methods kept as regular methods for
    /// internal use by control components.
    /// Returns the parameter name.
    ///
    /// @param parameterIndex The index of the parameter (see the enum above).
    /// @return The human-readable name of the parameter.
    const String getParameterName(int parameterIndex);
    /// Returns the parameter value (0-1 normalized).
    ///
    /// @param parameterIndex The index of the parameter (see the enum above).
    /// @return The current value of the parameter, normalized to 0-1.
    float getParameter(int parameterIndex);
    /// Returns the parameter's value as a string.
    ///
    /// @param parameterIndex The index of the parameter (see the enum above).
    /// @return A textual representation of the parameter's current value.
    const String getParameterText(int parameterIndex);
    /// Sets the parameter value (0-1 normalized).
    ///
    /// @param parameterIndex The index of the parameter (see the enum above).
    /// @param newValue The new value for the parameter, normalized to 0-1.
    void setParameter(int parameterIndex, float newValue);

    /// We have no programs.
    int getNumPrograms() override { return 0; }
    /// We have no programs.
    int getCurrentProgram() override { return 0; }
    /// We have no programs.
    void setCurrentProgram(int index) override {}
    /// We have no programs.
    const String getProgramName(int index) override { return ""; }
    /// We have no programs.
    void changeProgramName(int index, const String& newName) override {}
    /// Saves the file path, loop and sync state, and editor bounds to the block.
    ///
    /// @param destData The memory block to serialize state into.
    void getStateInformation(juce::MemoryBlock& destData) override;
    /// Restores the file path, loop and sync state, and editor bounds from the data.
    ///
    /// @param data Pointer to the serialized state data.
    /// @param sizeInBytes Size of the serialized state data in bytes.
    void setStateInformation(const void* data, int sizeInBytes) override;

  private:
    /// The transport source which plays the file.
    AudioTransportSource transportSource;
    /// The actual sound file source.
    std::unique_ptr<AudioFormatReaderSource> soundFileSource;

    /// The file we're playing.
    File soundFile;
    /// Whether or not we're looping this file.
    bool looping;
    /// Whether or not we're syncing to the main transport.
    bool syncToMainTransport;

    /// The editor's bounds.
    Rectangle<int> editorBounds;

    /// Used to ensure we don't reset the play position to zero when the user clicks pause.
    bool justPaused;
};

#endif
