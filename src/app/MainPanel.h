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

#include "AudioSingletons.h"
#include "ColourScheme.h"
#include "FilterGraph.h"
#include "Images.h"
#include "MidiAppFifo.h"
#include "PluginField.h"
#include "PropertiesSingleton.h"

#include <JuceHeader.h>

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
                  public juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback> {
  public:
    /// Constructs the main panel, setting up audio, MIDI, OSC, plugins, and the default patch.
    ///
    /// @param appManager The application command manager used for dispatching menu and shortcut commands.
    MainPanel(juce::ApplicationCommandManager* appManager);

    /// Destructor. Tears down audio callbacks, OSC, and owned patches.
    ~MainPanel() override;

    /// Returns the names of the top-level menus.
    juce::StringArray getMenuBarNames() override;

    /// Builds the popup menu for the given top-level menu index.
    ///
    /// @param topLevelMenuIndex The zero-based index of the top-level menu to build.
    /// @param menuName The name of the top-level menu to build.
    /// @return The populated popup menu for the requested index.
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;

    /// Handles menu item selection; commands are dispatched via the command manager.
    ///
    /// @param menuItemID The ID of the selected menu item.
    /// @param topLevelMenuIndex The index of the top-level menu the item belongs to.
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    /// Returns the next command target in the chain for keyboard shortcuts.
    juce::ApplicationCommandTarget* getNextCommandTarget() override;

    /// Populates the array with all command IDs this target handles.
    ///
    /// @param commands The array to fill with all command IDs supported by this target.
    void getAllCommands(juce::Array<juce::CommandID>& commands) override;

    /// Sets the name, category, and default keypress for each command.
    ///
    /// @param commandID The command ID to describe.
    /// @param result The ApplicationCommandInfo to populate with the command's name, category, and default keypress.
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;

    /// Executes the command identified by info.commandID.
    ///
    /// @param info The invocation info containing the command ID to execute.
    /// @return True if the command was handled; false otherwise.
    bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info) override;

    /// Replaces the command manager used for dispatching commands.
    ///
    /// @param manager The new command manager to use for dispatch.
    void setCommandManager(juce::ApplicationCommandManager* manager);

    /// Used to add an ApplicationCommand to the queue so it will get called in the message thread.
    ///
    /// @param commandID The command ID to queue for invocation on the message thread.
    void invokeCommandFromOtherThread(juce::CommandID commandID);
    /// Used to update the tempo from a non-message thread.
    ///
    /// @param tempo The new tempo in beats per minute.
    void updateTempoFromOtherThread(double tempo);

    /// Used to update the CPU usage slider.
    ///
    /// @param timerId The ID of the timer that triggered the callback (CpuTimer, MidiAppTimer, or ProgramChangeTimer).
    void timerCallback(int timerId) override;
    /// Used to save the plugin list.
    ///
    /// @param changedObject The change broadcaster that triggered the callback.
    void changeListenerCallback(juce::ChangeBroadcaster* changedObject) override;
    /// Used to update the tempo.
    ///
    /// @param editor The text editor whose text changed (expected to be the tempo editor).
    void textEditorTextChanged(juce::TextEditor& editor) override;
    /// Used to update the tempo.
    ///
    /// @param editor The text editor in which the return key was pressed (expected to be the tempo editor).
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    /// Used to accept dragged .pdl files.
    ///
    /// @param files The list of file paths being dragged.
    /// @return True if any of the dragged files has a .pdl or .filtergraph extension.
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    /// Used to accept dragged .pdl files.
    ///
    /// @param files The list of file paths that were dropped.
    /// @param x The x coordinate of the drop position (unused).
    /// @param y The y coordinate of the drop position (unused).
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    /// Used to update the OSC port.
    ///
    /// @param port The new OSC port number as a string; must parse to a value between 1 and 65535.
    void setSocketPort(const juce::String& port);
    /// Used to update the OSC multicast address input.
    ///
    /// @param address The new OSC multicast address to store for UI use.
    void setSocketMulticast(const juce::String& address);

    /// Enables/disables the audio input.
    ///
    /// @param val True to enable audio input; false to disable it.
    void enableAudioInput(bool val);
    /// Enables/disables the MIDI input.
    ///
    /// @param val True to enable MIDI input; false to disable it.
    void enableMidiInput(bool val);
    /// Enables/disables the OSC input.
    ///
    /// @param val True to enable OSC input; false to disable it.
    void enableOscInput(bool val);
    /// Sets whether to automatically open the mappings window or not.
    ///
    /// @param val True to automatically open the mappings window; false to suppress it.
    void setAutoMappingsWindow(bool val);

    /// Returns the title of the current set of patches.
    juce::String getDocumentTitle() override;
    /// Loads a set of patches from the passed-in file.
    ///
    /// @param file The .pdl file to load patches from.
    /// @return Result::ok() on success; Result::fail() if no plugin field is available.
    juce::Result loadDocument(const juce::File& file) override;
    /// Tries to save the current patches to the passed-in file.
    ///
    /// @param file The destination .pdl file to write.
    /// @return Result::ok() on success; Result::fail() if no plugin field is available.
    juce::Result saveDocument(const juce::File& file) override;
    /// Returns the last set of patches opened.
    juce::File getLastDocumentOpened() override;
    /// Saves the last set of patches opened.
    ///
    /// @param file The file to record as the most recently opened document.
    void setLastDocumentOpened(const juce::File& file) override;

    /// Used to update patches from PatchOrganiser.
    juce::ComboBox* getPatchComboBox() { return patchComboBox.get(); }

    /// Used to clear the listWindow variable.
    void setListWindow(PluginListWindow* win) { listWindow = win; }

    /// Adds a patch from its XmlElement.
    ///
    /// @param patch The XmlElement describing the patch to add; ownership is transferred.
    void addPatch(juce::XmlElement* patch);
    /// Saves the current patch to our patches array.
    void savePatch();
    /// Duplicates the indexed patch.
    ///
    /// @param index The zero-based index of the patch to duplicate.
    void duplicatePatch(int index);
    /// Called from PatchOrganiser when it is moving patches up/down.
    void nextSwitchDoNotSavePrev();
    /// Triggers a patch change from a Midi Program Change message.
    ///
    /// @param newPatch The zero-based index of the patch to switch to.
    void switchPatchFromProgramChange(int newPatch);
    /// Returns the index of the current patch.
    int getCurrentPatch() const { return patchComboBox->getSelectedId() - 1; }

    /// Returns the PluginField's MidiMappingManager.
    MidiMappingManager* getMidiMappingManager();
    /// Returns the PluginField's OscMappingManager.
    OscMappingManager* getOscMappingManager();

    /// Command IDs for every menu and shortcut action.
    ///
    /// New constants must be appended to the end of this list. Inserting or
    /// reordering values would break previously saved user key mappings,
    /// which store IDs by ordinal position.
    enum {
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

    /// Paints the background and refreshes button colours.
    void paint(juce::Graphics& g) override;

    /// Lays out all child components.
    void resized() override;

    /// Dispatches button clicks to their corresponding commands.
    ///
    /// @param buttonThatWasClicked The button that was clicked (prevPatch, nextPatch, playButton, rtzButton, or
    /// tapTempoButton).
    void buttonClicked(juce::Button* buttonThatWasClicked) override;

    /// Handles patch combo box selection and text edits.
    ///
    /// @param comboBoxThatHasChanged The combo box that changed (expected to be the patch combo box).
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    /// Handles CPU slider value changes; the slider is read-only and updated by timer.
    ///
    /// @param sliderThatWasMoved The slider whose value changed (expected to be the CPU usage slider).
    void sliderValueChanged(juce::Slider* sliderThatWasMoved) override;

  private:
    /// Switches to the patch at the given index.
    ///
    /// @param newPatch The zero-based index of the patch to load.
    /// @param savePrev When true the current patch state is written back to the patches array before switching.
    /// @param reloadPatch When true the current patch is reloaded from its stored XML even if newPatch equals
    /// currentPatch.
    void switchPatch(int newPatch, bool savePrev = true, bool reloadPatch = false);

    /// Called when an OSC message is received.
    ///
    /// @param message The OSC message that was received.
    void oscMessageReceived(const juce::OSCMessage& message) override;
    /// Called when an OSC bundle is received.
    ///
    /// @param bundle The OSC bundle that was received.
    void oscBundleReceived(const juce::OSCBundle& bundle) override;

    /// The IDs of the three timers.
    enum { CpuTimer = 0, MidiAppTimer, ProgramChangeTimer };

    /// The last opened document.
    static juce::File lastDocument;

    /// Our copy of the commandManager.
    juce::ApplicationCommandManager* commandManager;
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
    PluginListWindow* listWindow;

    /// The currently-loaded patches.
    juce::Array<juce::XmlElement*> patches;
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
    class ProgramChangeWarning : public juce::Component {
      public:
        /// Constructs the warning component with a default index of zero.
        ProgramChangeWarning() : index(0) { setSize(250, 150); }

        /// Destructor.
        ~ProgramChangeWarning() override = default;

        /// Draws the warning.
        void paint(juce::Graphics& g) override {
            juce::String tempstr;
            juce::Font smallFont(juce::FontOptions().withHeight(24.0f));
            juce::Font bigFont(juce::FontOptions(48.0f, juce::Font::bold));

            g.setColour(ColourScheme::getInstance().colours["Text Colour"]);

            g.setFont(smallFont);
            g.drawText("Out of bounds", 0, 0, 250, 50, juce::Justification(juce::Justification::centred), false);
            g.drawText("MIDI Program Change", 0, 24, 250, 50, juce::Justification(juce::Justification::centred), false);
            g.drawText("received:", 0, 48, 250, 50, juce::Justification(juce::Justification::centred), false);

            tempstr << index;
            g.setFont(bigFont);
            g.drawText(tempstr, 0, 88, 250, 50, juce::Justification(juce::Justification::centred), false);
        }

        /// Sets the offending index.
        ///
        /// @param i The out-of-bounds MIDI program change index to display.
        void setIndex(int i) { index = i; }

      private:
        /// The offending index.
        int index;
    };

    /// Component drawn inside the warning call-out box.
    ProgramChangeWarning warningText;

    /// Call-out box shown when a MIDI program change is outside the patch list range.
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
class PluginListWindow : public juce::DocumentWindow {
  public:
    /// Creates the plugin list window, restoring its last saved position.
    ///
    /// @param knownPluginList The known plugin list to display and edit in the window.
    /// @param p The parent MainPanel; used to clear the panel's list window pointer on destruction.
    PluginListWindow(juce::KnownPluginList& knownPluginList, MainPanel* p)
        : juce::DocumentWindow("Available Plugins", juce::Colour(0xffeeece1),
                               juce::DocumentWindow::minimiseButton | juce::DocumentWindow::closeButton),
          panel(p) {
        const juce::File deadMansPedalFile(
            PropertiesSingleton::getInstance().getUserSettings()->getFile().getSiblingFile(
                "RecentlyCrashedPluginsList"));

        setContentOwned(new juce::PluginListComponent(AudioPluginFormatManagerSingleton::getInstance(), knownPluginList,
                                                      deadMansPedalFile,
                                                      PropertiesSingleton::getInstance().getUserSettings()),
                        true);

        setResizable(true, false);
        centreWithSize(300, 400);
        setUsingNativeTitleBar(true);

        if (auto* peer = getPeer())
            peer->setIcon(juce::ImageCache::getFromMemory(Images::icon512_png, Images::icon512_pngSize));

        restoreWindowStateFromString(PropertiesSingleton::getInstance().getUserSettings()->getValue("listWindowPos"));
        setVisible(true);
    }

    /// Saves the window position and clears the panel's list window pointer.
    ~PluginListWindow() override {
        if (panel)
            panel->setListWindow(nullptr);

        PropertiesSingleton::getInstance().getUserSettings()->setValue("listWindowPos", getWindowStateAsString());
    }

    /// Closes the window.
    void closeButtonPressed() override { delete this; }

  private:
    /// The 'parent' main panel.
    MainPanel* panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
};

#endif
