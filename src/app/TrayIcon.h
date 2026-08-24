// TrayIcon.h - System tray icon.
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

#ifndef TRAYICON_H_
#define TRAYICON_H_

#ifndef JUCE_MAC

#include <JuceHeader.h>

/// System tray icon.
///
/// Provides a system tray icon with right-click popup menu and
/// double-click toggle for showing/hiding the main window.
class TrayIcon : public SystemTrayIconComponent
{
  public:
    /// Constructor.
    TrayIcon(DocumentWindow* win);
    /// Destructor.
    ~TrayIcon() override;

    /// Called to display a PopupMenu on right-click.
    void mouseDown(const MouseEvent& e) override;
    /// Called to display/hide the main window.
    void mouseDoubleClick(const MouseEvent& e) override;

  private:
    /// Our copy of the main window.
    DocumentWindow* window;
};

#endif

#endif
