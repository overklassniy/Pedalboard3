// ColourSchemeEditor.cpp - Editor for the colour scheme.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
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

#include "Vectors.h"
#include "ColourScheme.h"
#include "PropertiesSingleton.h"
#include "JuceHelperStuff.h"

#include "ColourSchemeEditor.h"

//==============================================================================
ColourSchemeEditor::ColourSchemeEditor()
{
    colourEditor = std::make_unique<ColourSelector>(
        ColourSelector::showAlphaChannel | ColourSelector::showColourAtTop |
            ColourSelector::showSliders | ColourSelector::showColourspace,
        0);
    addAndMakeVisible(*colourEditor);
    colourEditor->setName("colourEditor");

    colourSelector = std::make_unique<ListBox>("colourSelector", this);
    addAndMakeVisible(*colourSelector);
    colourSelector->setName("colourSelector");

    presetSelector = std::make_unique<ComboBox>("presetSelector");
    addAndMakeVisible(*presetSelector);
    presetSelector->setEditableText(true);
    presetSelector->setJustificationType(Justification::centredLeft);
    presetSelector->setTextWhenNothingSelected({});
    presetSelector->setTextWhenNoChoicesAvailable("(no choices)");
    presetSelector->addListener(this);

    deleteButton = std::make_unique<DrawableButton>("deleteButton",
                                                     DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*deleteButton);
    deleteButton->setName("deleteButton");

    saveButton = std::make_unique<DrawableButton>("saveButton",
                                                   DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*saveButton);
    saveButton->setName("saveButton");

    newButton = std::make_unique<DrawableButton>("newButton",
                                                  DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*newButton);
    newButton->setName("newButton");

    Colour tempCol = ColourScheme::getInstance().colours["Button Colour"];
    std::unique_ptr<Drawable> newImage(loadSVGFromMemory(Vectors::newbutton_svg,
                                                         Vectors::newbutton_svgSize));
    std::unique_ptr<Drawable> saveImage(loadSVGFromMemory(Vectors::savebutton_svg,
                                                          Vectors::savebutton_svgSize));
    std::unique_ptr<Drawable> deleteImage(loadSVGFromMemory(Vectors::deletebutton_svg,
                                                            Vectors::deletebutton_svgSize));

    newButton->setImages(newImage.get());
    newButton->setColour(DrawableButton::backgroundColourId, tempCol);
    newButton->setColour(DrawableButton::backgroundOnColourId, tempCol);
    newButton->setTooltip("New colour scheme");
    saveButton->setImages(saveImage.get());
    saveButton->setColour(DrawableButton::backgroundColourId, tempCol);
    saveButton->setColour(DrawableButton::backgroundOnColourId, tempCol);
    saveButton->setTooltip("Save current colour scheme");
    deleteButton->setImages(deleteImage.get());
    deleteButton->setColour(DrawableButton::backgroundColourId, tempCol);
    deleteButton->setColour(DrawableButton::backgroundOnColourId, tempCol);
    deleteButton->setTooltip("Delete selected colour scheme");

    colourEditor->setColour(ColourSelector::backgroundColourId, tempCol);

    colourSelector->setOutlineThickness(1);
    colourSelector->setColour(ListBox::outlineColourId, Colour(0x60000000));

    colourSelector->updateContent();
    colourSelector->selectRow(0);
    colourEditor->setCurrentColour(ColourScheme::getInstance().colours.begin()->second);
    colourEditor->addChangeListener(this);

    // Fill out the preset combo box.
    Array<File> presets;
    File settingsDir = JuceHelperStuff::getAppDataFolder();

    settingsDir.findChildFiles(presets, File::findFiles, false, "*.colourscheme");
    for (int i = 0; i < presets.size(); ++i)
    {
        String tempstr = presets[i].getFileNameWithoutExtension();

        presetSelector->addItem(tempstr, i + 1);
        if (tempstr == ColourScheme::getInstance().presetName)
            presetSelector->setSelectedId(i + 1, true);
    }

    newButton->addListener(this);
    saveButton->addListener(this);
    deleteButton->addListener(this);

    setSize(550, 375);
}

//------------------------------------------------------------------------------

ColourSchemeEditor::~ColourSchemeEditor()
{
    // Check if the selected preset has been saved.
    if (!ColourScheme::getInstance().doesColoursMatchPreset(presetSelector->getText()))
    {
        if (AlertWindow::showOkCancelBox(MessageBoxIconType::WarningIcon,
                                         "Colour scheme not saved",
                                         "Save current scheme?",
                                         "Yes",
                                         "No"))
        {
            ColourScheme::getInstance().savePreset(presetSelector->getText());
        }
    }

    // Save the selected preset to the properties file.
    PropertiesSingleton::getInstance().getUserSettings()->setValue("colourScheme",
                                                                   presetSelector->getText());
}

//------------------------------------------------------------------------------

void ColourSchemeEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

//------------------------------------------------------------------------------

void ColourSchemeEditor::resized()
{
    colourEditor->setBounds(192, 40, getWidth() - 200, getHeight() - 48);
    colourSelector->setBounds(8, 40, 176, getHeight() - 48);
    presetSelector->setBounds(8, 8, getWidth() - 106, 24);
    deleteButton->setBounds(getWidth() - 32, 8, 24, 24);
    saveButton->setBounds(getWidth() - 62, 8, 24, 24);
    newButton->setBounds(getWidth() - 92, 8, 24, 24);
}

