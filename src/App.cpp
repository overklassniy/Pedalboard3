// App.cpp - Main application entry point.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2026 Pedalboard3 Project.
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

/// Placeholder main window. Will be replaced by the full MainPanel in Phase 3.
class MainWindow : public juce::DocumentWindow
{
  public:
    MainWindow(juce::String name)
        : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                                  juce::ResizableWindow::backgroundColourId),
                         DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new juce::Label({}, "Pedalboard3 - Phase 0 build verification"), true);

        centreWithSize(600, 400);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

void Pedalboard3App::initialise(const juce::String& commandLine)
{
    juce::ignoreUnused(commandLine);

    mainWindow = std::make_unique<MainWindow>(getApplicationName());
}

void Pedalboard3App::shutdown()
{
    mainWindow = nullptr;
}

void Pedalboard3App::anotherInstanceStarted(const juce::String& commandLine)
{
    juce::ignoreUnused(commandLine);
}

/// Required JUCE application macro.
START_JUCE_APPLICATION(Pedalboard3App)
