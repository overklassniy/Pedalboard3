// BranchesLAF.cpp - LookAndFeel class implementing custom widgets.
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

#include "BranchesLAF.h"
#include "ColourScheme.h"
#include "LookAndFeelImages.h"

#include <map>

//------------------------------------------------------------------------------
BranchesLAF::BranchesLAF()
{
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    setColour(TextButton::buttonColourId, colours["Button Colour"]);
    setColour(TextButton::buttonOnColourId, colours["Button Colour"]);
    setColour(PopupMenu::highlightedBackgroundColourId, colours["Menu Selection Colour"]);
    setColour(PopupMenu::backgroundColourId, colours["Window Background"]);
    setColour(AlertWindow::backgroundColourId, colours["Window Background"]);
    setColour(ComboBox::buttonColourId, colours["Button Colour"]);
    setColour(TextEditor::highlightColourId, colours["Button Highlight"]);
    setColour(TextEditor::focusedOutlineColourId, colours["Menu Selection Colour"]);
    setColour(DirectoryContentsDisplayComponent::highlightColourId, Colour(0xFFD7D1B5));
    setColour(ProgressBar::backgroundColourId, colours["Window Background"]);
    setColour(ProgressBar::foregroundColourId, colours["CPU Meter Colour"]);
}

//------------------------------------------------------------------------------
BranchesLAF::~BranchesLAF()
{
}

//------------------------------------------------------------------------------
void BranchesLAF::drawButtonBackground(Graphics &g,
                                       Button &button,
                                       const Colour &backgroundColour,
                                       bool shouldDrawButtonAsHighlighted,
                                       bool shouldDrawButtonAsDown)
{
    Path highlight, shadow;
    Colour buttonCol = ColourScheme::getInstance().colours["Button Colour"];
    ColourGradient grad(buttonCol.brighter(0.8f),
                        0.0f,
                        0.0f,
                        buttonCol.darker(0.05f),
                        0.0f,
                        static_cast<float>(button.getHeight()),
                        false);

    g.setColour(Colours::black);
    g.drawRoundedRectangle(1.0f,
                           1.0f,
                           static_cast<float>(button.getWidth() - 2),
                           static_cast<float>(button.getHeight() - 2),
                           4.0f,
                           1.0f);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(1.0f,
                           1.0f,
                           static_cast<float>(button.getWidth() - 2),
                           static_cast<float>(button.getHeight() - 2),
                           4.0f);

    // Draw mouse over highlight.
    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(ColourScheme::getInstance().colours["Button Highlight"]);
        g.drawRoundedRectangle(2.0f,
                               2.0f,
                               static_cast<float>(button.getWidth() - 4),
                               static_cast<float>(button.getHeight() - 4),
                               4.0f,
                               3.0f);
    }

    // Draw highlight.
    highlight.startNewSubPath(2.0f, static_cast<float>(button.getHeight() - 3));
    highlight.lineTo(2.0f, 4.0f);
    highlight.quadraticTo(2.0f, 2.0f, 4.0f, 2.0f);
    highlight.lineTo(static_cast<float>(button.getWidth() - 3), 2.0f);
    if (!shouldDrawButtonAsDown)
    {
        if (!shouldDrawButtonAsHighlighted)
            g.setColour(buttonCol.brighter(1.0f).withAlpha(0.69f));
        else
            g.setColour(buttonCol.brighter(1.0f).withAlpha(0.38f));
        g.strokePath(highlight, PathStrokeType(2.0f,
                                               PathStrokeType::curved,
                                               PathStrokeType::rounded));
    }
    else
    {
        g.setColour(Colour(0x20000000));
        g.strokePath(highlight, PathStrokeType(1.0f,
                                               PathStrokeType::curved,
                                               PathStrokeType::rounded));
    }

    // Draw shadow.
    shadow.startNewSubPath(3.0f, static_cast<float>(button.getHeight() - 2));
    shadow.lineTo(static_cast<float>(button.getWidth() - 4),
                  static_cast<float>(button.getHeight() - 2));
    shadow.quadraticTo(static_cast<float>(button.getWidth() - 2),
                       static_cast<float>(button.getHeight() - 2),
                       static_cast<float>(button.getWidth() - 2),
                       static_cast<float>(button.getHeight() - 4));
    shadow.lineTo(static_cast<float>(button.getWidth() - 2), 3.0f);
    if (!shouldDrawButtonAsDown)
    {
        g.setColour(Colour(0x20000000));
        g.strokePath(shadow, PathStrokeType(1.0f,
                                            PathStrokeType::curved,
                                            PathStrokeType::rounded));
    }
    else
    {
        if (!shouldDrawButtonAsHighlighted)
            g.setColour(buttonCol.brighter(1.0f).withAlpha(0.69f));
        else
            g.setColour(buttonCol.brighter(1.0f).withAlpha(0.38f));
        g.strokePath(shadow, PathStrokeType(2.0f,
                                            PathStrokeType::curved,
                                            PathStrokeType::rounded));
    }
}

