// AudioSingletons.h - Singleton wrappers for JUCE audio classes.
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

#ifndef AUDIOSINGLETONS_H_
#define AUDIOSINGLETONS_H_

#include <JuceHeader.h>

/// Singleton wrapper for AudioPluginFormatManager.
class AudioPluginFormatManagerSingleton
{
  public:
    /// Returns the sole AudioPluginFormatManager instance.
    static AudioPluginFormatManager& getInstance();
    /// Kills the instance. Call once at shutdown.
    static void killInstance();

  private:
    AudioPluginFormatManagerSingleton();
    ~AudioPluginFormatManagerSingleton();

    static AudioPluginFormatManagerSingleton singletonInstance;
    std::unique_ptr<AudioPluginFormatManager> instance;
};

/// Singleton wrapper for AudioFormatManager.
class AudioFormatManagerSingleton
{
  public:
    /// Returns the sole AudioFormatManager instance.
    static AudioFormatManager& getInstance();
    /// Kills the instance. Call once at shutdown.
    static void killInstance();

  private:
    AudioFormatManagerSingleton();
    ~AudioFormatManagerSingleton();

    static AudioFormatManagerSingleton singletonInstance;
    std::unique_ptr<AudioFormatManager> instance;
};

/// Singleton wrapper for AudioThumbnailCache.
class AudioThumbnailCacheSingleton
{
  public:
    /// Returns the sole AudioThumbnailCache instance.
    static AudioThumbnailCache& getInstance();
    /// Kills the instance. Call once at shutdown.
    static void killInstance();

  private:
    AudioThumbnailCacheSingleton();
    ~AudioThumbnailCacheSingleton();

    static AudioThumbnailCacheSingleton singletonInstance;
    std::unique_ptr<AudioThumbnailCache> instance;
};

/// Singleton accessor for KnownPluginList (set by MainPanel on startup).
class KnownPluginListSingleton
{
  public:
    /// Sets the global KnownPluginList reference (call from MainPanel constructor).
    static void setInstance(KnownPluginList* list) { instance = list; }
    /// Returns the KnownPluginList, or nullptr if not yet set.
    static KnownPluginList* getInstance() { return instance; }

  private:
    static KnownPluginList* instance;
};

#endif
