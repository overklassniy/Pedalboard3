// MidiMappingManager.h - Dispatches MIDI CC messages to MidiMappings.
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

#ifndef MIDIMAPPINGMANAGER_H_
#define MIDIMAPPINGMANAGER_H_

#include "Mapping.h"
#include "TapTempoHelper.h"

#include <JuceHeader.h>
#include <map>

class MidiMappingManager;

/// Represents a mapping between MIDI CC and a plugin parameter.
class MidiMapping : public Mapping
{
  public:
    ///
    /// manager is the MidiMappingManager to register with.
    /// graph is the FilterGraph this Mapping exists in.
    /// pluginId is the uid of the plugin whose parameter is being mapped.
    /// param is the plugin parameter which is being mapped to.
    /// midiCc is the MIDI CC being mapped to the parameter.
    /// latch determines whether the CC should be latched.
    /// chan is the MIDI channel to respond to (0 means omni).
    /// lower is the lower bound of the plugin parameter mapping.
    /// upper is the upper bound of the plugin parameter mapping.
    MidiMapping(MidiMappingManager* manager,
                FilterGraph* graph,
                uint32 pluginId,
                int param,
                int midiCc,
                bool latch,
                int chan = 0,
                float lower = 0.0f,
                float upper = 1.0f);
    /// Constructor to load Mapping parameters from an XmlElement.
    MidiMapping(MidiMappingManager* manager, FilterGraph* graph, XmlElement* e);
    ~MidiMapping() override;

    /// Called when a MIDI CC message matching this mapping's CC is received.
    void ccReceived(int val);

    /// Returns an XmlElement representing this Mapping.
    XmlElement* getXml() const override;

    /// Returns the MIDI CC this mapping applies to.
    int getCc() const { return cc; }
    /// Returns whether the CC should be latched.
    bool getLatched() const { return latched; }
    /// Returns the MIDI channel the mapping applies to.
    int getChannel() const { return channel; }
    /// Returns the lower bound of the parameter mapping.
    float getLowerBound() const { return lowerBound; }
    /// Returns the upper bound of the parameter mapping.
    float getUpperBound() const { return upperBound; }

    /// Sets the mapping's MIDI CC.
    void setCc(int val);
    /// Sets the mapping's latch value.
    void setLatched(bool val);
    /// Sets the MIDI channel the mapping applies to.
    void setChannel(int val);
    /// Sets the mapping's lower bound.
    void setLowerBound(float val);
    /// Sets the mapping's upper bound.
    void setUpperBound(float val);

  private:
    /// The MidiMappingManager to register with.
    MidiMappingManager* mappingManager;

    /// The CC this mapping applies to.
    int cc;
    /// If the CC should be latched (useful for buttons).
    bool latched;
    /// The MIDI channel this mapping applies to.
    int channel;
    /// The lower bound of the plugin parameter mapping.
    float lowerBound;
    /// The upper bound of the plugin parameter mapping.
    float upperBound;

    /// The currently-latched value.
    float latchVal;
    /// The current high latched value.
    float latchHi;
    /// The current low latched value.
    float latchLo;
    /// Whether to toggle the latch.
    bool latchToggle;
};

/// Represents a mapping between MIDI CC and an ApplicationCommandTarget.
class MidiAppMapping
{
  public:
    ///
    /// manager is the MidiMappingManager to register with.
    /// midiCc is the MIDI CC being mapped to the command.
    /// commandId is the ID of the ApplicationCommandTarget to invoke.
    MidiAppMapping(MidiMappingManager* manager, int midiCc, CommandID commandId);
    /// Constructor to load Mapping parameters from an XmlElement.
    MidiAppMapping(MidiMappingManager* manager, XmlElement* e);
    ~MidiAppMapping();

    /// Returns an XmlElement representing this Mapping.
    XmlElement* getXml() const;

    /// Returns the MIDI CC this mapping applies to.
    int getCc() const { return cc; }
    /// Returns the CommandId this mapping applies to.
    CommandID getId() const { return id; }

  private:
    /// The main MidiMappingManager.
    MidiMappingManager* midiManager;

    /// This mapping's MIDI CC.
    int cc;
    /// This mapping's command ID.
    CommandID id;
};

/// Dispatches MIDI CC messages to MidiMappings.
class MidiMappingManager
{
  public:
    MidiMappingManager(ApplicationCommandManager* manager);
    ~MidiMappingManager();

