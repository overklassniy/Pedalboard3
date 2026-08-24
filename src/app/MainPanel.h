// MainPanel.h - Main top-level UI component.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2009 Niall Moody.
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

#ifndef MAINPANEL_H_
#define MAINPANEL_H_

#include <JuceHeader.h>

#include "FilterGraph.h"
#include "ColourScheme.h"
#include "PluginField.h"
#include "MidiAppFifo.h"
#include "Images.h"
#include "AudioSingletons.h"
#include "PropertiesSingleton.h"

class PluginListWindow;
class MidiMappingManager;
class OscMappingManager;

/// Main top-level UI component for the application.
///
/// Manages the audio device, plugin graph, patch list, menus, transport,
/// and OSC input.
class MainPanel : public juce::Component,
                  public juce::MenuBarModel,
                  public juce::ApplicationCommandTarget,
                  public juce::MultiTimer,
                  public juce::ChangeListener,
                  public juce::FileBasedDocument,
                  public juce::FileDragAndDropTarget,
                  public juce::TextEditor::Listener,
                  public juce::Button::Listener,
                  public juce::ComboBox::Listener,
                  public juce::Slider::Listener,
                  public juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
  public:
    MainPanel(juce::ApplicationCommandManager *appManager);
    ~MainPanel() override;

    /// For the menu bar.
    juce::StringArray getMenuBarNames() override;
    /// For the menu bar.
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex,
                                    const juce::String &menuName) override;
    /// For the menu bar.
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    /// For keyboard shortcuts etc.
    juce::ApplicationCommandTarget *getNextCommandTarget() override;
    /// For keyboard shortcuts etc.
    void getAllCommands(juce::Array<juce::CommandID>& commands) override;
    /// For keyboard shortcuts etc.
    void getCommandInfo(juce::CommandID commandID,
                        juce::ApplicationCommandInfo& result) override;
    /// For keyboard shortcuts etc.
    bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info) override;

    /// So we know what the commandManager is.
    void setCommandManager(juce::ApplicationCommandManager *manager);

    /// Used to add an ApplicationCommand to the queue so it will get called in the message thread.
    void invokeCommandFromOtherThread(juce::CommandID commandID);
    /// Used to update the tempo from a non-message thread.
    void updateTempoFromOtherThread(double tempo);

    /// Used to update the CPU usage slider.
    void timerCallback(int timerId) override;
    /// Used to save the plugin list.
    void changeListenerCallback(juce::ChangeBroadcaster *changedObject) override;
    /// Used to update the tempo.
    void textEditorTextChanged(juce::TextEditor &editor) override;
    /// Used to update the tempo.
    void textEditorReturnKeyPressed(juce::TextEditor &editor) override;

    /// Used to accept dragged .pdl files.
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    /// Used to accept dragged .pdl files.
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    /// Used to update the OSC port.
    void setSocketPort(const juce::String& port);
    /// Used to update the OSC multicast address input.
    void setSocketMulticast(const juce::String& address);

    /// Enables/disables the audio input.
    void enableAudioInput(bool val);
    /// Enables/disables the MIDI input.
    void enableMidiInput(bool val);
    /// Enables/disables the OSC input.
    void enableOscInput(bool val);
    /// Sets whether to automatically open the mappings window or not.
    void setAutoMappingsWindow(bool val);

    /// Returns the title of the current set of patches.
    juce::String getDocumentTitle() override;
    /// Loads a set of patches from the passed-in file.
    juce::Result loadDocument(const juce::File& file) override;
    /// Tries to save the current patches to the passed-in file.
    juce::Result saveDocument(const juce::File& file) override;
    /// Returns the last set of patches opened.
    juce::File getLastDocumentOpened() override;
    /// Saves the last set of patches opened.
    void setLastDocumentOpened(const juce::File& file) override;

    /// Used to update patches from PatchOrganiser.
    juce::ComboBox *getPatchComboBox() { return patchComboBox.get(); }

    /// Used to clear the listWindow variable.
    void setListWindow(PluginListWindow *win) { listWindow = win; }

    /// Adds a patch from its XmlElement.
    void addPatch(juce::XmlElement *patch);
    /// Saves the current patch to our patches array.
    void savePatch();
    /// Duplicates the indexed patch.
    void duplicatePatch(int index);
    /// Called from PatchOrganiser when it is moving patches up/down.
    void nextSwitchDoNotSavePrev();
    /// Triggers a patch change from a Midi Program Change message.
    void switchPatchFromProgramChange(int newPatch);
    /// Returns the index of the current patch.
    int getCurrentPatch() const { return patchComboBox->getSelectedId() - 1; }

    /// Returns the PluginField's MidiMappingManager.
    MidiMappingManager *getMidiMappingManager();
    /// Returns the PluginField's OscMappingManager.
    OscMappingManager *getOscMappingManager();

    /// Constants for the various menu options.
    ///
    /// NOTE: Add new constants TO THE END OF THE LIST, or the user's saved
    /// mappings will get screwed up!
    enum
    {
        FileNew = 1,
        FileOpen,
        FileSave,
        FileSaveAs,
        FileSaveAsDefault,
        FileResetDefault,
        FileExit,
        EditDeleteConnection,
        EditOrganisePatches,
        OptionsAudio,
        OptionsPluginList,
        OptionsPreferences,
        OptionsColourSchemes,
        OptionsKeyMappings,
        HelpAbout,
        PatchNextPatch,
        PatchPrevPatch,
        TransportPlay,
        TransportRtz,
        TransportTapTempo,
        EditUserPresetManagement,
        HelpDocumentation,
        HelpLog
    };

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void sliderValueChanged(juce::Slider* sliderThatWasMoved) override;

  private:
    /// Helper method. Switches patches.
    ///
    /// \param newPatch Index of the new patch to load.
    /// \param savePrev Saves the current patch in the process.
    /// \param reloadPatch Reloads the current patch.
    void switchPatch(int newPatch, bool savePrev = true, bool reloadPatch = false);

    /// Called when an OSC message is received.
    void oscMessageReceived(const juce::OSCMessage& message) override;
    /// Called when an OSC bundle is received.
    void oscBundleReceived(const juce::OSCBundle& bundle) override;

    /// The IDs of the three timers.
    enum
    {
        CpuTimer = 0,
        MidiAppTimer,
        ProgramChangeTimer
    };

    /// The last opened document.
    static juce::File lastDocument;

    /// Our copy of the commandManager.
    juce::ApplicationCommandManager *commandManager;
    /// Used to display tooltips.
    juce::TooltipWindow tooltips;

    /// The sound card settings.
    juce::AudioDeviceManager deviceManager;
    /// The graph representing the audio signal path.
    FilterGraph signalPath;
    /// Object used to 'play' the signalPath object.
    juce::AudioProcessorPlayer graphPlayer;
    /// The list of plugins the user can load.
    juce::KnownPluginList pluginList;
    /// The OSC receiver we listen for OSC messages on.
    juce::OSCReceiver oscReceiver;
    /// The current OSC port.
    int oscPortNumber;
    /// The current OSC multicast address (stored for UI only).
    juce::String oscMulticastAddress;

    /// Window to display/edit the list of possible plugins.
    PluginListWindow *listWindow;

    /// The currently-loaded patches.
    juce::Array<juce::XmlElement *> patches;
    /// The index of the current patch.
    int currentPatch;
    /// Used to switch patches via Midi Program Changes.
    int programChangePatch;

    /// The two drawables we use for the playButton.
    std::unique_ptr<juce::Drawable> playImage;
    std::unique_ptr<juce::Drawable> pauseImage;
    /// Drawable for the rtzButton.
    std::unique_ptr<juce::Drawable> rtzImage;
    /// Whether the playPauseButton is currently displaying the play icon.
    bool playing;

    /// Used to compensate for the way the combo box handles the user editing its text.
    int lastCombo;

    /// Used to ensure patches do not get overwritten when they are being re-ordered in PatchOrganiser.
    bool doNotSaveNextPatch;

    /// Used for tap tempo when the user does it via keyboard.
    juce::int64 lastTempoTicks;

    /// Used to pass messages from the audio thread to the message thread.
    MidiAppFifo midiAppFifo;

    /// Simple Component to pass to warningBox.
    class ProgramChangeWarning : public juce::Component
    {
      public:
        ProgramChangeWarning():
            index(0)
        {
            setSize(250, 150);
        }

        ~ProgramChangeWarning() override = default;

        /// Draws the warning.
        void paint(juce::Graphics& g) override
        {
            juce::String tempstr;
            juce::Font smallFont(juce::FontOptions().withHeight(24.0f));
            juce::Font bigFont(juce::FontOptions(48.0f, juce::Font::bold));

            g.setColour(ColourScheme::getInstance().colours["Text Colour"]);

            g.setFont(smallFont);
            g.drawText("Out of bounds",
                       0,
                       0,
                       250,
                       50,
                       juce::Justification(juce::Justification::centred),
                       false);
            g.drawText("MIDI Program Change",
                       0,
                       24,
                       250,
                       50,
                       juce::Justification(juce::Justification::centred),
                       false);
            g.drawText("received:",
                       0,
                       48,
                       250,
                       50,
                       juce::Justification(juce::Justification::centred),
                       false);

            tempstr << index;
            g.setFont(bigFont);
            g.drawText(tempstr,
                       0,
                       88,
                       250,
                       50,
                       juce::Justification(juce::Justification::centred),
                       false);
        }

        /// Sets the offending index.
        void setIndex(int i)
        {
            index = i;
        }

      private:
        /// The offending index.
        int index;
    };

    /// Used to inform the user when the user does a MIDI program change outside the limits of the patch list.
    ProgramChangeWarning warningText;
    /// Used to inform the user when the user does a MIDI program change outside the limits of the patch list.
    std::unique_ptr<juce::CallOutBox> warningBox;

    std::unique_ptr<juce::Label> patchLabel;
    std::unique_ptr<juce::TextButton> prevPatch;
    std::unique_ptr<juce::TextButton> nextPatch;
    std::unique_ptr<juce::ComboBox> patchComboBox;
    std::unique_ptr<juce::Viewport> viewport;
    std::unique_ptr<juce::Slider> cpuSlider;
    std::unique_ptr<juce::Label> cpuLabel;
    std::unique_ptr<juce::DrawableButton> playButton;
    std::unique_ptr<juce::DrawableButton> rtzButton;
    std::unique_ptr<juce::Label> tempoLabel;
    std::unique_ptr<juce::TextEditor> tempoEditor;
    std::unique_ptr<juce::ArrowButton> tapTempoButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};

