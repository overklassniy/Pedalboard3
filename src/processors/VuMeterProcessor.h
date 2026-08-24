// VuMeterProcessor.h - Simple processor which provides a VU meter.
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

#ifndef VUMETERPROCESSOR_H_
#define VUMETERPROCESSOR_H_

#include "PedalboardProcessor.h"

/// Simple processor which provides a VU meter.
class VuMeterProcessor : public PedalboardProcessor {
  public:
    /// Constructs the VU meter processor with 2 inputs and 0 outputs.
    VuMeterProcessor();
    /// Destructor.
    ~VuMeterProcessor() override;

    /// Returns the component which is added to the instance's PluginComponent.
    ///
    /// @return A new VuMeterControl component; deleted by the caller.
    Component* getControls();
    /// Returns the size of the controls component.
    Point<int> getSize() override { return Point<int>(64, 128); }

    /// Returns the current left channel level (linear, 0.0 to 1.0).
    float getLeftLevel() const { return levelLeft; }
    /// Returns the current right channel level (linear, 0.0 to 1.0).
    float getRightLevel() const { return levelRight; }

    /// Updates the bounds of our editor window.
    ///
    /// @param bounds The new editor window bounds to store.
    void updateEditorBounds(const Rectangle<int>& bounds);

    /// Provides a description of the processor to the filter graph.
    ///
    /// @param description The plugin description to fill in.
    void fillInPluginDescription(PluginDescription& description) const override;

    /// Tracks peak levels per channel with gradual decay; audio passes through unmodified.
    ///
    /// @param buffer The audio buffer to measure peak levels from.
    /// @param midiMessages The MIDI buffer (unused).
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    /// Returns the name of the processor.
    const String getName() const override { return "VU Meter"; }
    /// No preparation needed.
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override {}
    /// No resources to release.
    void releaseResources() override {}
    /// Returns the length of the plugin's tail.
    double getTailLengthSeconds() const override { return 0.0; }
    /// This processor does not accept MIDI.
    bool acceptsMidi() const override { return false; }
    /// This processor does not produce MIDI.
    bool producesMidi() const override { return false; }
    /// Creates the VuMeterEditor for this processor.
    AudioProcessorEditor* createEditor() override;
    /// This processor has a custom editor.
    bool hasEditor() const override { return true; }

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
    /// Serializes the editor bounds to destData.
    ///
    /// @param destData The memory block to serialize state into.
    void getStateInformation(juce::MemoryBlock& destData) override;
    /// Restores the editor bounds from data.
    ///
    /// @param data Pointer to the serialized state data.
    /// @param sizeInBytes Size of the serialized state data in bytes.
    void setStateInformation(const void* data, int sizeInBytes) override;

  private:
    /// Current left channel peak level with gradual decay.
    float levelLeft;
    /// Current right channel peak level with gradual decay.
    float levelRight;

    /// The editor's bounds.
    Rectangle<int> editorBounds;
};

#endif
