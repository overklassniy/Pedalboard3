// LooperProcessor.h - A basic looper processor.
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

#ifndef LOOPERPROCESSOR_H_
#define LOOPERPROCESSOR_H_

#include "PedalboardProcessor.h"

class LooperControl;

/// A basic looper processor.
///
/// How this works:
///
/// When the user hits record, the input audio is saved to disk via an
/// AudioFormatWriter, and stored in memory for playback. While this is
/// happening the input audio is also streamed to the output.
///
/// Because the looper is intended to handle any length of loop, the memory
/// buffer is a little complicated:
///
/// To avoid reallocating memory in the audio thread when the buffer needs to
/// be resized we actually store an array of fixed-size buffers (8 seconds long
/// at 44.1k samplerate; that's 4 bars at 4/4 120bpm). We have a separate thread
/// which monitors how close we are to the end of the current buffer, and
/// allocates a new buffer if we are getting close to the end. Then when the
/// audio thread gets to the end of the current buffer it can just start
/// writing to the next buffer which should be allocated and ready for it.
class LooperProcessor : public PedalboardProcessor,
                        public ChangeListener,
                        public ChangeBroadcaster,
                        public TimeSliceClient,
                        public AsyncUpdater {
  public:
    LooperProcessor();
    ~LooperProcessor() override;

    /// Sets the sound file to play.
    ///
    /// @param phil The sound file to load; stops any active recording first.
    void setFile(const File& phil);
    /// Returns the sound file.
    const File& getFile() {
        newFileLoaded = false;
        return soundFile;
    }
    /// Returns whether or not we're currently playing.
    bool isPlaying() const { return (playing && !stopPlaying); }
    /// Returns whether or not we're currently recording.
    bool isRecording() const { return (recording && !stopRecording); }
    /// Returns the current read position within the file (0->1).
    ///
    /// @return The playback position as a normalized value from 0.0 to 1.0.
    double getReadPosition() const;

    /// Returns true if we've just loaded a new sound file.
    ///
    /// This is necessary so that any ChangeListeners (e.g. LooperControl) can
    /// update their elements accordingly.
    bool getNewFileLoaded() const { return newFileLoaded; }

    /// Used to start recording.
    ///
    /// This is necessary because recording may be started via a MidiMessage,
    /// which will happen in the audio thread. If we do our setup in the audio
    /// thread we get glitches in the audio (from file deletion, memory
    /// allocation etc.), so we need to make sure recording is only ever started
    /// from the main thread. Hence the use of an AsyncUpdater.
    void handleAsyncUpdate() override;
    /// Used to allocate new loop buffers if necessary.
    ///
    /// @return The time in milliseconds to wait before the next call.
    int useTimeSlice() override;

    /// Returns the component which is added to the instance's PluginComponent.
    ///
    /// @return A new LooperControl component; deleted by the caller.
    Component* getControls();
    /// Returns the size of the controls component.
    Point<int> getSize() override { return Point<int>(300, 100); }

    /// Updates the bounds of our editor window.
    ///
    /// @param bounds The new editor window bounds to store.
    void updateEditorBounds(const Rectangle<int>& bounds);

    /// So we can listen to the main transport.
    ///
    /// @param source The change broadcaster that triggered the callback.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Provides a description of the processor to the filter graph.
    ///
    /// @param description The plugin description to fill in.
    void fillInPluginDescription(PluginDescription& description) const override;

    /// Alters the input audio's level accordingly.
    ///
    /// @param buffer The audio buffer to process (record, play, and mix).
    /// @param midiMessages The MIDI buffer (unused).
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    /// Ignored.
    ///
    /// @param sampleRate The current sample rate in Hz.
    /// @param estimatedSamplesPerBlock The maximum number of samples per block.
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override;

    /// Parameter constants.
    enum {
        Play = 0,
        ReturnToZero,
        Record,
        ReadPosition,
        SyncToMainTransport,
        StopAfterBar,
        AutoPlay,
        BarNumerator,
        BarDenominator,
        InputLevel,
        LoopLevel,

        NumParameters
    };

    /// Returns the name of the processor.
    const String getName() const override { return "Looper"; }
    /// Ignored.
    void releaseResources() override {}
    /// Returns the length of the plugin's tail.
    double getTailLengthSeconds() const override { return 0.0; }
    /// Does not accept MIDI input.
    bool acceptsMidi() const override { return false; }
    /// Does not produce MIDI output.
    bool producesMidi() const override { return false; }
    /// Creates the full editor window.
    AudioProcessorEditor* createEditor() override;
    /// Returns true; a full editor is available.
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
    /// Loads the position of the slider and the size and position of the editor.
    ///
    /// @param destData The memory block to serialize state into.
    void getStateInformation(juce::MemoryBlock& destData) override;
    /// Saves the position of the slider and the size and position of the editor.
    ///
    /// @param data Pointer to the serialized state data.
    /// @param sizeInBytes Size of the serialized state data in bytes.
    void setStateInformation(const void* data, int sizeInBytes) override;

  private:
    /// Helper method. Copies the contents of tempBuffer into fadeInBuffer.
    void fillFadeInBuffer();
    /// Helper method. Copies the contents of tempBuffer into fadeOutBuffer.
    void fillFadeOutBuffer();

    /// The size of each memory buffer we use.
    enum { LoopBufferSize = (44100 * 8), FadeBufferSize = 128 };

    /// The file we're playing.
    File soundFile;
    /// If we're currently playing or not.
    bool playing;
    /// Safeguard in case the user tries to record a new loop while we're playing.
    bool stopPlaying;
    /// If we're currently recording or not.
    bool recording;
    /// Safeguard in case the user tries to change the file while we're recording.
    bool stopRecording;
    /// Whether or not we're syncing to the main transport.
    bool syncToMainTransport;
    /// Whether or not recording should stop after a bar.
    bool stopAfterBar;
    /// True if playback should start immediately after recording has stopped.
    bool autoPlay;
    /// The output level of the looper's input.
    float inputLevel;
    /// The output level of the looper's loop.
    float loopLevel;

    /// Used to record the audio input.
    AudioFormatWriter::ThreadedWriter* threadWriter;

    /// The thumbnail image which gets passed to the Controls and Editor.
    AudioThumbnail thumbnail;

    /// The editor's bounds.
    Rectangle<int> editorBounds;

    /// The time signature numerator.
    int numerator;
    /// The time signature denominator.
    int denominator;

    /// Used to count down to the next click.
    float clickCount;
    /// Used to decrement clickCount.
    float clickDec;
    /// Used to count down to the next start of the measure.
    int measureCount;

    /// The samplerate passed to prepareToPlay().
    double currentRate;
    /// Used to ensure we don't reset the play position to zero when the user clicks pause.
    bool justPaused;

    /// The length of the loop in samples.
    uint64_t loopLength;
    /// The loop buffer in memory.
    OwnedArray<juce::AudioBuffer<float>> loopBuffer;
    /// Our playback position in the loop buffer.
    int loopPos;
    /// Which loop buffer we're currently playing back.
    int loopIndex;
    /// Used to ensure the last loop buffer is deleted.
    ///
    /// This is necessary for when we stop recording after a new loop buffer
    /// has been created, but before we've actually started writing into it.
    bool deleteLastBuffer;

    /// The temporary buffer which is copied into the two fade buffers.
    float tempBuffer[2][FadeBufferSize];
    /// The current write position into tempBuffer.
    int tempBufferWrite;
    /// The fade-in buffer (for seamless looping).
    float fadeInBuffer[2][FadeBufferSize];
    /// The fade-out buffer (for seamless looping).
    float fadeOutBuffer[2][FadeBufferSize];
    /// Used to determine when the fade-out buffer can be filled from the temp buffer.
    int fadeOutCount;
    /// Used to determine when to start the fade-in at the end.
    uint64_t fadeInCount;
    /// Used to fade in the loop buffer after recording stops (and autoPlay is on).
    float autoPlayFade;

    /// Used to read a file into the loopBuffers.
    AudioFormatReader* fileReader;
    /// Used to read a file into the loopBuffers.
    int64 fileReaderPos;
    /// Which loopBuffer we are writing to next.
    int fileReaderBufIndex;

    /// Used to let our ChangeListeners know that we've loaded a new file.
    bool newFileLoaded;

    /// The input audio data to processBlock().
    juce::AudioBuffer<float> inputAudio;
};

#endif
