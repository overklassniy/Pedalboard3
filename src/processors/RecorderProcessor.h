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
class RecorderProcessor : public PedalboardProcessor,
                          public ChangeListener,
                          public ChangeBroadcaster
{
  public:
    RecorderProcessor();
    ~RecorderProcessor() override;

    /// Sets the sound file to play.
    void setFile(const File& phil);
    /// Stores the sound file to record to.
    void cacheFile(const File& phil);
    /// Returns the sound file.
    const File& getFile() const { return soundFile; }
    /// Returns whether or not we're currently recording.
    bool isRecording() const { return recording; }

    /// Returns the component which is added to the instance's PluginComponent.
    Component* getControls();
    /// Returns the size of the controls component.
    Point<int> getSize() override { return Point<int>(300, 100); }

    /// Updates the bounds of our editor window.
    void updateEditorBounds(const Rectangle<int>& bounds);

    /// So we can listen to the main transport.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Provides a description of the processor to the filter graph.
    void fillInPluginDescription(PluginDescription& description) const override;

    /// Alters the input audio's level accordingly.
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    /// Parameter constants.
    enum
    {
        Record = 0,
        SyncToMainTransport,

        NumParameters
    };

    /// Returns the name of the processor.
    const String getName() const override { return "Audio Recorder"; }
    /// Ignored.
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override;
    /// Ignored.
    void releaseResources() override {}
    /// Returns the length of the plugin's tail.
    double getTailLengthSeconds() const override { return 0.0; }
    /// We definitely want Midi input.
    bool acceptsMidi() const override { return false; }
    /// But we don't need to output it.
    bool producesMidi() const override { return false; }
    /// We have no editor.
    AudioProcessorEditor* createEditor() override;
    /// We have no editor.
    bool hasEditor() const override { return true; }

    // JUCE 8: deprecated parameter methods kept as regular methods for
    // internal use by control components.
    /// Returns the parameter name.
    const String getParameterName(int parameterIndex);
    /// Returns the parameter value (0-1 normalized).
    float getParameter(int parameterIndex);
    /// Returns the parameter's value as a string.
    const String getParameterText(int parameterIndex);
    /// Sets the parameter value (0-1 normalized).
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
    /// Loads the position of the slider and the size and position of the editor.
    void getStateInformation(juce::MemoryBlock& destData) override;
    /// Saves the position of the slider and the size and position of the editor.
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
