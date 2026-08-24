// BranchesLAF.h - LookAndFeel class implementing custom widgets.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2008 Niall Moody.
//
// Originally written for Branches, a branching story editor.
// Adapted for Pedalboard3 from the original Branches source.
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

#ifndef BRANCHESLAF_H_
#define BRANCHESLAF_H_

#include <JuceHeader.h>

/// LookAndFeel class implementing custom buttons, scrollbars, menus, and other widgets.
class BranchesLAF : public juce::LookAndFeel
{
  public:
    /// Constructor.
    BranchesLAF();
    /// Destructor.
    ~BranchesLAF() override;

    /// Draws the button background.
    void drawButtonBackground(juce::Graphics &g,
                              juce::Button &button,
                              const juce::Colour &backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
    /// Draws button text.
    void drawButtonText(juce::Graphics &g,
                        juce::TextButton &button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    /// Draws the scrollbar buttons.
    void drawScrollbarButton(juce::Graphics &g,
                             juce::ScrollBar &scrollbar,
                             int width,
                             int height,
                             int buttonDirection,
                             bool isScrollbarVertical,
                             bool isMouseOverButton,
                             bool isButtonDown) override;
    /// Draws the scrollbar.
    void drawScrollbar(juce::Graphics &g,
                       juce::ScrollBar &scrollbar,
                       int x,
                       int y,
                       int width,
                       int height,
                       bool isScrollbarVertical,
                       int thumbStartPosition,
                       int thumbSize,
                       bool isMouseOver,
                       bool isMouseDown) override;

    /// Draws the menubar background.
    void drawMenuBarBackground(juce::Graphics &g,
                               int width,
                               int height,
                               bool isMouseOverBar,
                               juce::MenuBarComponent &menuBar) override;
    /// Returns the menubar font.
    juce::Font getMenuBarFont(juce::MenuBarComponent &menuBar,
                              int itemIndex,
                              const juce::String &itemText) override;
    /// Draws the menubar items.
    void drawMenuBarItem(juce::Graphics &g,
                         int width,
                         int height,
                         int itemIndex,
                         const juce::String &itemText,
                         bool isMouseOverItem,
                         bool isMenuOpen,
                         bool isMouseOverBar,
                         juce::MenuBarComponent &menuBar) override;
    /// The width of a menubar item.
    int getMenuBarItemWidth(juce::MenuBarComponent &menuBar,
                            int itemIndex,
                            const juce::String &itemText) override;
    /// Returns the popup menu font.
    juce::Font getPopupMenuFont() override
    {
        return juce::Font(juce::FontOptions().withHeight(15.0f));
    }
    /// Draws the popup menu background.
    void drawPopupMenuBackground(juce::Graphics &g, int width, int height) override;
    /// Cancels menus' drop shadow.
    int getMenuWindowFlags() override { return 0; }

    /// Returns the image of a folder for the file chooser.
    const juce::Drawable *getDefaultFolderImage() override;
    /// Draws a combobox (used in the file chooser).
    void drawComboBox(juce::Graphics &g,
                      int width,
                      int height,
                      bool isButtonDown,
                      int buttonX,
                      int buttonY,
                      int buttonW,
                      int buttonH,
                      juce::ComboBox &box) override;

    /// Draws the ProgressBar.
    void drawProgressBar(juce::Graphics &g,
                         juce::ProgressBar &progressBar,
                         int width,
                         int height,
                         double progress,
                         const juce::String &textToShow) override;

    /// Draws the KeymapChange button.
    void drawKeymapChangeButton(juce::Graphics &g,
                                int width,
                                int height,
                                juce::Button &button,
                                const juce::String &keyDescription) override;

    /// Draws a Label.
    void drawLabel(juce::Graphics &g, juce::Label &label) override;

    /// Draws a ToggleButton.
    void drawToggleButton(juce::Graphics &g,
                          juce::ToggleButton &button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    /// Draws a tick box.
    void drawTickBox(juce::Graphics &g,
                     juce::Component &component,
                     float x, float y, float w, float h,
                     bool ticked,
                     bool isEnabled,
                     bool shouldDrawButtonAsHighlighted,
                     bool shouldDrawButtonAsDown) override;

    /// Fills in the TextEditor background.
    void fillTextEditorBackground(juce::Graphics &g, int width, int height,
                                  juce::TextEditor &textEditor) override;

    /// Draws the callout box background.
    void drawCallOutBoxBackground(juce::CallOutBox &box,
                                  juce::Graphics &g,
                                  const juce::Path &path,
                                  juce::Image &cachedImage) override;
};

#endif