    /// Called when a MIDI CC message is received.
    void midiCcReceived(const MidiMessage& message, double secondsSinceStart);

    /// Registers a MidiMapping with the manager.
    void registerMapping(int midiCc, MidiMapping* mapping);
    /// Unregisters a MidiMapping with the manager.
    void unregisterMapping(MidiMapping* mapping);

    /// Registers a MidiAppMapping with the manager.
    void registerAppMapping(MidiAppMapping* mapping);
    /// Unregisters a MidiAppMapping with the manager.
    void unregisterAppMapping(MidiAppMapping* mapping);

    /// Returns the number of MidiAppMappings.
    int getNumAppMappings() const { return static_cast<int>(appMappings.size()); }
    /// Returns the indexed MidiAppMapping.
    MidiAppMapping* getAppMapping(int index);

    /// Callback used by the MIDI learn functions.
    class MidiLearnCallback
    {
      public:
        virtual ~MidiLearnCallback() {}

        /// Called when the manager receives a MIDI CC message.
        virtual void midiCcReceived(int val) = 0;
    };
    /// Registers a callback for the next received MIDI CC message.
    ///
    /// The callback is automatically unregistered after it is called once.
    void registerMidiLearnCallback(MidiLearnCallback* callback);
    /// Unregisters a midi learn callback.
    ///
    /// Only needed if the callback is deleted before receiving a MIDI CC.
    void unregisterMidiLearnCallback(MidiLearnCallback* callback);

    /// Returns a StringArray with the full range of named MIDI CCs.
    static StringArray getCCNames();

  private:
    /// Holds all the MidiMappings to dispatch messages to, keyed by CC.
    std::multimap<int, MidiMapping*> mappings;

    /// Holds any MIDI CC to ApplicationCommand mappings.
    std::multimap<int, MidiAppMapping*> appMappings;
    /// Holds a copy of the app's ApplicationCommandManager.
    ApplicationCommandManager* appManager;

    /// Used for tap tempo.
    TapTempoHelper tapHelper;

    /// The midi learn callback for the next received MIDI CC message.
    ///
    /// nullptr if there is no callback currently registered.
    MidiLearnCallback* midiLearnCallback;
};

/// Intercepts MIDI messages so they can be sent to the MidiMappingManager.
class MidiInterceptor : public AudioPluginInstance
{
  public:
    MidiInterceptor();
    ~MidiInterceptor() override;

    /// Registers the current MidiMappingManager with this instance.
    void setManager(MidiMappingManager* manager);

    /// Provides a description of the processor to the filter graph.
    void fillInPluginDescription(PluginDescription& description) const override;

    /// Passes any MIDI messages to the MidiMappingManager.
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    /// Returns the name of the processor.
    const String getName() const override { return "Midi Interceptor"; }
    /// Ignored.
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override
    {
        juce::ignoreUnused(sampleRate, estimatedSamplesPerBlock);
    }
    /// Ignored.
    void releaseResources() override {}
    /// Returns true if there is no tail.
    double getTailLengthSeconds() const override { return 0.0; }
    /// We definitely want MIDI input.
    bool acceptsMidi() const override { return true; }
    /// But we don't need to output it.
    bool producesMidi() const override { return false; }
    /// We have no editor.
    AudioProcessorEditor* createEditor() override { return nullptr; }
    /// We have no editor.
    bool hasEditor() const override { return false; }
    /// We have no programs.
    int getNumPrograms() override { return 0; }
    /// We have no programs.
    int getCurrentProgram() override { return 0; }
    /// We have no programs.
    void setCurrentProgram(int index) override { juce::ignoreUnused(index); }
    /// We have no programs.
    const String getProgramName(int index) override { juce::ignoreUnused(index); return {}; }
    /// We have no programs.
    void changeProgramName(int index, const String& newName) override { juce::ignoreUnused(index, newName); }
    /// We have no state information.
    void getStateInformation(juce::MemoryBlock& destData) override { juce::ignoreUnused(destData); }
    /// We have no state information.
    void setStateInformation(const void* data, int sizeInBytes) override { juce::ignoreUnused(data, sizeInBytes); }

    /// No audio buses.
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
        return layouts.getMainInputChannels() == 0 && layouts.getMainOutputChannels() == 0;
    }

  private:
    /// The MidiMappingManager to pass MIDI messages to.
    MidiMappingManager* midiManager;

    /// Used to help calculate tempo.
    int64 samplesSinceStart;
};

#endif