//------------------------------------------------------------------------------
void BranchesLAF::drawButtonText(Graphics &g,
                                 TextButton &button,
                                 bool shouldDrawButtonAsHighlighted,
                                 bool shouldDrawButtonAsDown)
{
    int inc;

    auto buttonFont = getTextButtonFont(button, button.getHeight());
    g.setFont(buttonFont);
    g.setColour(ColourScheme::getInstance().colours["Text Colour"]
                    .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = jmin(4, button.proportionOfHeight(0.3f));
    const int cornerSize = jmin(button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = juce::roundToInt(buttonFont.getHeight() * 0.6f);
    const int leftIndent = jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));

    if (shouldDrawButtonAsDown)
        inc = 1;
    else
        inc = 0;

    g.drawFittedText(button.getButtonText(),
                     leftIndent + inc,
                     yIndent + inc,
                     button.getWidth() - leftIndent - rightIndent,
                     button.getHeight() - yIndent * 2,
                     Justification::centred, 2);
}

//------------------------------------------------------------------------------
void BranchesLAF::drawScrollbarButton(Graphics &g,
                                      ScrollBar &scrollbar,
                                      int width,
                                      int height,
                                      int buttonDirection,
                                      bool isScrollbarVertical,
                                      bool isMouseOverButton,
                                      bool isButtonDown)
{
    float inc;
    Path highlight, shadow, tri;
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    if (!isScrollbarVertical)
    {
        // Background behind the button.
        ColourGradient grad(colours["Window Background"].darker(0.25f),
                            0.0f,
                            0.0f,
                            colours["Window Background"],
                            0.0f,
                            static_cast<float>(height),
                            false);

        if (buttonDirection == 3)
        {
            g.setGradientFill(grad);
            g.fillRect(static_cast<float>(3), static_cast<float>(0), static_cast<float>(width), static_cast<float>(height));

            g.setColour(Colour(0x30000000));
            g.fillRect(static_cast<float>(width) - 3.0f, static_cast<float>(0), 3.0f, 1.0f);
            g.fillRect(static_cast<float>(width) - 3.0f, static_cast<float>(height) - 1.0f, 3.0f, 1.0f);
        }
        else if (buttonDirection == 1)
        {
            g.setGradientFill(grad);
            g.fillRect(static_cast<float>(0), static_cast<float>(0), static_cast<float>(width) - 3.0f, static_cast<float>(height));

            g.setColour(Colour(0x30000000));
            g.fillRect(static_cast<float>(0), static_cast<float>(0), 3.0f, 1.0f);
            g.fillRect(static_cast<float>(0), static_cast<float>(height) - 1.0f, 3.0f, 1.0f);
        }

        g.setColour(Colour(0x80000000));
        g.drawRoundedRectangle(1.0f,
                               1.0f,
                               static_cast<float>(width - 2),
                               static_cast<float>(height - 2),
                               2.0f,
                               1.0f);
        g.setColour(colours["Button Colour"]);
        g.fillRoundedRectangle(1.0f,
                               1.0f,
                               static_cast<float>(width - 2),
                               static_cast<float>(height - 2),
                               2.0f);

        g.setColour(Colour(0x08000000));
        g.fillRoundedRectangle(1.0f,
                               static_cast<float>(height / 2),
                               static_cast<float>(width - 2),
                               static_cast<float>(height - 2),
                               2.0f);

        if (isMouseOverButton)
        {
            // Draw highlight.
            highlight.startNewSubPath(2.0f, static_cast<float>(height - 3));
            highlight.lineTo(2.0f, 4.0f);
            highlight.quadraticTo(2.0f, 2.0f, 4.0f, 2.0f);
            highlight.lineTo(static_cast<float>(width - 3), 2.0f);
            if (!isButtonDown)
            {
                g.setColour(Colour(0xB0FFFFFF));
                g.strokePath(highlight, PathStrokeType(2.0f,
                                                       PathStrokeType::curved,
                                                       PathStrokeType::rounded));
            }
            else
            {
                g.setColour(Colour(0x20000000));
                g.strokePath(highlight, PathStrokeType(1.0f,
                                                       PathStrokeType::curved,
                                                       PathStrokeType::rounded));
            }

            // Draw shadow.
            shadow.startNewSubPath(3.0f, static_cast<float>(height - 2));
            shadow.lineTo(static_cast<float>(width - 4),
                          static_cast<float>(height - 2));
            shadow.quadraticTo(static_cast<float>(width - 2),
                               static_cast<float>(height - 2),
                               static_cast<float>(width - 2),
                               static_cast<float>(height - 4));
            shadow.lineTo(static_cast<float>(width - 2), 3.0f);
            if (!isButtonDown)
            {
                g.setColour(Colour(0x20000000));
                g.strokePath(shadow, PathStrokeType(1.0f,
                                                    PathStrokeType::curved,
                                                    PathStrokeType::rounded));
            }
            else
            {
                g.setColour(Colour(0xB0FFFFFF));
                g.strokePath(shadow, PathStrokeType(2.0f,
                                                    PathStrokeType::curved,
                                                    PathStrokeType::rounded));
            }
        }
    }
    else
    {
        // Background behind the button.
        ColourGradient grad(colours["Window Background"].darker(0.25f),
                            0.0f,
                            0.0f,
                            colours["Window Background"],
                            static_cast<float>(width),
                            0.0f,
                            false);

        if (buttonDirection == 2)
        {
            g.setGradientFill(grad);
            g.fillRect(static_cast<float>(0), static_cast<float>(0), static_cast<float>(width), static_cast<float>(height) - 3.0f);

            g.setColour(Colour(0x30000000));
            g.fillRect(static_cast<float>(0), static_cast<float>(0), 1.0f, 3.0f);
            g.fillRect(static_cast<float>(width) - 1.0f, 0.0f, 1.0f, 3.0f);
        }
        else if (buttonDirection == 0)
        {
            g.setGradientFill(grad);
            g.fillRect(static_cast<float>(0), static_cast<float>(3), static_cast<float>(width), static_cast<float>(height));

            g.setColour(Colour(0x30000000));
            g.fillRect(static_cast<float>(0), static_cast<float>(height) - 3.0f, 1.0f, 3.0f);
            g.fillRect(static_cast<float>(width) - 1.0f, static_cast<float>(height) - 3.0f, 1.0f, 3.0f);
        }

        g.setColour(Colour(0x80000000));
        g.drawRoundedRectangle(1.0f,
                               1.0f,
                               static_cast<float>(width - 2),
                               static_cast<float>(height - 2),
                               2.0f,
                               1.0f);
        g.setColour(colours["Button Colour"]);
        g.fillRoundedRectangle(1.0f,
                               1.0f,
                               static_cast<float>(width - 2),
                               static_cast<float>(height - 2),
                               2.0f);

        g.setColour(Colour(0x08000000));
        g.fillRoundedRectangle(static_cast<float>(width / 2),
                               1.0f,
                               static_cast<float>(width - 2),
                               static_cast<float>(height - 2),
                               2.0f);

        if (isMouseOverButton)
        {
            // Draw highlight.
            highlight.startNewSubPath(2.0f, static_cast<float>(height - 3));
            highlight.lineTo(2.0f, 4.0f);
            highlight.quadraticTo(2.0f, 2.0f, 4.0f, 2.0f);
            highlight.lineTo(static_cast<float>(width - 3), 2.0f);
            if (!isButtonDown)
            {
                g.setColour(Colour(0xB0FFFFFF));
                g.strokePath(highlight, PathStrokeType(2.0f,
                                                       PathStrokeType::curved,
                                                       PathStrokeType::rounded));
            }
            else
            {
                g.setColour(Colour(0x20000000));
                g.strokePath(highlight, PathStrokeType(1.0f,
                                                       PathStrokeType::curved,
                                                       PathStrokeType::rounded));
            }

            // Draw shadow.
            shadow.startNewSubPath(3.0f, static_cast<float>(height - 2));
            shadow.lineTo(static_cast<float>(width - 4),
                          static_cast<float>(height - 2));
            shadow.quadraticTo(static_cast<float>(width - 2),
                               static_cast<float>(height - 2),
                               static_cast<float>(width - 2),
                               static_cast<float>(height - 4));
            shadow.lineTo(static_cast<float>(width - 2), 3.0f);
            if (!isButtonDown)
            {
                g.setColour(Colour(0x20000000));
                g.strokePath(shadow, PathStrokeType(1.0f,
                                                    PathStrokeType::curved,
                                                    PathStrokeType::rounded));
            }
            else
            {
                g.setColour(Colour(0xB0FFFFFF));
                g.strokePath(shadow, PathStrokeType(2.0f,
                                                    PathStrokeType::curved,
                                                    PathStrokeType::rounded));
            }
        }
    }

    // Draw triangle.
    if (isButtonDown)
        inc = 1.0f;
    else
        inc = 0.0f;
    switch (buttonDirection)
    {
        case 0:
            tri.startNewSubPath(static_cast<float>(width / 2) - static_cast<float>(width / 4) + inc,
                                static_cast<float>(height / 2) + static_cast<float>(height / 8) + inc);
            tri.lineTo(static_cast<float>(width / 2) + inc,
                       static_cast<float>(height / 2) - static_cast<float>(height / 8) + inc);
            tri.lineTo(static_cast<float>(width / 2) + static_cast<float>(width / 4) + inc,
                       static_cast<float>(height / 2) + static_cast<float>(height / 8) + inc);
            break;
        case 1:
            tri.startNewSubPath(static_cast<float>(width / 2) - static_cast<float>(width / 8) + inc,
                                static_cast<float>(height / 2) - static_cast<float>(height / 4) + inc);
            tri.lineTo(static_cast<float>(width / 2) + static_cast<float>(width / 8) + inc,
                       static_cast<float>(height / 2) + inc);
            tri.lineTo(static_cast<float>(width / 2) - static_cast<float>(width / 8) + inc,
                       static_cast<float>(height / 2) + static_cast<float>(height / 4) + inc);
            break;
        case 2:
            tri.startNewSubPath(static_cast<float>(width / 2) - static_cast<float>(width / 4) + inc,
                                static_cast<float>(height / 2) - static_cast<float>(height / 8) + inc);
            tri.lineTo(static_cast<float>(width / 2) + inc,
                       static_cast<float>(height / 2) + static_cast<float>(height / 8) + inc);
            tri.lineTo(static_cast<float>(width / 2) + static_cast<float>(width / 4) + inc,
                       static_cast<float>(height / 2) - static_cast<float>(height / 8) + inc);
            break;
        case 3:
            tri.startNewSubPath(static_cast<float>(width / 2) + static_cast<float>(width / 8) + inc,
                                static_cast<float>(height / 2) + static_cast<float>(height / 4) + inc);
            tri.lineTo(static_cast<float>(width / 2) - static_cast<float>(width / 8) + inc,
                       static_cast<float>(height / 2) + inc);
            tri.lineTo(static_cast<float>(width / 2) + static_cast<float>(width / 8) + inc,
                       static_cast<float>(height / 2) - static_cast<float>(height / 4) + inc);
            break;
    }
    g.setColour(colours["Vector Colour"]);
    g.strokePath(tri, juce::PathStrokeType(2.0f));
}

