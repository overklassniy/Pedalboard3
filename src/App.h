// App.h - Main application entry point.
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

#pragma once

#include <JuceHeader.h>

/// Main JUCE application class. Bootstraps the main window and audio engine.
class Pedalboard3App : public juce::JUCEApplication
{
  public:
    Pedalboard3App() = default;
    ~Pedalboard3App() override = default;

    void initialise(const juce::String& commandLine) override;
    void shutdown() override;

    const juce::String getApplicationName() override { return "Pedalboard3"; }
    const juce::String getApplicationVersion() override { return "3.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }
    void anotherInstanceStarted(const juce::String& commandLine) override;

  private:
    std::unique_ptr<juce::DocumentWindow> mainWindow;
};
