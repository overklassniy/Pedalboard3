// InternalFilters.h - Internal plugin format for built-in processors.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
//
// Derived from the JUCE audio plugin host example by Raw Material Software.
// Modified by Niall Moody for Pedalboard2, and further modified for Pedalboard3.
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

#ifndef INTERNALFILTERS_H_
#define INTERNALFILTERS_H_

#include <JuceHeader.h>

/// Manages the internal plugin types (built-in processors).
///
/// Provides an AudioPluginFormat for the app's built-in processors:
/// audio input, audio output, MIDI input, and the various internal
/// processors (level, file player, output toggle, VU meter, recorder,
/// metronome, looper). Additional processors will be added in Phase 3.
class InternalPluginFormat : public AudioPluginFormat
{
  public:
    InternalPluginFormat();
    ~InternalPluginFormat() override = default;

    /// Types of internal filters available.
    enum InternalFilterType
    {
        audioInputFilter = 0,
        audioOutputFilter,
        midiInputFilter,
        oscInputFilter,

        // Additional processors will be added in Phase 3:
        // midiInterceptorFilter, levelProcFilter,
        // filePlayerProcFilter, outputToggleProcFilter, vuMeterProcFilter,
        // recorderProcFilter, metronomeProcFilter, looperProcFilter,

        endOfFilterTypes
    };

    /// Returns the plugin description for the given internal filter type.
    const PluginDescription* getDescriptionFor(const InternalFilterType type);

    /// Populates the array with descriptions for all available internal filters.
    void getAllTypes(OwnedArray<PluginDescription>& results);

    bool canScanForPlugins() const override { return false; }

    // AudioPluginFormat overrides
    String getName() const override { return "Internal"; }
    bool fileMightContainThisPluginType(const String&) override { return true; }
    FileSearchPath getDefaultLocationsToSearch() override { return FileSearchPath(); }
    void findAllTypesForFile(OwnedArray<PluginDescription>&, const String&) override {}
    String getNameOfPluginFromIdentifier(const String&) override { return "Internal"; }
    bool doesPluginStillExist(const PluginDescription&) override { return true; }
    bool isTrivialToScan() const override { return true; }
    bool pluginNeedsRescanning(const PluginDescription&) override { return false; }
    StringArray searchPathsForPlugins(const FileSearchPath&, bool, bool) override { return {}; }
    bool requiresUnblockedMessageThreadDuringCreation(const PluginDescription&) const override { return false; }

  protected:
    void createPluginInstance(const PluginDescription& desc, double initialSampleRate, int initialBufferSize,
                              PluginCreationCallback callback) override;

  private:
    PluginDescription audioInDesc;
    PluginDescription audioOutDesc;
    PluginDescription midiInDesc;
    PluginDescription oscInputDesc;
};

#endif
