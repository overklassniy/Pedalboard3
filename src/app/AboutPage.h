// AboutPage.h - About dialog component.
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

#pragma once

#include <JuceHeader.h>

/// About dialog component showing application version, credits, and links.
///
/// Displays the application title, description, credits, author, license,
/// hyperlinks to niallmoody.com and the JUCE website, version information,
/// the current IP address, and a live update-status label that queries the
/// GitHub releases API for overklassniy/Pedalboard3 in a background thread.
class AboutPage : public juce::Component {
  public:
    /// Constructs the about page, populating all labels and links, and
    /// starts a background thread that checks GitHub for a newer release.
    ///
    /// @param ip The current local IP address shown in the dialog.
    explicit AboutPage(const juce::String& ip);

    /// Destructor.
    ~AboutPage() override;

    /// Paints the background colour.
    void paint(juce::Graphics& g) override;

    /// Lays out all labels, links, and the version display.
    void resized() override;

    /// Updates the update-status display.
    ///
    /// Called on the message thread via MessageManager::callAsync after the
    /// background GitHub check completes. When showLink is true the status
    /// text is shown as a clickable hyperlink to the GitHub releases page
    /// (used when an update is available); otherwise it is shown as a
    /// plain label.
    ///
    /// @param text The status message to display.
    /// @param colour The colour to use for the status text.
    /// @param showLink True to show the text as a hyperlink; false for a label.
    void setUpdateStatus(const juce::String& text, const juce::Colour& colour, bool showLink);

  private:
    /// The current computer's IP address.
    juce::String ipAddress;

    std::unique_ptr<juce::Label> titleLabel;
    std::unique_ptr<juce::Label> descriptionLabel;
    std::unique_ptr<juce::Label> creditsLabel;
    std::unique_ptr<juce::Label> authorLabel;
    std::unique_ptr<juce::HyperlinkButton> niallmoodyLink;
    std::unique_ptr<juce::HyperlinkButton> juceLink;
    std::unique_ptr<juce::Label> versionLabel;
    std::unique_ptr<juce::Label> juceVersionLabel;
    std::unique_ptr<juce::Label> ipAddressLabel;
    std::unique_ptr<juce::Label> updateStatusLabel;
    std::unique_ptr<juce::HyperlinkButton> githubLink;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutPage)
};
