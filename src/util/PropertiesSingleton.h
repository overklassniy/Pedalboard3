// PropertiesSingleton.h - Singleton wrapper for ApplicationProperties.
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

#ifndef PROPERTIESSINGLETON_H_
#define PROPERTIESSINGLETON_H_

#include <JuceHeader.h>

/// Singleton wrapper for ApplicationProperties.
///
/// Because JUCE's ApplicationProperties is no longer a singleton, this class
/// provides a single global instance accessible from anywhere in the codebase.
class PropertiesSingleton {
  public:
    /// Returns the sole ApplicationProperties instance.
    static ApplicationProperties& getInstance();
    /// Kills the ApplicationProperties instance.
    ///
    /// Only call this once, when closing the program.
    static void killInstance();

  private:
    PropertiesSingleton();
    ~PropertiesSingleton();

    static PropertiesSingleton singletonInstance;
    std::unique_ptr<ApplicationProperties> instance;
};

#endif
