// MetronomeProcessor.h - Simple metronome processor.
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

#ifndef METRONOMEPROCESSOR_H_
#define METRONOMEPROCESSOR_H_

#include "PedalboardProcessor.h"

/// Simple metronome processor.
class MetronomeProcessor : public PedalboardProcessor,
                           public ChangeListener,
                           public ChangeBroadcaster
{
  public:
    MetronomeProcessor();
    ~MetronomeProcessor() override;

    /// Returns true if the metronome is currently playing.
    bool isPlaying() const { return playing; }
    /// Sets the accent sound file to play.
    void setAccentFile(const File& phil);
    /// Returns the accent sound file.
    const File& getAccentFile() const { return files[0]; }
    /// Sets the click sound file to play.
    void setClickFile(const File& phil);
    /// Returns the click sound file.
    const File& getClickFile() const { return files[1]; }

    /// Returns the component which is added to the instance's PluginComponent.
    Component* getControls();
    /// Returns the size of the controls component.
    Point<int> getSize() override { return Point<int>(170, 100); }

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
        Play = 0,
        Numerator,
        Denominator,
        SyncToMainTransport,

        NumParameters
    };

    /// Returns the name of the processor.
    const String getName() const override { return "Metronome"; }
    /// Ignored.
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override {}
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
    /// The transport sources which play the accent and click files.
    AudioTransportSource transportSource[2];
    /// The accent and click sound file sources.
    std::unique_ptr<AudioFormatReaderSource> soundFileSource[2];
    /// The files we're playing.
    File files[2];

    /// If we're currently playing or not.
    bool playing;
    /// Whether or not we're syncing to the main transport.
    bool syncToMainTransport;
    /// The time signature numerator.
    int numerator;
    /// The time signature denominator.
    int denominator;

    /// Default click sound.
    float sineX0;
    /// Default click sound.
    float sineX1;
    /// Default click sound.
    float sineCoeff;
    /// The amplitude envelope for the default click.
    float sineEnv;

    /// Used to count down to the next click.
    float clickCount;
    /// Used to decrement clickCount.
    float clickDec;
    /// Used to count down to the next start of the measure.
    int measureCount;
    /// Whether we're currently playing the accent or the click.
    bool isAccent;

    /// The editor's bounds.
    Rectangle<int> editorBounds;
};

#endif