//------------------------------------------------------------------------------
void BranchesLAF::drawScrollbar(Graphics &g,
                                ScrollBar &scrollbar,
                                int x,
                                int y,
                                int width,
                                int height,
                                bool isScrollbarVertical,
                                int thumbStartPosition,
                                int thumbSize,
                                bool isMouseOver,
                                bool isMouseDown)
{
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    if (!isScrollbarVertical)
    {
        ColourGradient grad(colours["Window Background"].darker(0.25f),
                            0.0f,
                            static_cast<float>(y),
                            colours["Window Background"],
                            0.0f,
                            static_cast<float>(height),
                            false);

        g.setGradientFill(grad);
        g.fillRect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));

        g.setColour(Colour(0x30000000));
        g.fillRect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), 1.0f);
        g.fillRect(static_cast<float>(x), static_cast<float>(height) - 1.0f, static_cast<float>(width), 1.0f);

        g.setColour(Colour(0x80000000));
        g.drawRoundedRectangle(static_cast<float>(thumbStartPosition + 1),
                               static_cast<float>(y + 1),
                               static_cast<float>(thumbSize - 2),
                               static_cast<float>(height - 2),
                               2.0f,
                               1.0f);
        g.setColour(colours["Button Colour"]);
        g.fillRoundedRectangle(static_cast<float>(thumbStartPosition + 1),
                               static_cast<float>(y + 1),
                               static_cast<float>(thumbSize - 2),
                               static_cast<float>(height - 2),
                               2.0f);

        g.setColour(Colour(0x08000000));
        g.fillRoundedRectangle(static_cast<float>(thumbStartPosition + 1),
                               static_cast<float>(height / 2),
                               static_cast<float>(thumbSize - 2),
                               static_cast<float>(height - 2),
                               2.0f);
    }
    else
    {
        ColourGradient grad(colours["Window Background"].darker(0.25f),
                            static_cast<float>(x),
                            0.0f,
                            colours["Window Background"],
                            static_cast<float>(width),
                            0.0f,
                            false);

        g.setGradientFill(grad);
        g.fillRect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));

        g.setColour(Colour(0x30000000));
        g.fillRect(static_cast<float>(x), static_cast<float>(y), 1.0f, static_cast<float>(height));
        g.fillRect(static_cast<float>(width) - 1.0f, static_cast<float>(y), 1.0f, static_cast<float>(height));

        g.setColour(Colour(0x80000000));
        g.drawRoundedRectangle(static_cast<float>(x + 1),
                               static_cast<float>(thumbStartPosition + 1),
                               static_cast<float>(width - 2),
                               static_cast<float>(thumbSize - 2),
                               2.0f,
                               1.0f);
        g.setColour(colours["Button Colour"]);
        g.fillRoundedRectangle(static_cast<float>(x + 1),
                               static_cast<float>(thumbStartPosition + 1),
                               static_cast<float>(width - 2),
                               static_cast<float>(thumbSize - 2),
                               2.0f);

        g.setColour(Colour(0x08000000));
        g.fillRoundedRectangle(static_cast<float>(width / 2),
                               static_cast<float>(thumbStartPosition + 1),
                               static_cast<float>(width - 2),
                               static_cast<float>(thumbSize - 2),
                               2.0f);
    }
}

