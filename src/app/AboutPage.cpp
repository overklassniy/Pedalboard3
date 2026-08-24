// AboutPage.cpp - About dialog component.
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

#include "AboutPage.h"

#include "ColourScheme.h"

AboutPage::AboutPage(const juce::String& ip) : ipAddress(ip) {
    titleLabel = std::make_unique<juce::Label>("titleLabel", "Pedalboard 3");
    addAndMakeVisible(*titleLabel);
    titleLabel->setFont(juce::Font(juce::FontOptions().withHeight(32.0f).withStyle("Bold")));
    titleLabel->setJustificationType(juce::Justification::centredLeft);
    titleLabel->setEditable(false, false, false);
    titleLabel->setColour(juce::Label::textColourId, juce::Colour(0x80000000));
    titleLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    titleLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x0));

    descriptionLabel = std::make_unique<juce::Label>("descriptionLabel",
                                                     "A simple plugin host intended for live use, with plugin\n"
                                                     "parameters easily mapped to MIDI or Open Sound Control inputs.");
    addAndMakeVisible(*descriptionLabel);
    descriptionLabel->setFont(juce::Font(juce::FontOptions().withHeight(15.0f)));
    descriptionLabel->setJustificationType(juce::Justification::topLeft);
    descriptionLabel->setEditable(false, false, false);
    descriptionLabel->setColour(juce::Label::textColourId, juce::Colour(0x80000000));
    descriptionLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    descriptionLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x0));

    creditsLabel =
        std::make_unique<juce::Label>("creditsLabel", "Written using the JUCE library, with sections taken from\n"
                                                      "the 'audio plugin host' example code.");
    addAndMakeVisible(*creditsLabel);
    creditsLabel->setFont(juce::Font(juce::FontOptions().withHeight(15.0f)));
    creditsLabel->setJustificationType(juce::Justification::topLeft);
    creditsLabel->setEditable(false, false, false);
    creditsLabel->setColour(juce::Label::textColourId, juce::Colour(0x80000000));
    creditsLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    creditsLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x0));

    authorLabel = std::make_unique<juce::Label>("authorLabel", "Author: Niall Moody\n"
                                                               "License: GPL v3");
    addAndMakeVisible(*authorLabel);
    authorLabel->setFont(juce::Font(juce::FontOptions().withHeight(15.0f)));
    authorLabel->setJustificationType(juce::Justification::topLeft);
    authorLabel->setEditable(false, false, false);
    authorLabel->setColour(juce::Label::textColourId, juce::Colour(0x80000000));
    authorLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    authorLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x0));

    niallmoodyLink = std::make_unique<juce::HyperlinkButton>("niallmoody.com", juce::URL("http://www.niallmoody.com"));
    addAndMakeVisible(*niallmoodyLink);
    niallmoodyLink->setTooltip("http://www.niallmoody.com");
    niallmoodyLink->setButtonText("niallmoody.com");

    juceLink = std::make_unique<juce::HyperlinkButton>("rawmaterialsoftware.com/juce",
                                                       juce::URL("http://www.rawmaterialsoftware.com/juce"));
    addAndMakeVisible(*juceLink);
    juceLink->setTooltip("http://www.rawmaterialsoftware.com/juce");
    juceLink->setButtonText("rawmaterialsoftware.com/juce");

    versionLabel = std::make_unique<juce::Label>("versionLabel", "Version: 3.0.0");
    addAndMakeVisible(*versionLabel);
    versionLabel->setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
    versionLabel->setJustificationType(juce::Justification::centredRight);
    versionLabel->setEditable(false, false, false);
    versionLabel->setColour(juce::Label::textColourId, juce::Colour(0x80000000));
    versionLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    versionLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x0));

    juceVersionLabel = std::make_unique<juce::Label>("juceVersionLabel", "JUCE Version: 8.0.15");
    addAndMakeVisible(*juceVersionLabel);
    juceVersionLabel->setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
    juceVersionLabel->setJustificationType(juce::Justification::centredRight);
    juceVersionLabel->setEditable(false, false, false);
    juceVersionLabel->setColour(juce::Label::textColourId, juce::Colour(0x80000000));
    juceVersionLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    juceVersionLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x0));

    ipAddressLabel = std::make_unique<juce::Label>("ipAddressLabel", "Current IP Address: 192.168.1.68");
    addAndMakeVisible(*ipAddressLabel);
    ipAddressLabel->setFont(juce::Font(juce::FontOptions().withHeight(15.0f)));
    ipAddressLabel->setJustificationType(juce::Justification::centredLeft);
    ipAddressLabel->setEditable(false, false, false);
    ipAddressLabel->setColour(juce::Label::textColourId, juce::Colour(0x80000000));
    ipAddressLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    ipAddressLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x0));

    juce::String tempstr;
    juce::Colour textCol = ColourScheme::getInstance().colours["Text Colour"].withAlpha(0.5f);

    tempstr << "Version: " << juce::JUCEApplication::getInstance()->getApplicationVersion();
    versionLabel->setText(tempstr, juce::dontSendNotification);

    tempstr = "";
    tempstr << juce::SystemStats::getJUCEVersion();
    juceVersionLabel->setText(tempstr, juce::dontSendNotification);

    tempstr = "";
    tempstr << "Current IP Address: " << ipAddress;
    ipAddressLabel->setText(tempstr, juce::dontSendNotification);

    titleLabel->setColour(juce::Label::textColourId, textCol);
    descriptionLabel->setColour(juce::Label::textColourId, textCol);
    creditsLabel->setColour(juce::Label::textColourId, textCol);
    authorLabel->setColour(juce::Label::textColourId, textCol);
    versionLabel->setColour(juce::Label::textColourId, textCol);
    juceVersionLabel->setColour(juce::Label::textColourId, textCol);
    ipAddressLabel->setColour(juce::Label::textColourId, textCol);

    setSize(400, 280);
}

AboutPage::~AboutPage() {}

void AboutPage::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xffeeece1));

    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

void AboutPage::resized() {
    titleLabel->setBounds(8, 8, 208, 32);
    descriptionLabel->setBounds(16, 48, getWidth() - 16, 56);
    creditsLabel->setBounds(16, 104, getWidth() - 16, 56);
    authorLabel->setBounds(16, 152, getWidth() - 16, 40);
    niallmoodyLink->setBounds(proportionOfWidth(0.5f) - (150 / 2), 224, 150, 24);
    juceLink->setBounds(proportionOfWidth(0.5f) - (252 / 2), 248, 252, 24);
    versionLabel->setBounds(getWidth() - 154, 0, 150, 24);
    juceVersionLabel->setBounds(getWidth() - 154, 16, 150, 24);
    ipAddressLabel->setBounds(16, 192, getWidth() - 16, 24);
}
