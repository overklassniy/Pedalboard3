// App.h - Main application entry point.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2009 Niall Moody.
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

#pragma once

#include <JuceHeader.h>

class TrayIcon;

/// Main JUCE application class. Bootstraps the main window and audio engine.
class Pedalboard3App : public juce::JUCEApplication {
  public:
    /// Default constructor.
    Pedalboard3App() = default;

    /// Destructor.
    ~Pedalboard3App() override = default;

    /// Creates the properties singleton and main window at startup.
    ///
    /// @param commandLine The command-line arguments passed to the application (currently unused).
    void initialise(const juce::String& commandLine) override;

    /// Destroys the main window and tears down all audio singletons.
    void shutdown() override;

    /// Returns the application name.
    const juce::String getApplicationName() override { return "Pedalboard3"; }

    /// Returns the application version string.
    const juce::String getApplicationVersion() override { return "3.0.0"; }

    /// Reports whether multiple instances may run simultaneously.
    bool moreThanOneInstanceAllowed() override { return true; }

    /// Called when a second instance is launched; currently unused.
    ///
    /// @param commandLine The command-line arguments passed to the second instance (currently unused).
    void anotherInstanceStarted(const juce::String& commandLine) override;

    /// Shows or hides the system tray icon.
    ///
    /// @param val True to show the tray icon; false to hide it.
    void showTrayIcon(bool val);

  private:
    /// The top-level document window.
    std::unique_ptr<juce::DocumentWindow> mainWindow;

    /// The optional system tray icon (not used on macOS).
    std::unique_ptr<TrayIcon> trayIcon;
};