//------------------------------------------------------------------------------
void BranchesLAF::drawMenuBarBackground(Graphics &g,
                                        int width,
                                        int height,
                                        bool isMouseOverBar,
                                        MenuBarComponent &menuBar)
{
    Colour col = ColourScheme::getInstance().colours["Window Background"];

    ColourGradient grad(col.brighter(0.8f),
                        0.0f,
                        0.0f,
                        col.darker(0.05f),
                        0.0f,
                        static_cast<float>(height),
                        false);

    g.setGradientFill(grad);
    g.fillRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    g.setColour(Colour(0x30000000));
    g.fillRect(0.0f, static_cast<float>(height) - 1.0f, static_cast<float>(width), 1.0f);
}

//------------------------------------------------------------------------------
Font BranchesLAF::getMenuBarFont(MenuBarComponent &menuBar,
                                 int itemIndex,
                                 const String &itemText)
{
    return Font(juce::FontOptions().withHeight(15.0f));
}

//------------------------------------------------------------------------------
void BranchesLAF::drawMenuBarItem(Graphics &g,
                                  int width,
                                  int height,
                                  int itemIndex,
                                  const String &itemText,
                                  bool isMouseOverItem,
                                  bool isMenuOpen,
                                  bool isMouseOverBar,
                                  MenuBarComponent &menuBar)
{
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    if (!menuBar.isEnabled())
    {
        g.setColour(colours["Text Colour"].withMultipliedAlpha(0.5f));
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        g.fillAll(colours["Menu Selection Colour"]);
        g.setColour(colours["Menu Selection Colour"].contrasting());
    }
    else
    {
        g.setColour(colours["Text Colour"]);
    }

    g.setFont(getMenuBarFont(menuBar, itemIndex, itemText));
    g.drawFittedText(itemText, 0, 0, width, height, Justification::centred, 1);
}

