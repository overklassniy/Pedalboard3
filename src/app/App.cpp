// App.cpp - Main application entry point.
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

#include "App.h"

#include "AudioSingletons.h"
#include "BranchesLAF.h"
#include "ColourScheme.h"
#include "MainPanel.h"
#include "PropertiesSingleton.h"
#include "TrayIcon.h"

/// Main application window holding the top-level MainPanel.
class MainWindow : public juce::DocumentWindow {
  public:
    /// Creates the main window and its MainPanel content.
    ///
    /// @param name The window title to display in the native title bar.
    MainWindow(juce::String name) : DocumentWindow(name, juce::Colours::black, DocumentWindow::allButtons) {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainPanel(&commandManager), true);

        centreWithSize(1024, 768);
        setVisible(true);
    }

    /// Requests the application to quit when the window is closed.
    void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }

  private:
    juce::ApplicationCommandManager commandManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

/// Creates the properties singleton and main window at startup.
///
/// @param commandLine The command-line arguments passed to the application (currently unused).
void Pedalboard3App::initialise(const juce::String& commandLine) {
    juce::ignoreUnused(commandLine);

    // Set up application properties storage before the UI is created.
    PropertiesSingleton::getInstance();

    mainWindow = std::make_unique<MainWindow>(getApplicationName());
}

/// Destroys the main window and tears down all audio singletons.
void Pedalboard3App::shutdown() {
    mainWindow = nullptr;
    AudioPluginFormatManagerSingleton::killInstance();
    AudioFormatManagerSingleton::killInstance();
    AudioThumbnailCacheSingleton::killInstance();
    PropertiesSingleton::killInstance();
}

/// Called when a second instance is launched; currently unused.
///
/// @param commandLine The command-line arguments passed to the second instance (currently unused).
void Pedalboard3App::anotherInstanceStarted(const juce::String& commandLine) {
    juce::ignoreUnused(commandLine);
}

/// Shows or hides the system tray icon.
///
/// The tray icon is not used on macOS, where the Dock serves the same role.
///
/// @param val True to show the tray icon; false to hide it.
void Pedalboard3App::showTrayIcon(bool val) {
#ifndef JUCE_MAC
    if (val && !trayIcon)
        trayIcon = std::make_unique<TrayIcon>(mainWindow.get());
    else if (!val && trayIcon)
        trayIcon = nullptr;
#else
    juce::ignoreUnused(val);
#endif
}

/// Required JUCE application macro.
START_JUCE_APPLICATION(Pedalboard3App)
