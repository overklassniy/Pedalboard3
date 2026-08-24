// LevelProcessor.h - Simple processor which provides a level control.
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

#ifndef LEVELPROCESSOR_H_
#define LEVELPROCESSOR_H_

#include "PedalboardProcessor.h"

/// Simple processor which provides a level control.
class LevelProcessor : public PedalboardProcessor {
  public:
    /// Constructs a level processor with the level set to 0.5.
    LevelProcessor();
    ~LevelProcessor() override;

    /// Returns the component which is added to the instance's PluginComponent.
    ///
    /// @return A new LevelControl component; deleted by the caller.
    Component* getControls();
    /// Returns the size of the controls component.
    Point<int> getSize() override { return Point<int>(64, 64); }

    /// Updates the bounds of our editor window.
    ///
    /// @param bounds The new editor window bounds to store.
    void updateEditorBounds(const Rectangle<int>& bounds);

    /// Provides a description of the processor to the filter graph.
    ///
    /// @param description The plugin description to fill in.
    void fillInPluginDescription(PluginDescription& description) const override;

    /// Alters the input audio's level accordingly.
    ///
    /// @param buffer The audio buffer to apply the level change to.
    /// @param midiMessages The MIDI buffer (unused).
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    /// Returns the name of the processor.
    const String getName() const override { return "Level"; }
    /// Ignored.
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override {}
    /// Ignored.
    void releaseResources() override {}
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
    /// Returns the parameter value (0-1 normalized).
    ///
    /// @param parameterIndex The index of the parameter (unused; only parameter 0 exists).
    /// @return The current level value, normalized to 0-1.
    float getParameter(int parameterIndex) { return level; }
    /// Sets the parameter value (0-1 normalized).
    ///
    /// @param parameterIndex The index of the parameter (unused; only parameter 0 exists).
    /// @param newValue The new level value, normalized to 0-1.
    void setParameter(int parameterIndex, float newValue);
    /// Returns the parameter's value as a string.
    ///
    /// @param parameterIndex The index of the parameter (unused; only parameter 0 exists).
    /// @return A textual representation of the current level value.
    const String getParameterText(int parameterIndex);

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
    /// Saves the level value and editor bounds to the block.
    ///
    /// @param destData The memory block to serialize state into.
    void getStateInformation(juce::MemoryBlock& destData) override;
    /// Restores the level value and editor bounds from the data.
    ///
    /// @param data Pointer to the serialized state data.
    /// @param sizeInBytes Size of the serialized state data in bytes.
    void setStateInformation(const void* data, int sizeInBytes) override;

  private:
    /// The level parameter.
    float level;

    /// The editor's bounds.
    Rectangle<int> editorBounds;
};

#endif