//------------------------------------------------------------------------------
int BranchesLAF::getMenuBarItemWidth(MenuBarComponent &menuBar,
                                     int itemIndex,
                                     const String &itemText)
{
    return juce::GlyphArrangement::getStringWidthInt(
               getMenuBarFont(menuBar, itemIndex, itemText), itemText)
           + menuBar.getHeight() - 8;
}

//------------------------------------------------------------------------------
void BranchesLAF::drawPopupMenuBackground(Graphics &g, int width, int height)
{
    Path highlight, shadow;

    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);

    highlight.startNewSubPath(2.0f, static_cast<float>(height - 3));
    highlight.lineTo(2.0f, 2.0f);
    highlight.lineTo(static_cast<float>(width), 2.0f);
    g.setColour(Colour(0xB0FFFFFF));
    g.strokePath(highlight, PathStrokeType(1.0f,
                                           PathStrokeType::curved,
                                           PathStrokeType::rounded));

    // Draw shadow.
    shadow.startNewSubPath(3.0f, static_cast<float>(height - 2));
    shadow.lineTo(static_cast<float>(width - 2),
                  static_cast<float>(height - 2));
    shadow.lineTo(static_cast<float>(width - 2), 3.0f);
    g.setColour(Colour(0x20000000));
    g.strokePath(shadow, PathStrokeType(1.0f,
                                        PathStrokeType::curved,
                                        PathStrokeType::rounded));

    g.setColour(Colour(0x60000000));
    g.drawRect(0, 0, width, height);
}

