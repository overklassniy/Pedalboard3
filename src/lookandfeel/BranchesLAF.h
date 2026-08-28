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
///
/// Subclasses LookAndFeel_V4 so the many JUCE 8 widget methods that
/// BranchesLAF does not customise fall back to the default V4
/// implementation instead of remaining pure virtual.
class BranchesLAF : public juce::LookAndFeel_V4 {
  public:
    /// Configures the widget colours from the current ColourScheme.
    BranchesLAF();
    ~BranchesLAF() override;

    /// Draws the button background.
    ///
    /// @param g The graphics context to draw with.
    /// @param button The button being drawn.
    /// @param backgroundColour The base background colour for the button.
    /// @param shouldDrawButtonAsHighlighted Whether the button should be drawn in its highlighted state.
    /// @param shouldDrawButtonAsDown Whether the button should be drawn in its pressed-down state.
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    /// Draws button text.
    ///
    /// @param g The graphics context to draw with.
    /// @param button The text button being drawn.
    /// @param shouldDrawButtonAsHighlighted Whether the button should be drawn in its highlighted state.
    /// @param shouldDrawButtonAsDown Whether the button should be drawn in its pressed-down state.
    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    /// Draws the scrollbar buttons.
    ///
    /// @param g The graphics context to draw with.
    /// @param scrollbar The scrollbar the button belongs to.
    /// @param width The width of the button.
    /// @param height The height of the button.
    /// @param buttonDirection The direction of the button (0=up, 1=right, 2=down, 3=left).
    /// @param isScrollbarVertical Whether the scrollbar is oriented vertically.
    /// @param isMouseOverButton Whether the mouse is over the button.
    /// @param isButtonDown Whether the button is currently pressed.
    void drawScrollbarButton(juce::Graphics& g, juce::ScrollBar& scrollbar, int width, int height, int buttonDirection,
                             bool isScrollbarVertical, bool isMouseOverButton, bool isButtonDown) override;
    /// Draws the scrollbar.
    ///
    /// @param g The graphics context to draw with.
    /// @param scrollbar The scrollbar being drawn.
    /// @param x The x position of the scrollbar.
    /// @param y The y position of the scrollbar.
    /// @param width The width of the scrollbar.
    /// @param height The height of the scrollbar.
    /// @param isScrollbarVertical Whether the scrollbar is oriented vertically.
    /// @param thumbStartPosition The start position of the scrollbar thumb.
    /// @param thumbSize The size of the scrollbar thumb.
    /// @param isMouseOver Whether the mouse is over the scrollbar.
    /// @param isMouseDown Whether the mouse button is currently pressed.
    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height,
                       bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver,
                       bool isMouseDown) override;

    /// Draws the menubar background.
    ///
    /// @param g The graphics context to draw with.
    /// @param width The width of the menubar.
    /// @param height The height of the menubar.
    /// @param isMouseOverBar Whether the mouse is over the menubar.
    /// @param menuBar The menubar component being drawn.
    void drawMenuBarBackground(juce::Graphics& g, int width, int height, bool isMouseOverBar,
                               juce::MenuBarComponent& menuBar) override;
    /// Returns the menubar font.
    ///
    /// @param menuBar The menubar component.
    /// @param itemIndex The index of the menu item.
    /// @param itemText The text of the menu item.
    /// @return The font to use for the menubar item.
    juce::Font getMenuBarFont(juce::MenuBarComponent& menuBar, int itemIndex, const juce::String& itemText) override;
    /// Draws the menubar items.
    ///
    /// @param g The graphics context to draw with.
    /// @param width The width of the item.
    /// @param height The height of the item.
    /// @param itemIndex The index of the menu item.
    /// @param itemText The text of the menu item.
    /// @param isMouseOverItem Whether the mouse is over the item.
    /// @param isMenuOpen Whether the item's menu is open.
    /// @param isMouseOverBar Whether the mouse is over the menubar.
    /// @param menuBar The menubar component being drawn.
    void drawMenuBarItem(juce::Graphics& g, int width, int height, int itemIndex, const juce::String& itemText,
                         bool isMouseOverItem, bool isMenuOpen, bool isMouseOverBar,
                         juce::MenuBarComponent& menuBar) override;
    /// The width of a menubar item.
    ///
    /// @param menuBar The menubar component.
    /// @param itemIndex The index of the menu item.
    /// @param itemText The text of the menu item.
    /// @return The width in pixels of the menubar item.
    int getMenuBarItemWidth(juce::MenuBarComponent& menuBar, int itemIndex, const juce::String& itemText) override;
    /// Returns the popup menu font.
    juce::Font getPopupMenuFont() override { return juce::Font(juce::FontOptions().withHeight(15.0f)); }
    /// Returns the font used for TextButton labels.
    ///
    /// Overrides LookAndFeel_V4 which returns up to 16px; 15px keeps
    /// button text consistent with labels and menus.
    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;
    /// Returns the font used for ComboBox labels.
    ///
    /// Overrides LookAndFeel_V4 which returns up to 16px; 15px keeps
    /// combo box text consistent with the rest of the UI.
    juce::Font getComboBoxFont(juce::ComboBox& box) override;
    /// Returns the font used for Label components.
    ///
    /// Overrides LookAndFeel_V4 which returns the JUCE default 14px;
    /// 15px keeps labels consistent with the rest of the UI.
    juce::Font getLabelFont(juce::Label& label) override;
    /// Draws the popup menu background.
    ///
    /// @param g The graphics context to draw with.
    /// @param width The width of the popup menu.
    /// @param height The height of the popup menu.
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    /// Cancels menus' drop shadow.
    int getMenuWindowFlags() override { return 0; }

    /// Returns the image of a folder for the file chooser.
    ///
    /// @return A drawable folder image used by the file chooser.
    const juce::Drawable* getDefaultFolderImage() override;
    /// Draws a combobox (used in the file chooser).
    ///
    /// @param g The graphics context to draw with.
    /// @param width The width of the combobox.
    /// @param height The height of the combobox.
    /// @param isButtonDown Whether the combobox button is pressed.
    /// @param buttonX The x position of the dropdown button.
    /// @param buttonY The y position of the dropdown button.
    /// @param buttonW The width of the dropdown button.
    /// @param buttonH The height of the dropdown button.
    /// @param box The combobox being drawn.
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY,
                      int buttonW, int buttonH, juce::ComboBox& box) override;

    /// Draws the ProgressBar.
    ///
    /// @param g The graphics context to draw with.
    /// @param progressBar The progress bar being drawn.
    /// @param width The width of the progress bar.
    /// @param height The height of the progress bar.
    /// @param progress The current progress value (0.0 to 1.0).
    /// @param textToShow The text to display on the progress bar.
    void drawProgressBar(juce::Graphics& g, juce::ProgressBar& progressBar, int width, int height, double progress,
                         const juce::String& textToShow) override;

    /// Draws the KeymapChange button.
    ///
    /// @param g The graphics context to draw with.
    /// @param width The width of the button.
    /// @param height The height of the button.
    /// @param button The button being drawn.
    /// @param keyDescription The text describing the assigned key, or empty if no key is assigned.
    void drawKeymapChangeButton(juce::Graphics& g, int width, int height, juce::Button& button,
                                const juce::String& keyDescription) override;

    /// Draws a Label.
    ///
    /// @param g The graphics context to draw with.
    /// @param label The label being drawn.
    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    /// Draws a ToggleButton.
    ///
    /// @param g The graphics context to draw with.
    /// @param button The toggle button being drawn.
    /// @param shouldDrawButtonAsHighlighted Whether the button should be drawn in its highlighted state.
    /// @param shouldDrawButtonAsDown Whether the button should be drawn in its pressed-down state.
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    /// Draws a tick box.
    ///
    /// @param g The graphics context to draw with.
    /// @param component The component the tick box belongs to.
    /// @param x The x position of the tick box.
    /// @param y The y position of the tick box.
    /// @param w The width of the tick box.
    /// @param h The height of the tick box.
    /// @param ticked Whether the tick box is checked.
    /// @param isEnabled Whether the tick box is enabled.
    /// @param shouldDrawButtonAsHighlighted Whether the tick box should be drawn in its highlighted state.
    /// @param shouldDrawButtonAsDown Whether the tick box should be drawn in its pressed-down state.
    void drawTickBox(juce::Graphics& g, juce::Component& component, float x, float y, float w, float h, bool ticked,
                     bool isEnabled, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    /// Fills in the TextEditor background.
    ///
    /// @param g The graphics context to draw with.
    /// @param width The width of the text editor (unused).
    /// @param height The height of the text editor (unused).
    /// @param textEditor The text editor being drawn.
    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;

    /// Draws the callout box background.
    ///
    /// @param box The callout box being drawn.
    /// @param g The graphics context to draw with.
    /// @param path The path of the callout box shape.
    /// @param cachedImage The cached image to draw into.
    void drawCallOutBoxBackground(juce::CallOutBox& box, juce::Graphics& g, const juce::Path& path,
                                  juce::Image& cachedImage) override;

    /// Creates the alert window.
    ///
    /// Overrides LookAndFeel_V4 which adds a 50px size offset and shifts
    /// buttons by (25, 40), causing text overlap and misaligned buttons.
    /// Delegates to LookAndFeel_V2 which sizes the window correctly via
    /// AlertWindow::updateLayout without any post-creation adjustments.
    ///
    /// @param title The title of the alert window.
    /// @param message The message text to display.
    /// @param button1 The text for the first button.
    /// @param button2 The text for the second button (empty if none).
    /// @param button3 The text for the third button (empty if none).
    /// @param iconType The icon type to display.
    /// @param numButtons The number of buttons to create.
    /// @param associatedComponent The component the alert is associated with.
    /// @return The created AlertWindow.
    juce::AlertWindow* createAlertWindow(const juce::String& title, const juce::String& message,
                                         const juce::String& button1, const juce::String& button2,
                                         const juce::String& button3, juce::MessageBoxIconType iconType,
                                         int numButtons, juce::Component* associatedComponent) override;
    /// Draws the alert box.
    ///
    /// Overrides LookAndFeel_V4 which draws text at a fixed y=30 position
    /// that assumes the V4 50px offset. Delegates to LookAndFeel_V2 which
    /// draws text at the textArea position set by updateLayout, ensuring
    /// the text and buttons are correctly aligned.
    ///
    /// @param g The graphics context to draw with.
    /// @param alert The alert window being drawn.
    /// @param textArea The area where text should be drawn.
    /// @param textLayout The pre-laid-out text to draw.
    void drawAlertBox(juce::Graphics& g, juce::AlertWindow& alert, const juce::Rectangle<int>& textArea,
                      juce::TextLayout& textLayout) override;
};

#endif
