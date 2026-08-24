// SafetyLimiter.h - Audio safety protection processor.
//
// This file is part of Pedalboard3, an audio plugin host.
// Ported from the Pedalboard3-VST3 fork by Project12x.
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

#ifndef SAFETYLIMITER_H_INCLUDED
#define SAFETYLIMITER_H_INCLUDED

#include "VuMeterDsp.h"

#include <JuceHeader.h>
#include <atomic>

/// SafetyLimiterProcessor
///
/// Final output protection that:
/// - Soft-limits peaks above -0.5 dBFS
/// - Auto-mutes on sustained dangerous levels (+6 dBFS for 100ms)
/// - Auto-mutes on DC offset (>0.5 for 500ms)
/// - Auto-mutes on sustained ultrasonic content (>18kHz)
/// - Requires manual unmute via Panic command
class SafetyLimiterProcessor : public AudioProcessor {
  public:
    /// Constructs the safety limiter with stereo input and output buses.
    SafetyLimiterProcessor();
    /// Defaulted destructor.
    ~SafetyLimiterProcessor() override = default;

    /// Calculates timing thresholds and decay coefficients for the given sample rate.
    ///
    /// @param sampleRate The current sample rate in Hz.
    /// @param samplesPerBlock The maximum number of samples per block (unused).
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    /// No resources to release.
    void releaseResources() override;
    /// Applies soft-knee limiting, DC blocking, and auto-mute detection to the buffer.
    ///
    /// @param buffer The audio buffer to process in place.
    /// @param midiMessages The MIDI buffer (unused).
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;

    /// Returns the processor's display name.
    const String getName() const override { return "SafetyLimiter"; }
    /// This processor does not accept MIDI.
    bool acceptsMidi() const override { return false; }
    /// This processor does not produce MIDI.
    bool producesMidi() const override { return false; }
    /// Returns the tail length in seconds (always zero).
    double getTailLengthSeconds() const override { return 0.0; }

    /// Returns 1 (a single default program).
    int getNumPrograms() override { return 1; }
    /// Returns the current program index (always 0).
    int getCurrentProgram() override { return 0; }
    /// No-op; only one program exists.
    void setCurrentProgram(int) override {}
    /// Returns an empty program name.
    const String getProgramName(int) override { return {}; }
    /// No-op; program names cannot be changed.
    void changeProgramName(int, const String&) override {}

    /// No state to serialize; no-op.
    void getStateInformation(MemoryBlock&) override {}
    /// No state to restore; no-op.
    void setStateInformation(const void*, int) override {}

    /// This processor has no editor.
    bool hasEditor() const override { return false; }
    /// Returns nullptr; this processor has no editor.
    AudioProcessorEditor* createEditor() override { return nullptr; }

    /// Safety state queries (thread-safe).
    bool isMuted() const { return muted.load(); }
    bool isLimiting() const { return limiting.load(); }

    /// Manual unmute (called from Panic).
    void unmute() { muted.store(false); }

    /// Check if mute was triggered since last check (for toast notification).
    ///
    /// @return True if a mute was triggered since the last call, false otherwise.
    bool checkAndClearMuteTriggered() {
        bool expected = true;
        return muteTriggered.compare_exchange_strong(expected, false);
    }

    /// Audio activity detection for wire glow.
    bool isAudioActive() const { return audioActive.load(); }

    /// Output level metering (peak with decay, read by UI for Audio Output VU).
    ///
    /// @param channel The channel index to read (0 or 1).
    /// @return The current output peak level for the given channel.
    float getOutputLevel(int channel) const {
        if (channel >= 0 && channel < 2)
            return outputLevels[channel].load(std::memory_order_relaxed);
        return 0.0f;
    }

    /// Input level metering (peak with decay, read by UI for Audio Input VU).
    ///
    /// @param channel The channel index to read (0 or 1).
    /// @return The current input peak level for the given channel.
    float getInputLevel(int channel) const {
        if (channel >= 0 && channel < 2)
            return inputLevels[channel].load(std::memory_order_relaxed);
        return 0.0f;
    }