//------------------------------------------------------------------------------
const Drawable *BranchesLAF::getDefaultFolderImage()
{
    static DrawableImage im;

    if (im.getImage().isNull())
        im.setImage(ImageCache::getFromMemory(LookAndFeelImages::lookandfeelfolder_32_png,
                                              LookAndFeelImages::lookandfeelfolder_32_pngSize));

    return &im;
}

//------------------------------------------------------------------------------
void BranchesLAF::drawComboBox(Graphics &g,
                               int width,
                               int height,
                               bool isButtonDown,
                               int buttonX,
                               int buttonY,
                               int buttonW,
                               int buttonH,
                               ComboBox &box)
{
    float inc;
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    g.setColour(colours["Text Editor Colour"]);
    g.fillRect(0, 0, width - buttonW + 3, height);

    if (box.isEnabled() && box.hasKeyboardFocus(false))
    {
        g.setColour(colours["Button Colour"]);
        g.drawRect(0, 0, width, height, 2);
    }
    else
    {
        g.setColour(box.findColour(ComboBox::outlineColourId));
        g.drawRect(0, 0, width - buttonW + 3, height);
    }

    g.setColour(Colour(0x80000000));
    g.drawRoundedRectangle(static_cast<float>(buttonX) + 1.0f,
                           static_cast<float>(buttonY) + 1.0f,
                           static_cast<float>(buttonW - 2),
                           static_cast<float>(buttonH - 2),
                           2.0f,
                           1.0f);
    g.setColour(colours["Button Colour"]);
    g.fillRoundedRectangle(static_cast<float>(buttonX) + 1.0f,
                           static_cast<float>(buttonY) + 1.0f,
                           static_cast<float>(buttonW - 2),
                           static_cast<float>(buttonH - 2),
                           2.0f);

    g.setColour(Colour(0x08000000));
    g.fillRoundedRectangle(static_cast<float>(buttonX) + 1.0f,
                           static_cast<float>(buttonY) + static_cast<float>(buttonH / 2),
                           static_cast<float>(buttonW - 2),
                           static_cast<float>(buttonH - 2),
                           2.0f);

    if (isButtonDown)
        inc = 1.0f;
    else
        inc = 0.0f;

    if (box.isEnabled())
    {
        Path tri;

        tri.startNewSubPath(static_cast<float>(buttonX) + static_cast<float>(buttonW / 2) - static_cast<float>(buttonW / 4) + inc,
                            static_cast<float>(buttonY) + static_cast<float>(buttonH / 2) - static_cast<float>(buttonH / 8) + inc);
        tri.lineTo(static_cast<float>(buttonX) + static_cast<float>(buttonW / 2) + inc,
                   static_cast<float>(buttonY) + static_cast<float>(buttonH / 2) + static_cast<float>(buttonH / 8) + inc);
        tri.lineTo(static_cast<float>(buttonX) + static_cast<float>(buttonW / 2) + static_cast<float>(buttonW / 4) + inc,
                   static_cast<float>(buttonY) + static_cast<float>(buttonH / 2) - static_cast<float>(buttonH / 8) + inc);

        g.setColour(colours["Vector Colour"]);
        g.strokePath(tri, juce::PathStrokeType(2.0f));
    }
}