/// Window to display/edit the list of possible plugins.
class PluginListWindow : public juce::DocumentWindow
{
  public:
    PluginListWindow(juce::KnownPluginList& knownPluginList, MainPanel *p):
        juce::DocumentWindow("Available Plugins",
                             juce::Colour(0xffeeece1),
                             juce::DocumentWindow::minimiseButton | juce::DocumentWindow::closeButton),
        panel(p)
    {
        const juce::File deadMansPedalFile(PropertiesSingleton::getInstance().getUserSettings()->getFile().getSiblingFile("RecentlyCrashedPluginsList"));

        setContentOwned(new juce::PluginListComponent(AudioPluginFormatManagerSingleton::getInstance(),
                                                      knownPluginList,
                                                      deadMansPedalFile,
                                                      PropertiesSingleton::getInstance().getUserSettings()),
                        true);

        setResizable(true, false);
        centreWithSize(300, 400);
        setUsingNativeTitleBar(true);

        if (auto *peer = getPeer())
            peer->setIcon(juce::ImageCache::getFromMemory(Images::icon512_png,
                                                          Images::icon512_pngSize));

        restoreWindowStateFromString(PropertiesSingleton::getInstance().getUserSettings()->getValue("listWindowPos"));
        setVisible(true);
    }

    ~PluginListWindow() override
    {
        if (panel)
            panel->setListWindow(nullptr);

        PropertiesSingleton::getInstance().getUserSettings()->setValue("listWindowPos", getWindowStateAsString());
    }

    /// Closes the window.
    void closeButtonPressed() override
    {
        delete this;
    }

  private:
    /// The 'parent' main panel.
    MainPanel *panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
};

#endif
