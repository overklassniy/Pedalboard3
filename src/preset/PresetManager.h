// PresetManager.h - A class used to keep track of user-saved plugin presets.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2012 Niall Moody.
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

#ifndef PRESETMANAGER_H_
#define PRESETMANAGER_H_

#include <JuceHeader.h>

/// A class used to keep track of user-saved plugin presets.
class PresetManager {
  public:
    /// Creates the manager and ensures the preset directory exists.
    PresetManager();
    /// Destructor.
    ~PresetManager();

    /// Loads a preset from an .fxp file into the given plugin.
    ///
    /// @param inFile The .fxp file to load the preset from.
    /// @param plugin The plugin processor to apply the preset to.
    void importPreset(const File& inFile, AudioProcessor* plugin);

    /// Loads a named preset from the user-saved presets directory for this
    /// plugin.
    ///
    /// This is a wrapper around the File-based importPreset(); it calculates
    /// the file path from the presetName as:
    /// <Pedalboard3 user data dir>/presets/<plugin name>/<presetName>.fxp
    ///
    /// @param presetName The name of the preset to load.
    /// @param plugin The plugin processor to apply the preset to.
    void importPreset(const String& presetName, AudioProcessor* plugin);

    /// Saves the contents of a MemoryBlock to a .fxp file.
    ///
    /// @param block The preset data to save.
    /// @param presetName The name to use for the saved preset file.
    /// @param pluginName The name of the plugin the preset belongs to.
    void savePreset(const MemoryBlock& block, const String& presetName, const String& pluginName);

    /// Returns a StringArray of the user-saved presets for the named plugin.
    ///
    /// @param pluginName The name of the plugin to list presets for.
    /// @param presetList Output parameter; filled with the names of user-saved presets.
    static void getListOfUserPresets(const String& pluginName, StringArray& presetList);

  private:
    /// Returns the directory where a plugin's user presets are stored.
    ///
    /// @param pluginName The name of the plugin.
    ///
    /// @return The directory path for the plugin's user presets.
    static File getPluginPresetDir(const String& pluginName);
};

#endif
