// BypassableInstance.h - Wrapper providing bypass for AudioPluginInstance.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
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

#ifndef BYPASSABLEINSTANCE_H_
#define BYPASSABLEINSTANCE_H_

#include <JuceHeader.h>
#include <atomic>

/// Wrapper class providing bypass functionality for AudioPluginInstance.
///
/// Wraps an AudioPluginInstance, adding a smooth bypass ramp and per-plugin
/// MIDI channel filtering. The bypass state is atomic for cross-thread safety
/// (set from UI, read from audio thread).
class BypassableInstance : public AudioPluginInstance
{
  public:
    /// Constructor. Takes ownership of the wrapped plugin.
    explicit BypassableInstance(std::unique_ptr<AudioPluginInstance> plug);
    /// Destructor.
    ~BypassableInstance() override;

    /// Sets up the internal buffer.
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override;
    /// Clears the internal buffer.
    void releaseResources() override;
    /// Handles the audio with bypass ramping.
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    /// Sets the bypass state.
    void setBypass(bool val);
    /// Returns the bypass state.
    bool getBypass() const { return bypass.load(); }

    /// Sets the MIDI channel the plugin responds to (0 = omni).
    void setMIDIChannel(int val);
    /// Returns the plugin's MIDI channel (0 = omni).
    int getMIDIChannel() const { return midiChannel.load(); }
    /// Passes a MIDI message to the plugin from the OSC input.
    void addMidiMessage(const juce::MidiMessage& message);

    /// Returns the wrapped plugin instance.
    AudioPluginInstance* getPlugin() const { return plugin.get(); }

    /// Returns the plugin's name.
    const String getName() const override { return plugin->getName(); }

    // JUCE 8 bus layout support - delegate to inner plugin
    bool isBusesLayoutSupported(const BusesLayout& layout) const override
    {
        return plugin->checkBusesLayoutSupported(layout);
    }

    /// Returns the length of the plugin's tail.
    double getTailLengthSeconds() const override { return plugin->getTailLengthSeconds(); }
    /// Returns true if the plugin wants MIDI input.
    bool acceptsMidi() const override { return plugin->acceptsMidi(); }
    /// Returns true if the plugin produces MIDI output.
    bool producesMidi() const override { return plugin->producesMidi(); }
    /// Can be used to reset the plugin.
    void reset() override { plugin->reset(); }

    // JUCE 8: createEditor() is private, use createEditorAndMakeActive()
    /// Creates the plugin's editor.
    AudioProcessorEditor* createEditor() override { return plugin->createEditorAndMakeActive(); }
    /// Returns true if the plugin has an editor.
    bool hasEditor() const override { return plugin->hasEditor(); }

    // JUCE 8 parameter access via AudioProcessorParameter array
    /// Returns the number of parameters the plugin has.
    int getNumPluginParameters() const { return plugin->getParameters().size(); }
    /// Returns the indexed parameter object.
    AudioProcessorParameter* getPluginParameter(int index) const
    {
        auto& params = plugin->getParameters();
        return (index >= 0 && index < params.size()) ? params[index] : nullptr;
    }
    /// Returns the indexed parameter's name.
    String getPluginParameterName(int parameterIndex) const
    {
        if (auto* param = getPluginParameter(parameterIndex))
            return param->getName(128);
        return {};
    }
    /// Returns the indexed parameter's value (0-1 normalized).
    float getPluginParameterValue(int parameterIndex) const
    {
        if (auto* param = getPluginParameter(parameterIndex))
            return param->getValue();
        return 0.0f;
    }
    /// Returns the indexed parameter's value as a string.
    String getPluginParameterText(int parameterIndex) const
    {
        if (auto* param = getPluginParameter(parameterIndex))
            return param->getCurrentValueAsText();
        return {};
    }
    /// Sets the indexed parameter value (0-1 normalized).
    void setPluginParameterValue(int parameterIndex, float newValue)
    {
        if (auto* param = getPluginParameter(parameterIndex))
            param->setValue(newValue);
    }
    /// Returns true if the indexed parameter is automatable.
    bool isPluginParameterAutomatable(int parameterIndex) const
    {
        if (auto* param = getPluginParameter(parameterIndex))
            return param->isAutomatable();
        return false;
    }
    /// Returns true if the indexed parameter is a meta parameter.
    bool isPluginMetaParameter(int parameterIndex) const
    {
        if (auto* param = getPluginParameter(parameterIndex))
            return param->isMetaParameter();
        return false;
    }

    /// Returns the number of programs the plugin has.
    int getNumPrograms() override { return plugin->getNumPrograms(); }
    /// Returns the index of the current program.
    int getCurrentProgram() override { return plugin->getCurrentProgram(); }
    /// Sets the current program.
    void setCurrentProgram(int index) override { plugin->setCurrentProgram(index); }
    /// Returns the indexed program's name.
    const String getProgramName(int index) override { return plugin->getProgramName(index); }
    /// Changes the indexed program's name.
    void changeProgramName(int index, const String& newName) override { plugin->changeProgramName(index, newName); }

    /// Saves the plugin's internal state.
    void getStateInformation(juce::MemoryBlock& destData) override { plugin->getStateInformation(destData); }
    /// Saves the state of the current program.
    void getCurrentProgramStateInformation(juce::MemoryBlock& destData) override
    {
        plugin->getCurrentProgramStateInformation(destData);
    }
    /// Restores the plugin's internal state.
    void setStateInformation(const void* data, int sizeInBytes) override
    {
        plugin->setStateInformation(data, sizeInBytes);
    }
    /// Sets the state of the current program.
    void setCurrentProgramStateInformation(const void* data, int sizeInBytes) override
    {
        plugin->setCurrentProgramStateInformation(data, sizeInBytes);
    }
    /// Fills in the plugin's description.
    void fillInPluginDescription(PluginDescription& description) const override
    {
        plugin->fillInPluginDescription(description);
    }

  private:
    /// The plugin instance we're wrapping (owned).
    std::unique_ptr<AudioPluginInstance> plugin;

    /// Buffer used to store the original audio for bypass ramping.
    juce::AudioBuffer<float> tempBuffer;

    /// Whether we are currently bypassing (set from UI, read from audio thread).
    std::atomic<bool> bypass{false};
    /// Used to ramp the bypass audio.
    float bypassRamp;

    /// MIDI channel the plugin responds to (0 = omni, set from UI, read from audio thread).
    std::atomic<int> midiChannel{0};
    /// Used to pass OSC MIDI messages to the plugin.
    juce::MidiMessageCollector midiCollector;
};

#endif
