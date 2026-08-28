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
#include "Images.h"
#include "InternalFilters.h"
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
        setResizable(true, true);

        auto* panel = new MainPanel(&commandManager);
        setContentOwned(panel, true);

        // Render the menu bar (File, Edit, Options, Help) below the title
        // bar. MainPanel implements MenuBarModel.
        setMenuBar(panel, 0);

        centreWithSize(1024, 768);

        // Restore the saved window position and size.
        auto savedState = PropertiesSingleton::getInstance().getUserSettings()->getValue("WindowState");
        if (savedState.isNotEmpty())
            restoreWindowStateFromString(savedState);

        setVisible(true);

        // Set the window icon in the native title bar and taskbar.
        if (auto* peer = getPeer())
            peer->setIcon(juce::ImageCache::getFromMemory(Images::icon512_png, Images::icon512_pngSize));

        // Register the key-mapping set as a key listener on the window
        // so that keyboard shortcuts work even when focus is inside a
        // child component that does not forward key presses to the
        // MainPanel.
        addKeyListener(commandManager.getKeyMappings());
    }

    /// Handles the close button.
    ///
    /// When the tray icon is enabled, closing the window hides it
    /// instead of quitting. Clicking close on an already-hidden window
    /// prompts the user to save and then quits. When the tray icon is
    /// disabled, the user is prompted to save and the app quits.
    void closeButtonPressed() override {
        auto* panel = dynamic_cast<MainPanel*>(getContentComponent());

        if (PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("useTrayIcon")) {
            if (isVisible())
                setVisible(false);
            else {
                if (panel) {
                    auto result = panel->saveIfNeededAndUserAgrees();
                    if (result == juce::FileBasedDocument::savedOk)
                        juce::JUCEApplication::quit();
                } else
                    juce::JUCEApplication::quit();
            }
        } else {
            if (panel) {
                auto result = panel->saveIfNeededAndUserAgrees();
                if (result == juce::FileBasedDocument::savedOk)
                    juce::JUCEApplication::quit();
            } else
                juce::JUCEApplication::quit();
        }
    }

    /// Destroys the content panel and clears the menu bar before
    /// member destructors run.
    ///
    /// commandManager is a member of MainWindow. Member destructors
    /// run before the base class ~DocumentWindow(), which would
    /// otherwise destroy MainPanel. By calling setContentOwned
    /// (nullptr, true) here we destroy MainPanel while
    /// commandManager is still alive, so ~MainPanel() can safely
    /// access it to save key mappings.
    ~MainWindow() override {
        // Save the window position and size before destroying content.
        PropertiesSingleton::getInstance().getUserSettings()->setValue("WindowState", getWindowStateAsString());

        setMenuBar(nullptr);
        setContentOwned(nullptr, true);
    }

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

    // Register the internal plugin format (built-in processors) with the
    // plugin format manager. This is done here, after static initialisation
    // has completed, because the internal processors depend on singletons
    // (e.g. MainTransport) that are not safe to construct during static init.
    AudioPluginFormatManagerSingleton::getInstance().addFormat(std::make_unique<InternalPluginFormat>());

    // Apply the custom LookAndFeel before any UI is created so every
    // component inherits the Pedalboard2 widget styling.
    lookAndFeel = std::make_unique<BranchesLAF>();
    juce::LookAndFeel::setDefaultLookAndFeel(lookAndFeel.get());

    mainWindow = std::make_unique<MainWindow>(getApplicationName());

    // Create the system tray icon at startup if the user enabled it
    // in Preferences. Without this the setting is saved but never
    // applied on the next launch. If startInTray is also enabled,
    // hide the main window so the app starts minimized to the tray.
#ifndef JUCE_MAC
    if (PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("useTrayIcon")) {
        trayIcon = std::make_unique<TrayIcon>(mainWindow.get());

        if (PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("startInTray"))
            mainWindow->setVisible(false);
    }
#endif
}

/// Destroys the main window and tears down all audio singletons.
void Pedalboard3App::shutdown() {
    mainWindow = nullptr;
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    lookAndFeel = nullptr;
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
