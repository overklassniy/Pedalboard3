// RecorderProcessor.h - Basic processor used to record audio.
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

#ifndef RECORDERPROCESSOR_H_
#define RECORDERPROCESSOR_H_

#include "PedalboardProcessor.h"

/// Basic processor used to record audio.
class RecorderProcessor : public PedalboardProcessor, public ChangeListener, public ChangeBroadcaster {
  public:
    /// Constructs the recorder and registers it with the main transport.
    RecorderProcessor();
    /// Stops any active recording, deletes the writer, and unregisters from the transport.
    ~RecorderProcessor() override;

    /// Sets the file to record to, creating a WAV writer for it.
    ///
    /// If a recording is in progress it is stopped first. Any existing file
    /// at the path is deleted so the new recording overwrites it. Passing an
    /// empty File finalises and closes the current recording.
    ///
    /// @param phil The file path to record to; pass an empty File to finalise.
    void setFile(const File& phil);
    /// Stores the file path without creating a writer (used for state restore).
    ///
    /// @param phil The file path to cache for later use.
    void cacheFile(const File& phil);
    /// Returns the sound file.
    const File& getFile() const { return soundFile; }
    /// Returns whether recording is currently active.
    bool isRecording() const { return recording; }

    /// Returns the component which is added to the instance's PluginComponent.
    ///
    /// @return A new AudioRecorderControl component; deleted by the caller.
    Component* getControls();
    /// Returns the size of the controls component.
    Point<int> getSize() override { return Point<int>(300, 100); }

    /// Updates the bounds of our editor window.
    ///
    /// @param bounds The new editor window bounds to store.
    void updateEditorBounds(const Rectangle<int>& bounds);

    /// Handles main transport state changes to start or stop recording when
    /// sync mode is enabled.
    ///
    /// @param source The change broadcaster that triggered the callback.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Provides a description of the processor to the filter graph.
    ///
    /// @param description The plugin description to fill in.
    void fillInPluginDescription(PluginDescription& description) const override;

    /// Writes the incoming audio to the threaded WAV writer.
    ///
    /// @param buffer The audio buffer to record from.
    /// @param midiMessages The MIDI buffer (unused).
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    /// Parameter indices used by the legacy get/set parameter methods.
    enum {
        Record = 0,
        SyncToMainTransport,

        NumParameters
    };

    /// Returns the name of the processor.
    const String getName() const override { return "Audio Recorder"; }
    /// Stores the sample rate for the WAV writer; no buffer allocation needed.
    ///
    /// @param sampleRate The current sample rate in Hz.
    /// @param estimatedSamplesPerBlock The maximum number of samples per block (unused).
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override;
    /// No resources to release.
    void releaseResources() override {}
    /// Returns the length of the plugin's tail.
    double getTailLengthSeconds() const override { return 0.0; }
    /// This processor does not accept MIDI.
    bool acceptsMidi() const override { return false; }
    /// This processor does not produce MIDI.
    bool producesMidi() const override { return false; }
    /// Creates the AudioRecorderEditor for this processor.
    AudioProcessorEditor* createEditor() override;
    /// This processor has a custom editor.
    bool hasEditor() const override { return true; }

    // JUCE 8: deprecated parameter methods kept as regular methods for
    // internal use by control components.
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

    /// No programs; returns 0.
    int getNumPrograms() override { return 0; }
    /// No programs; returns 0.
    int getCurrentProgram() override { return 0; }
    /// No programs; no-op.
    void setCurrentProgram(int index) override {}
    /// No programs; returns an empty string.
    const String getProgramName(int index) override { return ""; }
    /// No programs; no-op.
    void changeProgramName(int index, const String& newName) override {}
    /// Serializes the editor bounds, file path, and sync flag to destData.
    ///
    /// @param destData The memory block to serialize state into.
    void getStateInformation(juce::MemoryBlock& destData) override;
    /// Restores the editor bounds, file path, and sync flag from data.
    ///
    /// @param data Pointer to the serialized state data.
    /// @param sizeInBytes Size of the serialized state data in bytes.
    void setStateInformation(const void* data, int sizeInBytes) override;

  private:
    /// The file being recorded to.
    File soundFile;
    /// Used to record the audio input.
    AudioFormatWriter::ThreadedWriter* threadWriter;

    /// The thumbnail image which gets passed to the Controls and Editor.
    AudioThumbnail thumbnail;

    /// If we're currently recording or not.
    bool recording;
    /// Safeguard in case the user tries to change the file while we're recording.
    bool stopRecording;
    /// Whether or not we're syncing to the main transport.
    bool syncToMainTransport;

    /// The editor's bounds.
    Rectangle<int> editorBounds;

    /// The samplerate passed to prepareToPlay().
    double currentRate;
};

#endif