//------------------------------------------------------------------------------
void BranchesLAF::drawProgressBar(Graphics &g,
                                  ProgressBar &progressBar,
                                  int width,
                                  int height,
                                  double progress,
                                  const String &textToShow)
{
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;
    ColourGradient grad(colours["Window Background"].darker(0.2f),
                        0.0f,
                        0.0f,
                        colours["Window Background"],
                        0.0f,
                        static_cast<float>(height),
                        false);

    // Draw the background.
    g.setGradientFill(grad);
    g.fillRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    g.setColour(Colour(0x80000000));
    g.drawRect(0, 0, width, height, 1);

    // Draw the foreground.
    g.setColour(Colour(0x80000000));
    g.drawRoundedRectangle(1.0f,
                           1.0f,
                           static_cast<float>(width - 2) * static_cast<float>(progress),
                           static_cast<float>(height - 2),
                           2.0f,
                           1.0f);
    g.setColour(colours["CPU Meter Colour"]);
    g.fillRoundedRectangle(1.0f,
                           1.0f,
                           static_cast<float>(width - 2) * static_cast<float>(progress),
                           static_cast<float>(height - 2),
                           2.0f);

    g.setColour(Colour(0x08000000));
    g.fillRoundedRectangle(1.0f,
                           static_cast<float>(height / 2),
                           static_cast<float>(width - 2) * static_cast<float>(progress),
                           static_cast<float>(height - 2),
                           2.0f);

    // Draw the text.
    g.setColour(Colour(0x80000000));
    g.drawText(textToShow,
               2,
               1,
               width - 4,
               height - 2,
               Justification(Justification::horizontallyCentred),
               true);
}

//------------------------------------------------------------------------------
void BranchesLAF::drawKeymapChangeButton(Graphics &g,
                                         int width,
                                         int height,
                                         Button &button,
                                         const String &keyDescription)
{
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    if (keyDescription.isNotEmpty())
    {
        drawButtonBackground(g,
                             button,
                             colours["Button Colour"],
                             button.isOver(),
                             button.isDown());

        g.setColour(colours["Text Colour"]);
        g.setFont(static_cast<float>(height) * 0.6f);
        g.drawFittedText(keyDescription,
                         3,
                         0,
                         width - 6,
                         height,
                         Justification::centred,
                         1);
    }
    else
    {
        const float thickness = 7.0f;
        const float indent = 22.0f;

        Path p;
        p.addEllipse(0.0f, 0.0f, 100.0f, 100.0f);
        p.addRectangle(indent,
                       50.0f - thickness,
                       100.0f - indent * 2.0f,
                       thickness * 2.0f);
        p.addRectangle(50.0f - thickness,
                       indent,
                       thickness * 2.0f,
                       50.0f - indent - thickness);
        p.addRectangle(50.0f - thickness,
                       50.0f + thickness,
                       thickness * 2.0f,
                       50.0f - indent - thickness);
        p.setUsingNonZeroWinding(false);

        g.setColour(colours["Text Colour"].withAlpha(button.isDown() ? 0.7f : (button.isOver() ? 0.5f : 0.3f)));
        g.fillPath(p,
                   p.getTransformToScaleToFit(2.0f,
                                              2.0f,
                                              static_cast<float>(width) - 4.0f,
                                              static_cast<float>(height) - 4.0f,
                                              true));
    }
}

