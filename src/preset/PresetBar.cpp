// PresetBar.cpp - Preset bar component for individual plugins.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2012 Niall Moody.
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

#include "PresetBar.h"

#include "ColourScheme.h"
#include "PluginComponent.h"
#include "PresetManager.h"
#include "Vectors.h"

PresetBar::PresetBar(PluginComponent* comp) : component(comp), lastComboBox(0) {
    presetsComboBox = std::make_unique<ComboBox>("presetsComboBox");
    addAndMakeVisible(*presetsComboBox);
    presetsComboBox->setEditableText(true);
    presetsComboBox->setJustificationType(Justification::centredLeft);
    presetsComboBox->setTextWhenNothingSelected({});
    presetsComboBox->setTextWhenNoChoicesAvailable("(no choices)");
    presetsComboBox->addListener(this);

    presetsLabel = std::make_unique<Label>("presetsLabel", "Presets:");
    addAndMakeVisible(*presetsLabel);
    presetsLabel->setFont(juce::FontOptions().withHeight(15.0f));
    presetsLabel->setJustificationType(Justification::centredLeft);
    presetsLabel->setEditable(false, false, false);
    presetsLabel->setColour(TextEditor::textColourId, Colours::black);
    presetsLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    importButton = std::make_unique<DrawableButton>("importButton", DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*importButton);
    importButton->setName("importButton");

    saveButton = std::make_unique<DrawableButton>("saveButton", DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*saveButton);
    saveButton->setName("saveButton");

    AudioProcessor* proc;

    Colour tempCol = ColourScheme::getInstance().colours["Button Colour"];
    std::unique_ptr<Drawable> saveImage(loadSVGFromMemory(Vectors::savebutton_svg, Vectors::savebutton_svgSize));
    std::unique_ptr<Drawable> openImage(loadSVGFromMemory(Vectors::openbutton_svg, Vectors::openbutton_svgSize));

    proc = component->getNode()->getProcessor();

    saveButton->setImages(saveImage.get());
    saveButton->setColour(DrawableButton::backgroundColourId, tempCol);
    saveButton->setColour(DrawableButton::backgroundOnColourId, tempCol);
    saveButton->setTooltip("Save current preset");
    saveButton->addListener(this);
    importButton->setImages(openImage.get());
    importButton->setColour(DrawableButton::backgroundColourId, tempCol);
    importButton->setColour(DrawableButton::backgroundOnColourId, tempCol);
    importButton->setTooltip("Import preset from .fxp file");
    importButton->addListener(this);

    fillOutComboBox();
    presetsComboBox->setSelectedId(proc->getCurrentProgram() + 1, true);
    lastComboBox = presetsComboBox->getSelectedId();

    setSize(396, 32);
}

PresetBar::~PresetBar() {}

void PresetBar::paint(Graphics& g) {
    g.fillAll(Colour(0xffeeece1));

    g.setColour(Colour(0xff2aa545));
    g.fillPath(internalPath1);
    g.setColour(Colour(0x20000000));
    g.strokePath(internalPath1, PathStrokeType(1.0f));
}

void PresetBar::resized() {
    presetsComboBox->setBounds(64, 4, 272, 24);
    presetsLabel->setBounds(0, 4, 64, 24);
    importButton->setBounds(340, 4, 24, 24);
    saveButton->setBounds(368, 4, 24, 24);
    internalPath1.clear();
    internalPath1.startNewSubPath(0.0f, 32.0f);
    internalPath1.lineTo(static_cast<float>(getWidth()), 32.0f);
    internalPath1.closeSubPath();
}

void PresetBar::comboBoxChanged(ComboBox* comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == presetsComboBox.get()) {
        int index = presetsComboBox->getSelectedItemIndex();
        AudioProcessor* proc = component->getNode()->getProcessor();

        if ((index > -1) && (index < proc->getNumPrograms())) {
            MemoryBlock cachedPreset;

            component->getCachedPreset(index, cachedPreset);

            proc->setCurrentProgram(index);

            // If we have a cached preset for this program, load it.
            if (cachedPreset.getSize() > 0) {
                proc->setCurrentProgramStateInformation(cachedPreset.getData(),
                                                        static_cast<int>(cachedPreset.getSize()));
            }

            lastComboBox = index + 1;
        } else if (index >= proc->getNumPrograms()) {
            PresetManager manager;

            // Only cache plugin presets, not user ones.
            if ((lastComboBox - 1) < proc->getNumPrograms())
                component->cacheCurrentPreset();

            manager.importPreset(presetsComboBox->getText(), proc);

            lastComboBox = index + 1;
        } else {
            proc->changeProgramName(proc->getCurrentProgram(), presetsComboBox->getText());
            presetsComboBox->changeItemText(lastComboBox, presetsComboBox->getText());
        }
    }
}

void PresetBar::buttonClicked(Button* button) {
    if (button == importButton.get()) {
        FileChooser dlg("Select an .fxp file to import...", File(), "*.fxp");

        if (dlg.browseForFileToOpen()) {
            PresetManager manager;
            AudioProcessor* proc = component->getNode()->getProcessor();

            manager.importPreset(dlg.getResult(), proc);
            presetsComboBox->setText(proc->getProgramName(proc->getCurrentProgram()), true);
        }
    } else if (button == saveButton.get()) {
        int currentId;
        MemoryBlock memBlock;
        PresetManager manager;
        AudioProcessor* proc = component->getNode()->getProcessor();

        proc->getCurrentProgramStateInformation(memBlock);
        manager.savePreset(memBlock, presetsComboBox->getText(), proc->getName());

        currentId = presetsComboBox->getSelectedId();
        fillOutComboBox();
        presetsComboBox->setSelectedId(currentId, true);
    }
}

void PresetBar::fillOutComboBox() {
    int j;
    StringArray userPresets;
    AudioProcessor* proc = component->getNode()->getProcessor();

    presetsComboBox->clear(true);

    j = 1;
    for (int i = 0; i < proc->getNumPrograms(); ++i, ++j) {
        String tempstr = proc->getProgramName(i);
        if (tempstr.isEmpty())
            tempstr = " ";
        presetsComboBox->addItem(tempstr, j);
    }

    PresetManager::getListOfUserPresets(proc->getName(), userPresets);

    for (int i = 0; i < userPresets.size(); ++i, ++j)
        presetsComboBox->addItem(userPresets[i], j);
}

Drawable* PresetBar::loadSVGFromMemory(const void* dataToInitialiseFrom, size_t sizeInBytes) {
    Drawable* retval = nullptr;

    MemoryBlock memBlock(dataToInitialiseFrom, sizeInBytes);
    XmlDocument doc(memBlock.toString());
    std::unique_ptr<XmlElement> svgData(doc.getDocumentElement());

    retval = Drawable::createFromSVG(*svgData).release();

    return retval;
}
