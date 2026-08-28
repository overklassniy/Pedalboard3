// CrossfadeMixer.h - Glitch-free patch switching crossfade mixer.
//
// This file is part of Pedalboard3, an audio plugin host.
// Ported and modified from the Pedalboard3 fork by Project12x.
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

#pragma once

#include <JuceHeader.h>
#include <atomic>

/// CrossfadeMixerProcessor provides smooth audio crossfading during patch switching.
///
/// When a patch change is triggered:
/// 1. startFadeOut() is called - audio fades to silence over fadeMs
/// 2. Patch is loaded while audio is silent
/// 3. startFadeIn() is called - audio fades back in over fadeMs
///
/// All operations are audio-thread safe using atomics.
class CrossfadeMixerProcessor : public AudioProcessor {
  public:
    /// Constructs the mixer with stereo input and output buses.
    CrossfadeMixerProcessor();
    ~CrossfadeMixerProcessor() override = default;

    // Crossfade control (call from message thread)

    /// Start fading audio out. Call before clearing the graph.
    ///
    /// @param durationMs Fade duration in milliseconds; defaults to 100 if <= 0.
    void startFadeOut(int durationMs = 100);

    /// Start fading audio in. Call after loading the new patch.
    ///
    /// @param durationMs Fade duration in milliseconds; defaults to 100 if <= 0.
    void startFadeIn(int durationMs = 100);

    /// Returns true if currently fading (out or in).
    bool isFading() const { return fading.load(); }

    /// Returns true if currently faded out (silent).
    bool isSilent() const { return fadeGain.load() < 0.001f; }

    /// Sets the default fade duration in milliseconds.
    void setDefaultFadeDuration(int ms) { defaultFadeMs = ms; }
    /// Returns the default fade duration in milliseconds.
    int getDefaultFadeDuration() const { return defaultFadeMs; }

    // AudioProcessor implementation

    /// Stores the sample rate for fade duration calculations.
    ///
    /// @param sampleRate The current sample rate in Hz.
    /// @param samplesPerBlock The maximum number of samples per block (unused).
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    /// Applies the current fade gain ramp to the audio buffer.
    ///
    /// @param buffer The audio buffer to apply the fade gain to.
    /// @param midi The MIDI buffer (unused).
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi) override;

    // AudioProcessor boilerplate

    const String getName() const override { return "Crossfade Mixer"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const String getProgramName(int) override { return {}; }
    void changeProgramName(int, const String&) override {}

    void getStateInformation(MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

  private:
    // Fade state (all atomic for audio thread safety)

    /// Whether a fade is currently in progress.
    std::atomic<bool> fading{false};

    /// Direction of the current fade: true for fade out, false for fade in.
    std::atomic<bool> fadingOut{false};

    /// Current gain applied to the output, ranging from 0.0 to 1.0.
    std::atomic<float> fadeGain{1.0f};

    /// Per-sample gain change used during the fade ramp.
    std::atomic<float> fadeIncrement{0.0f};

    double currentSampleRate = 44100.0;
    int defaultFadeMs = 100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrossfadeMixerProcessor)
};