    /// VU-ballistic level (300ms integration, read by UI for VU meter display).
    ///
    /// @param channel The channel index to read (0 or 1).
    /// @return The current output VU level for the given channel.
    float getOutputVuLevel(int channel) const {
        if (channel >= 0 && channel < 2)
            return outputVuLevels[channel].load(std::memory_order_relaxed);
        return 0.0f;
    }
    /// VU-ballistic input level (300ms integration, read by UI for VU meter display).
    ///
    /// @param channel The channel index to read (0 or 1).
    /// @return The current input VU level for the given channel.
    float getInputVuLevel(int channel) const {
        if (channel >= 0 && channel < 2)
            return inputVuLevels[channel].load(std::memory_order_relaxed);
        return 0.0f;
    }

    /// Called from MeteringProcessorPlayer after graph processes (RT-safe).
    ///
    /// @param outputData Array of per-channel output sample buffers.
    /// @param numChannels Number of channels in the output data.
    /// @param numSamples Number of samples per channel.
    void updateOutputLevelsFromDevice(const float* const* outputData, int numChannels, int numSamples);
    /// Called from MeteringProcessorPlayer before graph processes (RT-safe).
    ///
    /// @param inputData Array of per-channel input sample buffers.
    /// @param numChannels Number of channels in the input data.
    /// @param numSamples Number of samples per channel.
    void updateInputLevelsFromDevice(const float* const* inputData, int numChannels, int numSamples);

    /// Returns the singleton instance used by PluginComponent to read output levels.
    static SafetyLimiterProcessor* getInstance() { return instance; }
    /// Sets the singleton instance; called from prepareToPlay.
    static void setInstance(SafetyLimiterProcessor* inst) { instance = inst; }

  private:
    // Thresholds
    static constexpr float softLimitThreshold = 0.944f;   // -0.5 dBFS
    static constexpr float dangerousGainThreshold = 2.0f; // +6 dBFS
    static constexpr float dcOffsetThreshold = 0.5f;

    // Timing (in samples, set in prepareToPlay)
    int dangerousGainHoldSamples = 0; // 100ms
    int dcOffsetHoldSamples = 0;      // 500ms
    int ultrasonicHoldSamples = 0;    // 200ms

    // State
    std::atomic<bool> muted{false};
    std::atomic<bool> limiting{false};
    std::atomic<bool> muteTriggered{false};
    std::atomic<bool> audioActive{false}; // Set when audio is flowing

    // Detection counters
    int dangerousGainCounter = 0;
    int dcOffsetCounter = 0;
    int ultrasonicCounter = 0;

    // DC blocker state (per channel)
    float dcBlockerState[2] = {0.0f, 0.0f};
    float dcBlockerCoeff = 0.995f;

    // Ultrasonic detection (simple high-pass energy tracker)
    float ultrasonicEnergy = 0.0f;
    float ultrasonicDecay = 0.99f;

    // Limiter state
    float currentGain = 1.0f;
    float releaseCoeff = 0.0f; // Calculated in prepareToPlay

    double currentSampleRate = 44100.0;

    // Level metering (per-channel peak with decay, updated from device callback)
    std::atomic<float> outputLevels[2] = {{0.0f}, {0.0f}};
    std::atomic<float> inputLevels[2] = {{0.0f}, {0.0f}};
    float outputDecayCoeff = 0.9995f; // ~300ms decay at 44100Hz, refined in prepareToPlay

    // VU meter DSP (2-pole lowpass, 300ms integration per IEC 60268-17)
    VuMeterDsp inputVu[2];
    VuMeterDsp outputVu[2];
    std::atomic<float> inputVuLevels[2] = {{0.0f}, {0.0f}};
    std::atomic<float> outputVuLevels[2] = {{0.0f}, {0.0f}};

    static SafetyLimiterProcessor* instance;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SafetyLimiterProcessor)
};

#endif // SAFETYLIMITER_H_INCLUDED
