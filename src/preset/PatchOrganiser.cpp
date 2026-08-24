// PatchOrganiser.cpp - Patch management and navigation component.
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

#include "PatchOrganiser.h"

#include "ColourScheme.h"
#include "MainPanel.h"

PatchOrganiser::PatchOrganiser(MainPanel* panel, Array<XmlElement*>& patchArray)
    : mainPanel(panel), patches(patchArray) {
    patchList = std::make_unique<ListBox>("patchList", this);
    addAndMakeVisible(*patchList);
    patchList->setName("patchList");

    addButton = std::make_unique<TextButton>("addButton");
    addAndMakeVisible(*addButton);
    addButton->setButtonText("Add");
    addButton->addListener(this);

    copyButton = std::make_unique<TextButton>("copyButton");
    addAndMakeVisible(*copyButton);
    copyButton->setButtonText("Copy");
    copyButton->addListener(this);

    removeButton = std::make_unique<TextButton>("removeButton");
    addAndMakeVisible(*removeButton);
    removeButton->setButtonText("Remove");
    removeButton->addListener(this);

    moveUpButton = std::make_unique<TextButton>("moveUpButton");
    addAndMakeVisible(*moveUpButton);
    moveUpButton->setButtonText("Move Up");
    moveUpButton->addListener(this);

    moveDownButton = std::make_unique<TextButton>("moveDownButton");
    addAndMakeVisible(*moveDownButton);
    moveDownButton->setButtonText("Move Down");
    moveDownButton->addListener(this);

    importButton = std::make_unique<TextButton>("importButton");
    addAndMakeVisible(*importButton);
    importButton->setButtonText("Import...");
    importButton->addListener(this);

    addButton->setTooltip("Add a new patch");
    copyButton->setTooltip("Duplicate selected patch");
    removeButton->setTooltip("Delete selected patch");
    moveUpButton->setTooltip("Move patch up");
    moveDownButton->setTooltip("Move patch down");
    importButton->setTooltip("Import patch from another .pdl file");

    patchList->setOutlineThickness(1);
    patchList->setMultipleSelectionEnabled(true);
    patchList->setColour(ListBox::backgroundColourId, ColourScheme::getInstance().colours["Dialog Inner Background"]);

    setSize(600, 400);
}

PatchOrganiser::~PatchOrganiser() {}

void PatchOrganiser::paint(Graphics& g) {
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);

    g.setColour(Colour(0x40000000));
    g.fillRect(getWidth() - 89, 140, 80, 1);
}

void PatchOrganiser::resized() {
    patchList->setBounds(8, 8, getWidth() - 106, getHeight() - 15);
    addButton->setBounds(getWidth() - 90, 8, 82, 24);
    copyButton->setBounds(getWidth() - 90, 40, 82, 24);
    removeButton->setBounds(getWidth() - 90, 72, 82, 24);
    importButton->setBounds(getWidth() - 90, 104, 82, 24);
    moveUpButton->setBounds(getWidth() - 90, 152, 82, 24);
    moveDownButton->setBounds(getWidth() - 90, 184, 82, 24);
}

