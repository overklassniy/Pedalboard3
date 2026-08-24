// CrossfadeMixer.cpp - Glitch-free patch switching crossfade mixer.
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

#include "CrossfadeMixer.h"

CrossfadeMixerProcessor::CrossfadeMixerProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", AudioChannelSet::stereo(), true)
                         .withOutput("Output", AudioChannelSet::stereo(), true)) {}

void CrossfadeMixerProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    currentSampleRate = sampleRate;
}

void CrossfadeMixerProcessor::startFadeOut(int durationMs) {
    if (durationMs <= 0)
        durationMs = defaultFadeMs;

    // Convert fade duration from milliseconds to sample count.
    int fadeSamples = static_cast<int>((durationMs / 1000.0) * currentSampleRate);
    if (fadeSamples < 1)
        fadeSamples = 1;

    // Negative increment ramps gain down toward silence.
    fadeIncrement.store(-1.0f / static_cast<float>(fadeSamples));
    fadingOut.store(true);
    fading.store(true);
}

void CrossfadeMixerProcessor::startFadeIn(int durationMs) {
    if (durationMs <= 0)
        durationMs = defaultFadeMs;

    // Convert fade duration from milliseconds to sample count.
    int fadeSamples = static_cast<int>((durationMs / 1000.0) * currentSampleRate);
    if (fadeSamples < 1)
        fadeSamples = 1;

    // Positive increment ramps gain up toward full volume.
    fadeIncrement.store(1.0f / static_cast<float>(fadeSamples));
    fadingOut.store(false);
    fading.store(true);
}

void CrossfadeMixerProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& /*midi*/) {
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // When not fading, apply the stored gain directly and return.
    if (!fading.load()) {
        float gain = fadeGain.load();
        if (gain < 0.999f) {
            // Gain is below unity, so the output is partially attenuated.
            buffer.applyGain(gain);
        }
        // At unity gain the buffer is already at full volume.
        return;
    }

    // Fading: apply a per-sample gain ramp across all channels.
    float currentGain = fadeGain.load();
    float increment = fadeIncrement.load();
    bool isFadingOut = fadingOut.load();

    for (int sample = 0; sample < numSamples; ++sample) {
        for (int channel = 0; channel < numChannels; ++channel) {
            buffer.getWritePointer(channel)[sample] *= currentGain;
        }

        currentGain += increment;

        // Clamp gain and detect fade completion.
        if (isFadingOut) {
            if (currentGain <= 0.0f) {
                currentGain = 0.0f;
                fading.store(false);
                break;
            }
        } else {
            if (currentGain >= 1.0f) {
                currentGain = 1.0f;
                fading.store(false);
                break;
            }
        }
    }

    fadeGain.store(currentGain);
}
