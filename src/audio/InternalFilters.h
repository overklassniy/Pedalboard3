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
class InternalPluginFormat : public AudioPluginFormat {
  public:
    /// Constructor. Initializes plugin descriptions for all internal filter types.
    InternalPluginFormat();
    ~InternalPluginFormat() override = default;

    /// Types of internal filters available.
    enum InternalFilterType {
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
    ///
    /// @param type The internal filter type to look up.
    /// @return The plugin description for the given type, or nullptr if unknown.
    const PluginDescription* getDescriptionFor(const InternalFilterType type);

    /// Populates the array with descriptions for all available internal filters.
    ///
    /// @param results The array to fill with plugin descriptions.
    void getAllTypes(OwnedArray<PluginDescription>& results);

    /// Returns false; internal plugins are not discovered by scanning.
    bool canScanForPlugins() const override { return false; }

    // AudioPluginFormat overrides
    /// Returns the format name ("Internal").
    String getName() const override { return "Internal"; }
    /// Returns true; any file could nominally contain an internal plugin reference.
    bool fileMightContainThisPluginType(const String&) override { return true; }
    /// Returns an empty search path; internal plugins have no file locations.
    FileSearchPath getDefaultLocationsToSearch() override { return FileSearchPath(); }
    /// No-op; internal plugins are not file-based.
    void findAllTypesForFile(OwnedArray<PluginDescription>&, const String&) override {}
    /// Returns "Internal" for all identifiers.
    String getNameOfPluginFromIdentifier(const String&) override { return "Internal"; }
    /// Returns true; internal plugins always exist.
    bool doesPluginStillExist(const PluginDescription&) override { return true; }
    /// Returns true; scanning is trivial for built-in plugins.
    bool isTrivialToScan() const override { return true; }
    /// Returns false; internal plugins never need rescanning.
    bool pluginNeedsRescanning(const PluginDescription&) override { return false; }
    /// Returns an empty list; there are no file paths to search.
    StringArray searchPathsForPlugins(const FileSearchPath&, bool, bool) override { return {}; }
    /// Returns false; internal plugins can be created on any thread.
    bool requiresUnblockedMessageThreadDuringCreation(const PluginDescription&) const override { return false; }

  protected:
    /// Creates an instance of the internal plugin matching the given description.
    ///
    /// @param desc The plugin description to match against internal types.
    /// @param initialSampleRate The sample rate for the new instance (unused).
    /// @param initialBufferSize The buffer size for the new instance (unused).
    /// @param callback The callback to invoke with the created instance or error.
    void createPluginInstance(const PluginDescription& desc, double initialSampleRate, int initialBufferSize,
                              PluginCreationCallback callback) override;

  private:
    /// Plugin description for the audio input I/O processor.
    PluginDescription audioInDesc;
    /// Plugin description for the audio output I/O processor.
    PluginDescription audioOutDesc;
    /// Plugin description for the MIDI input I/O processor.
    PluginDescription midiInDesc;
    /// Plugin description for the OSC input processor.
    PluginDescription oscInputDesc;
};

#endif