//------------------------------------------------------------------------------
void BranchesLAF::drawLabel(Graphics &g, Label &label)
{
    g.fillAll(label.findColour(Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        const float alpha = label.isEnabled() ? 1.0f : 0.5f;

        g.setColour(ColourScheme::getInstance().colours["Text Colour"]);
        g.setFont(label.getFont());
        auto labelBorder = label.getBorderSize();
        g.drawFittedText(label.getText(),
                         labelBorder.getLeft(),
                         labelBorder.getTop(),
                         label.getWidth() - labelBorder.getLeftAndRight(),
                         label.getHeight() - labelBorder.getTopAndBottom(),
                         label.getJustificationType(),
                         jmax(1, static_cast<int>(label.getHeight() / label.getFont().getHeight())),
                         label.getMinimumHorizontalScale());

        g.setColour(label.findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
        g.drawRect(0, 0, label.getWidth(), label.getHeight());
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(Label::outlineColourId));
        g.drawRect(0, 0, label.getWidth(), label.getHeight());
    }
}

//------------------------------------------------------------------------------
void BranchesLAF::drawToggleButton(Graphics &g,
                                   ToggleButton &button,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    if (button.hasKeyboardFocus(true))
    {
        g.setColour(ColourScheme::getInstance().colours["List Selected Colour"]);
        g.drawRect(0, 0, button.getWidth(), button.getHeight());
    }

    float fontSize = jmin(15.0f, static_cast<float>(button.getHeight()) * 0.75f);
    const float tickWidth = fontSize * 1.1f;

    drawTickBox(g, button, 4.0f, (static_cast<float>(button.getHeight()) - tickWidth) * 0.5f,
                tickWidth, tickWidth,
                button.getToggleState(),
                button.isEnabled(),
                shouldDrawButtonAsHighlighted,
                shouldDrawButtonAsDown);

    g.setColour(ColourScheme::getInstance().colours["Text Colour"]);
    g.setFont(fontSize);

    if (!button.isEnabled())
        g.setOpacity(0.5f);

    const int textX = static_cast<int>(tickWidth) + 5;

    g.drawFittedText(button.getButtonText(),
                     textX, 0,
                     button.getWidth() - textX - 2, button.getHeight(),
                     Justification::centredLeft, 10);
}

//------------------------------------------------------------------------------
void BranchesLAF::drawTickBox(Graphics &g,
                              Component &component,
                              float x, float y, float w, float h,
                              bool ticked,
                              bool isEnabled,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown)
{
    const float boxSize = w * 0.7f;

    juce::LookAndFeel_V2::drawGlassSphere(g, x, y + (h - boxSize) * 0.5f, boxSize,
                    ColourScheme::getInstance().colours["Tick Box Colour"].withMultipliedAlpha(isEnabled ? 1.0f : 0.5f),
                    isEnabled ? ((shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted) ? 1.1f : 0.5f) : 0.3f);

    if (ticked)
    {
        Path tick;
        Colour tempCol = ColourScheme::getInstance().colours["Vector Colour"];
        tick.startNewSubPath(1.5f, 3.0f);
        tick.lineTo(3.0f, 6.0f);
        tick.lineTo(6.0f, 0.0f);

        g.setColour(isEnabled ? tempCol : tempCol.withAlpha(0.25f));

        const AffineTransform trans(AffineTransform::scale(w / 9.0f, h / 9.0f)
                                        .translated(x, y));

        g.strokePath(tick, PathStrokeType(2.5f), trans);
    }
}

//------------------------------------------------------------------------------
void BranchesLAF::fillTextEditorBackground(Graphics &g, int /*width*/, int /*height*/,
                                           TextEditor &textEditor)
{
    g.fillAll(ColourScheme::getInstance().colours["Text Editor Colour"]);
}

//------------------------------------------------------------------------------
void BranchesLAF::drawCallOutBoxBackground(CallOutBox &box,
                                           Graphics &g,
                                           const Path &path,
                                           Image &cachedImage)
{
    Image content(Image::ARGB, box.getWidth(), box.getHeight(), true);

    {
        Graphics g2(content);

        g2.setColour(ColourScheme::getInstance().colours["Window Background"].withAlpha(0.9f));
        g2.fillPath(path);

        g2.setColour(Colours::black.withAlpha(0.8f));
        g2.strokePath(path, PathStrokeType(2.0f));
    }

    DropShadowEffect shadow;
    DropShadow shad(Colours::black.withAlpha(0.5f), 5, Point<int>(2, 2));
    shadow.setShadowProperties(shad);
    shadow.applyEffect(content, g, 1.0f, 1.0f);
}
