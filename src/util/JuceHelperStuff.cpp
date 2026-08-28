// JuceHelperStuff.cpp - Some useful helper functions.
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

#include "JuceHelperStuff.h"

#include "Images.h"

Drawable* JuceHelperStuff::loadSVGFromMemory(const void* dataToInitialiseFrom, size_t sizeInBytes) {
    MemoryBlock memBlock(dataToInitialiseFrom, sizeInBytes);
    XmlDocument doc(memBlock.toString());
    std::unique_ptr<XmlElement> svgData(doc.getDocumentElement());

    if (svgData == nullptr)
        return nullptr;

    return Drawable::createFromSVG(*svgData).release();
}

/// Internal DialogWindow subclass used by showModalDialog and
/// showNonModalDialog.
///
/// When deleteContent is true the content component is owned by the window
/// and deleted alongside it, and the window self-deletes on close.
class TempDialogWindow : public DialogWindow {
  public:
    TempDialogWindow(const String& title, Component* contentComponent_, Component* componentToCentreAround,
                     const Colour& colour, const bool escapeKeyTriggersCloseButton_, const bool shouldBeResizable,
                     const bool useBottomRightCornerResizer, const bool deleteContent = false)
        : DialogWindow(title, colour, escapeKeyTriggersCloseButton_, false), deleteDialog(deleteContent) {
        if (!juce::JUCEApplication::isStandaloneApp())
            setAlwaysOnTop(true);

        if (deleteContent)
            setContentOwned(contentComponent_, true);
        else
            setContentNonOwned(contentComponent_, true);
        centreAroundComponent(componentToCentreAround, getWidth(), getHeight());
        setResizable(shouldBeResizable, useBottomRightCornerResizer);
    }

    void closeButtonPressed() override {
        setVisible(false);

        if (deleteDialog) {
            auto* self = this;
            juce::MessageManager::callAsync([self] { delete self; });
        }
    }

  private:
    JUCE_DECLARE_NON_COPYABLE(TempDialogWindow)

    /// Whether the window should self-delete when the close button is pressed.
    bool deleteDialog;
};

int JuceHelperStuff::showModalDialog(const String& dialogTitle, Component* contentComponent,
                                     Component* componentToCentreAround, const Colour& backgroundColour,
                                     bool escapeKeyTriggersCloseButton, bool shouldBeResizable,
                                     bool useBottomRightCornerResizer) {
    TempDialogWindow dw(dialogTitle, contentComponent, componentToCentreAround, backgroundColour,
                        escapeKeyTriggersCloseButton, shouldBeResizable, useBottomRightCornerResizer);
    // Set the native title bar before adding to desktop. Calling
    // setUsingNativeTitleBar after addToDesktop triggers a peer
    // recreation that crashes in JUCE 8.
    dw.setUsingNativeTitleBar(true);
    dw.addToDesktop();
    if (auto* peer = dw.getPeer())
        peer->setIcon(ImageCache::getFromMemory(Images::icon512_png, Images::icon512_pngSize));

    return dw.runModalLoop();
}

void JuceHelperStuff::showNonModalDialog(const String& dialogTitle, Component* contentComponent,
                                         Component* componentToCentreAround, const Colour& backgroundColour,
                                         bool escapeKeyTriggersCloseButton, bool shouldBeResizable,
                                         bool useBottomRightCornerResizer, bool stayOnTop) {
    auto* dw = new TempDialogWindow(dialogTitle, contentComponent, componentToCentreAround, backgroundColour,
                                    escapeKeyTriggersCloseButton, shouldBeResizable, useBottomRightCornerResizer, true);
    dw->setUsingNativeTitleBar(true);

    dw->addToDesktop();
    dw->setVisible(true);
    dw->setAlwaysOnTop(stayOnTop);
    if (auto* peer = dw->getPeer())
        peer->setIcon(ImageCache::getFromMemory(Images::icon512_png, Images::icon512_pngSize));
}

File JuceHelperStuff::getAppDataFolder() {
#ifdef __APPLE__
    File retval = File::getSpecialLocation(File::userApplicationDataDirectory)
                      .getChildFile("Application Support")
                      .getChildFile("Pedalboard3");

    if (!retval.exists())
        retval.createDirectory();

    return retval;
#else
    return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Pedalboard3");
#endif
}