void PatchOrganiser::buttonClicked(Button* buttonThatWasClicked) {
    int i, j;

    if (buttonThatWasClicked == addButton.get()) {
        ComboBox* patchComboBox = mainPanel->getPatchComboBox();

        // We call comboBoxChanged() directly because ComboBox uses an
        // AsyncUpdater to notify listeners, so comboBoxChanged() would
        // not fire before patchList->updateContent() is called below.
        patchComboBox->setSelectedId(patchComboBox->getNumItems(), false);
        mainPanel->comboBoxChanged(patchComboBox);

        patchList->updateContent();
        repaint();
    } else if (buttonThatWasClicked == copyButton.get()) {
        for (i = 0; i < patchList->getNumSelectedRows(); ++i) {
            mainPanel->duplicatePatch(patchList->getSelectedRow(i));

            patchList->updateContent();
            repaint();
        }
    } else if (buttonThatWasClicked == removeButton.get()) {
        ComboBox* patchComboBox = mainPanel->getPatchComboBox();

        if (patches.size() > 1) {
            for (i = (patchList->getNumSelectedRows() - 1); i >= 0; --i) {
                // Make sure the user can't delete the last patch.
                if (patches.size() == 1)
                    break;

                j = patchList->getSelectedRow(i);

                // Switch the active patch if we're deleting it.
                if (patchComboBox->getSelectedItemIndex() == j) {
                    if (j > 0) {
                        patchComboBox->setSelectedItemIndex(j - 1, true);
                        mainPanel->comboBoxChanged(patchComboBox);
                    } else {
                        patchComboBox->setSelectedItemIndex(j + 1, true);
                        mainPanel->comboBoxChanged(patchComboBox);
                    }
                }

                delete patches[j];
                patches.remove(j);
            }

            patchComboBox->clear(true);
            for (i = 0; i < patches.size(); ++i)
                patchComboBox->addItem(patches[i]->getStringAttribute("name"), i + 1);
            patchComboBox->addItem("<new patch>", patches.size() + 1);
            patchComboBox->setSelectedId(1);

            patchList->updateContent();
            repaint();
        }
    } else if (buttonThatWasClicked == moveUpButton.get()) {
        ComboBox* patchComboBox = mainPanel->getPatchComboBox();

        XmlElement* e1;
        XmlElement* e2;
        int tempint;

        tempint = patchList->getSelectedRow(0);

        if (tempint > 0) {
            e1 = patches[tempint];
            e2 = patches[tempint - 1];

            patches.set(tempint - 1, e1);
            patches.set(tempint, e2);

            patchList->selectRow(patchList->getSelectedRow(0) - 1);
        }

        patchComboBox->clear(true);
        for (i = 0; i < patches.size(); ++i)
            patchComboBox->addItem(patches[i]->getStringAttribute("name"), i + 1);
        patchComboBox->addItem("<new patch>", patches.size() + 1);
        mainPanel->nextSwitchDoNotSavePrev();
        patchComboBox->setSelectedId(1);

        patchList->updateContent();
        repaint();
    } else if (buttonThatWasClicked == moveDownButton.get()) {
        ComboBox* patchComboBox = mainPanel->getPatchComboBox();

        XmlElement* e1;
        XmlElement* e2;
        int tempint;

        tempint = patchList->getSelectedRow(0);

        if (tempint < (patches.size() - 1)) {
            e1 = patches[tempint];
            e2 = patches[tempint + 1];

            patches.set(tempint + 1, e1);
            patches.set(tempint, e2);

            patchList->selectRow(patchList->getSelectedRow(0) + 1);
        }

        patchComboBox->clear(true);
        for (i = 0; i < patches.size(); ++i)
            patchComboBox->addItem(patches[i]->getStringAttribute("name"), i + 1);
        patchComboBox->addItem("<new patch>", patches.size() + 1);
        mainPanel->nextSwitchDoNotSavePrev();
        patchComboBox->setSelectedId(1);

        patchList->updateContent();
        repaint();
    } else if (buttonThatWasClicked == importButton.get()) {
        FileChooser phil("Select file to import patch from...", File(), "*.pdl");

        if (phil.browseForFileToOpen()) {
            File philResult = phil.getResult();
            XmlDocument doc(philResult);
            std::unique_ptr<XmlElement> root(doc.getDocumentElement());

            if (root) {
                String tempstr;
                StringArray patchNames;
                XmlElement* tempEl = nullptr;

                forEachXmlChildElementWithTagName(*root, tempEl2, "Patch") {
                    patchNames.add(tempEl2->getStringAttribute("name"));
                }

                tempstr << "Patches in file: " << philResult.getFileName();
                AlertWindow win("Select patch...", tempstr, MessageBoxIconType::NoIcon);

                win.addComboBox("patchName", patchNames);
                win.addButton("OK", 1, KeyPress(KeyPress::returnKey));
                win.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

                if (win.runModalLoop()) {
                    int index = win.getComboBoxComponent("patchName")->getSelectedItemIndex();
                    forEachXmlChildElementWithTagName(*root, tempEl2, "Patch") {
                        if (tempEl2->getStringAttribute("name") == patchNames[index]) {
                            tempEl = tempEl2;
                            break;
                        }
                    }

                    XmlElement* newPatch = new XmlElement(*tempEl);

                    mainPanel->addPatch(newPatch);

                    patchList->updateContent();
                    repaint();
                }
            }
        }
    }
}

int PatchOrganiser::getNumRows() {
    return patches.size();
}

void PatchOrganiser::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) {
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    if (rowIsSelected) {
        ColourGradient basil(colours["List Selected Colour"].brighter(0.4f), 0.0f, 0.0f,
                             colours["List Selected Colour"].darker(0.125f), 0.0f, static_cast<float>(height), false);

        g.setGradientFill(basil);

        g.fillAll();
    } else if (rowNumber % 2)
        g.fillAll(Colour(0x10000000));
}

Component* PatchOrganiser::refreshComponentForRow(int rowNumber, bool isRowSelected,
                                                  Component* existingComponentToUpdate) {
    String tempstr;
    Label* retval = dynamic_cast<Label*>(existingComponentToUpdate);

    if (rowNumber >= patches.size()) {
        if (existingComponentToUpdate)
            delete existingComponentToUpdate;
        return nullptr;
    }

    if (!retval) {
        retval = new Label();
        // Double-click to edit, single-click to select.
        retval->setEditable(false, true);
        retval->addListener(this);
        retval->setInterceptsMouseClicks(false, true);
    }

    retval->setText(patches[rowNumber]->getStringAttribute("name"), dontSendNotification);
    tempstr << rowNumber;
    // So we know which patch to update in labelTextChanged().
    retval->setName(tempstr);

    if (isRowSelected)
        retval->setColour(Label::textColourId, Colour(0xFFFFFFFF));
    else
        retval->setColour(Label::textColourId, Colour(0xFF000000));

    return retval;
}

void PatchOrganiser::listBoxItemClicked(int row, const MouseEvent& e) {
    patchList->selectRow(row, false, !e.mods.isCtrlDown());

    if (e.getNumberOfClicks() == 2)
        dynamic_cast<Label*>(patchList->getComponentForRowNumber(row))->showEditor();
}

void PatchOrganiser::backgroundClicked(const MouseEvent&) {
    patchList->deselectAllRows();
}

void PatchOrganiser::labelTextChanged(Label* labelThatHasChanged) {
    int index = labelThatHasChanged->getName().getIntValue();
    ComboBox* patchComboBox = mainPanel->getPatchComboBox();

    if ((index > -1) && (index < patches.size())) {
        patches[index]->setAttribute("name", labelThatHasChanged->getText());
        patchComboBox->changeItemText(index + 1, labelThatHasChanged->getText());
        if (patchComboBox->getSelectedItemIndex() == index)
            patchComboBox->setSelectedId(index + 1);
    }
}