//------------------------------------------------------------------------------

void ColourSchemeEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == presetSelector.get())
    {
        ColourScheme::getInstance().loadPreset(presetSelector->getText());

        // Update colourEditor.
        {
            int i = 0;
            std::map<String, Colour>::iterator it;
            int row = colourSelector->getSelectedRow();
            std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

            for (it = colours.begin(); it != colours.end(); ++it, ++i)
            {
                if (i == row)
                {
                    colourEditor->setCurrentColour(it->second);
                    break;
                }
            }
        }

        repaint();
    }
}

//------------------------------------------------------------------------------

int ColourSchemeEditor::getNumRows()
{
    return static_cast<int>(ColourScheme::getInstance().colours.size());
}

//------------------------------------------------------------------------------

void ColourSchemeEditor::paintListBoxItem(int rowNumber,
                                          Graphics& g,
                                          int width,
                                          int height,
                                          bool rowIsSelected)
{
    int i = 0;
    std::map<String, Colour>::iterator it;
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    g.fillAll(colours["Dialog Inner Background"]);

    for (it = colours.begin(); it != colours.end(); ++it, ++i)
    {
        if (i == rowNumber)
        {
            // Colour in the background.
            if (rowIsSelected)
            {
                ColourGradient basil(colours["List Selected Colour"].brighter(0.4f),
                                     0.0f,
                                     0.0f,
                                     colours["List Selected Colour"].darker(0.125f),
                                     0.0f,
                                     static_cast<float>(height),
                                     false);

                g.setGradientFill(basil);

                g.fillAll();

                g.setColour(colours["List Selected Colour"].contrasting());
            }
            else
            {
                g.fillAll(it->second);

                g.setColour(it->second.contrasting());
            }

            // And draw the colour's name.
            g.drawSingleLineText(it->first, 4, 12);

            break;
        }
    }
}

//------------------------------------------------------------------------------

void ColourSchemeEditor::listBoxItemClicked(int row, const MouseEvent& e)
{
    (void) e;

    int i = 0;
    std::map<String, Colour>::iterator it;
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    for (it = colours.begin(); it != colours.end(); ++it, ++i)
    {
        if (i == row)
        {
            colourEditor->setCurrentColour(it->second);
            break;
        }
    }
}

//------------------------------------------------------------------------------

void ColourSchemeEditor::buttonClicked(Button* button)
{
    if (button == newButton.get())
    {
        presetSelector->addItem("New Colour Scheme", presetSelector->getNumItems() + 1);
        presetSelector->setSelectedId(presetSelector->getNumItems());
    }
    else if (button == saveButton.get())
    {
        ColourScheme::getInstance().savePreset(presetSelector->getText());
    }
    else if (button == deleteButton.get())
    {
        if (presetSelector->getNumItems() > 1)
        {
            File tempFile;
            String tempstr;
            StringArray presetsArray;
            String presetName = presetSelector->getText();
            File settingsDir = JuceHelperStuff::getAppDataFolder();

            tempstr << presetName << ".colourscheme";
            tempFile = settingsDir.getChildFile(tempstr);
            tempFile.deleteFile();

            if (presetSelector->getSelectedId() > 1)
            {
                for (int i = 0; i < presetSelector->getNumItems(); ++i)
                {
                    tempstr = presetSelector->getItemText(i);
                    if (presetSelector->getText() != tempstr)
                        presetsArray.add(tempstr);
                }

                presetSelector->clear();
                for (int i = 0; i < presetsArray.size(); ++i)
                    presetSelector->addItem(presetsArray[i], i + 1);
                presetSelector->setSelectedId(1);
            }
        }
    }
}

//------------------------------------------------------------------------------

void ColourSchemeEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == colourEditor.get())
    {
        int i = 0;
        std::map<String, Colour>::iterator it;
        int row = colourSelector->getSelectedRow();
        std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

        for (it = colours.begin(); it != colours.end(); ++it, ++i)
        {
            if (i == row)
            {
                Colour tempCol = ColourScheme::getInstance().colours["Button Colour"];

                it->second = colourEditor->getCurrentColour();
                colourSelector->updateContent();
                newButton->setColour(DrawableButton::backgroundColourId, tempCol);
                newButton->setColour(DrawableButton::backgroundOnColourId, tempCol);
                saveButton->setColour(DrawableButton::backgroundColourId, tempCol);
                saveButton->setColour(DrawableButton::backgroundOnColourId, tempCol);
                deleteButton->setColour(DrawableButton::backgroundColourId, tempCol);
                deleteButton->setColour(DrawableButton::backgroundOnColourId, tempCol);
                colourEditor->setColour(ColourSelector::backgroundColourId,
                                        ColourScheme::getInstance().colours["Window Background"]);
                repaint();
                sendChangeMessage();

                break;
            }
        }
    }
}

//------------------------------------------------------------------------------

Drawable* ColourSchemeEditor::loadSVGFromMemory(const void* dataToInitialiseFrom,
                                                size_t sizeInBytes)
{
    Drawable* retval = nullptr;

    MemoryBlock memBlock(dataToInitialiseFrom, sizeInBytes);
    XmlDocument doc(memBlock.toString());
    std::unique_ptr<XmlElement> svgData(doc.getDocumentElement());

    retval = Drawable::createFromSVG(*svgData).release();

    return retval;
}
