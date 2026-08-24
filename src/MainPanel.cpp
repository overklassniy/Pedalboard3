// MainPanel.cpp - Main top-level UI component.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2009 Niall Moody.
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

#include "MainPanel.h"

#include "Images.h"
#include "Vectors.h"
#include "LogFile.h"
#include "AboutPage.h"
#include "LogDisplay.h"
#include "TapTempoBox.h"
#include "PluginField.h"
#include "MainTransport.h"
#include "PatchOrganiser.h"
#include "AudioSingletons.h"
#include "UserPresetWindow.h"
#include "PreferencesDialog.h"
#include "ColourSchemeEditor.h"
#include "PropertiesSingleton.h"
#include "PedalboardProcessors.h"
#include "ApplicationMappingsEditor.h"
#include "JuceHelperStuff.h"

#include <sstream>
#include <iomanip>

using namespace juce;

//==============================================================================
File MainPanel::lastDocument = File();

//==============================================================================
MainPanel::MainPanel (ApplicationCommandManager *appManager)
    : FileBasedDocument(".pdl", "*.pdl", "Choose a set of patches to open...", "Choose a set of patches to save as..."),
      commandManager(appManager),
      currentPatch(0),
      programChangePatch(0),
      lastCombo(1),
      doNotSaveNextPatch(false),
      lastTempoTicks(0),
      playing(false),
      listWindow(nullptr),
      oscPortNumber(5678)
{
    patchLabel = std::make_unique<Label> ("patchLabel",
                                          "Patch:");
    patchLabel->setFont (Font (FontOptions().withHeight(15.0f)));
    patchLabel->setJustificationType (Justification::centredLeft);
    patchLabel->setEditable (false, false, false);
    patchLabel->setColour (TextEditor::textColourId, Colours::black);
    patchLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));
    addAndMakeVisible (patchLabel.get());

    prevPatch = std::make_unique<TextButton> ("prevPatch");
    prevPatch->setButtonText ("-");
    prevPatch->setConnectedEdges (Button::ConnectedOnRight);
    prevPatch->addListener (this);
    addAndMakeVisible (prevPatch.get());

    nextPatch = std::make_unique<TextButton> ("nextPatch");
    nextPatch->setButtonText ("+");
    nextPatch->setConnectedEdges (Button::ConnectedOnLeft);
    nextPatch->addListener (this);
    addAndMakeVisible (nextPatch.get());

    patchComboBox = std::make_unique<ComboBox> ("patchComboBox");
    patchComboBox->setEditableText (true);
    patchComboBox->setJustificationType (Justification::centredLeft);
    patchComboBox->setTextWhenNothingSelected (String());
    patchComboBox->setTextWhenNoChoicesAvailable ("(no choices)");
    patchComboBox->addItem ("1 - <untitled>", 1);
    patchComboBox->addItem ("<new patch>", 2);
    patchComboBox->addListener (this);
    addAndMakeVisible (patchComboBox.get());

    viewport = std::make_unique<Viewport> ("new viewport");
    addAndMakeVisible (viewport.get());

    cpuSlider = std::make_unique<Slider> ("cpuSlider");
    cpuSlider->setRange (0, 1, 0);
    cpuSlider->setSliderStyle (Slider::LinearBar);
    cpuSlider->setTextBoxStyle (Slider::NoTextBox, true, 80, 20);
    cpuSlider->addListener (this);
    addAndMakeVisible (cpuSlider.get());

    cpuLabel = std::make_unique<Label> ("cpuLabel",
                                        "CPU Usage:");
    cpuLabel->setFont (Font (FontOptions().withHeight(15.0f)));
    cpuLabel->setJustificationType (Justification::centredLeft);
    cpuLabel->setEditable (false, false, false);
    cpuLabel->setColour (TextEditor::textColourId, Colours::black);
    cpuLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));
    addAndMakeVisible (cpuLabel.get());

    playButton = std::make_unique<DrawableButton> ("playButton", DrawableButton::ImageOnButtonBackground);
    playButton->setName ("playButton");
    addAndMakeVisible (playButton.get());

    rtzButton = std::make_unique<DrawableButton> ("rtzButton", DrawableButton::ImageOnButtonBackground);
    rtzButton->setName ("rtzButton");
    addAndMakeVisible (rtzButton.get());

    tempoLabel = std::make_unique<Label> ("tempoLabel",
                                          "Tempo:");
    tempoLabel->setFont (Font (FontOptions().withHeight(15.0f)));
    tempoLabel->setJustificationType (Justification::centredLeft);
    tempoLabel->setEditable (false, false, false);
    tempoLabel->setColour (TextEditor::textColourId, Colours::black);
    tempoLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));
    addAndMakeVisible (tempoLabel.get());

    tempoEditor = std::make_unique<TextEditor> ("tempoEditor");
    tempoEditor->setMultiLine (false);
    tempoEditor->setReturnKeyStartsNewLine (false);
    tempoEditor->setReadOnly (false);
    tempoEditor->setScrollbarsShown (true);
    tempoEditor->setCaretVisible (true);
    tempoEditor->setPopupMenuEnabled (true);
    tempoEditor->setText ("120.00");
    addAndMakeVisible (tempoEditor.get());

    tapTempoButton = std::make_unique<ArrowButton> ("tapTempoButton", 0.0, Colour(0x40000000));
    tapTempoButton->setName ("tapTempoButton");
    addAndMakeVisible (tapTempoButton.get());

    Colour buttonCol = ColourScheme::getInstance().colours["Button Colour"];

    patches.add (nullptr);

    prevPatch->setTooltip("Previous patch");
    nextPatch->setTooltip("Next patch");
    playButton->setTooltip("Play (main transport)");
    rtzButton->setTooltip("Return to zero (main transport)");
    tapTempoButton->setTooltip("Tap tempo");

    // So the user cannot drag the cpu meter.
    cpuSlider->setInterceptsMouseClicks(false, true);
    cpuSlider->setColour(Slider::thumbColourId, ColourScheme::getInstance().colours["CPU Meter Colour"]);

    // Setup the DrawableButton images.
    playImage.reset (JuceHelperStuff::loadSVGFromMemory(Vectors::playbutton_svg,
                                                        Vectors::playbutton_svgSize));
    pauseImage.reset (JuceHelperStuff::loadSVGFromMemory(Vectors::pausebutton_svg,
                                                         Vectors::pausebutton_svgSize));
    playButton->setImages(playImage.get());
    playButton->setColour(DrawableButton::backgroundColourId,
                          buttonCol);
    playButton->setColour(DrawableButton::backgroundOnColourId,
                          buttonCol);
    playButton->addListener(this);

    rtzImage.reset (JuceHelperStuff::loadSVGFromMemory(Vectors::rtzbutton_svg,
                                                       Vectors::rtzbutton_svgSize));
    rtzButton->setImages(rtzImage.get());
    rtzButton->setColour(DrawableButton::backgroundColourId,
                         buttonCol);
    rtzButton->setColour(DrawableButton::backgroundOnColourId,
                         buttonCol);
    rtzButton->addListener(this);

    MainTransport::getInstance()->registerTransport(this);

    tempoEditor->addListener(this);
    tempoEditor->setInputRestrictions(0, "0123456789.");

    tapTempoButton->addListener(this);

    // Setup the soundcard.
    auto savedAudioState = PropertiesSingleton::getInstance().getUserSettings()->getXmlValue("audioDeviceState");
    String tempstr = deviceManager.initialise(2, 2, savedAudioState.get(), true);
    ignoreUnused(tempstr);

    // Load the plugin list.
    auto savedPluginList = PropertiesSingleton::getInstance().getUserSettings()->getXmlValue("pluginList");
    if (savedPluginList != nullptr)
        pluginList.recreateFromXml(*savedPluginList);

    {
        LevelProcessor lev;
        FilePlayerProcessor fPlay;
        OutputToggleProcessor toggle;
        VuMeterProcessor vuMeter;
        RecorderProcessor recorder;
        MetronomeProcessor metronome;
        LooperProcessor looper;
        PluginDescription desc;

        lev.fillInPluginDescription(desc);
        pluginList.addType(desc);

        fPlay.fillInPluginDescription(desc);
        pluginList.addType(desc);

        toggle.fillInPluginDescription(desc);
        pluginList.addType(desc);

        vuMeter.fillInPluginDescription(desc);
        pluginList.addType(desc);

        recorder.fillInPluginDescription(desc);
        pluginList.addType(desc);

        metronome.fillInPluginDescription(desc);
        pluginList.addType(desc);

        looper.fillInPluginDescription(desc);
        pluginList.addType(desc);
    }
    pluginList.addChangeListener(this);

    pluginList.sort(KnownPluginList::sortAlphabetically, true);

    // Setup the signal path to connect it to the soundcard.
    graphPlayer.setProcessor(&signalPath.getGraph());
    deviceManager.addAudioCallback(&graphPlayer);

    // Setup midi.
    for (const auto& device : MidiInput::getAvailableDevices())
    {
        deviceManager.setMidiInputDeviceEnabled(device.identifier, true);
        deviceManager.addMidiInputDeviceCallback(device.identifier, &graphPlayer);
    }

    // Setup the PluginField.
    PluginField *field = new PluginField(&signalPath,
                                         &pluginList,
                                         commandManager);
    field->addChangeListener(this);
    viewport->setViewedComponent(field);
    viewport->setWantsKeyboardFocus(false);

    KnownPluginListSingleton::setInstance(&pluginList);

    patchComboBox->setSelectedId(1, juce::dontSendNotification);

    // Setup the OSC receiver.
    oscReceiver.addListener(this);

    oscPortNumber = PropertiesSingleton::getInstance().getUserSettings()->getValue("OSCPort", "5678").getIntValue();
    if (oscPortNumber <= 0)
        oscPortNumber = 5678;
    oscMulticastAddress = PropertiesSingleton::getInstance().getUserSettings()->getValue("OSCMulticastAddress");

    if (PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("OscInput", true))
        oscReceiver.connect(oscPortNumber);

    savePatch();

    // Necessary?
    Process::setPriority(Process::HighPriority);

    // Used to ensure we get MidiAppMapping events even when the window's not
    // focused.
    appManager->setFirstCommandTarget(this);

    setSize (1024, 570);

    // Setup the program change warning.
    warningBox.reset(new CallOutBox(warningText, patchComboBox->getBounds(), this));
    warningBox->setVisible(false);

    // Start timers.
    startTimer(CpuTimer, 100);
    startTimer(MidiAppTimer, 30);

    // To load the default patch.
    File defaultPatch = JuceHelperStuff::getAppDataFolder().getChildFile("default.pdl");

    if (defaultPatch.existsAsFile())
        commandManager->invokeDirectly(FileNew, true);
}

MainPanel::~MainPanel()
{
    removeAllChildren();

    MainTransport::getInstance()->unregisterTransport(this);

    deviceManager.removeAudioCallback(&graphPlayer);

    for (const auto& device : MidiInput::getAvailableDevices())
        deviceManager.removeMidiInputDeviceCallback(device.identifier, &graphPlayer);

    graphPlayer.setProcessor(nullptr);
    signalPath.clear(false, false, false);

    oscReceiver.disconnect();
    oscReceiver.removeListener(this);

    if (listWindow)
        delete listWindow;

    for (auto* p : patches)
        delete p;
    patches.clear();

    if (LogFile::getInstance().getIsLogging())
        LogFile::getInstance().stop();
}

//==============================================================================
void MainPanel::paint (Graphics& g)
{
    Colour tempCol = ColourScheme::getInstance().colours["Button Colour"];

    playButton->setColour(DrawableButton::backgroundColourId,
                          tempCol);
    playButton->setColour(DrawableButton::backgroundOnColourId,
                          tempCol);
    rtzButton->setColour(DrawableButton::backgroundColourId,
                         tempCol);
    rtzButton->setColour(DrawableButton::backgroundOnColourId,
                         tempCol);

    g.fillAll (Colour (0xffeeece1));
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

void MainPanel::resized()
{
    patchLabel->setBounds (8, getHeight() - 33, 48, 24);
    prevPatch->setBounds (264, getHeight() - 33, 24, 24);
    nextPatch->setBounds (288, getHeight() - 33, 24, 24);
    patchComboBox->setBounds (56, getHeight() - 33, 200, 24);
    viewport->setBounds (0, 0, getWidth(), getHeight() - 40);
    cpuSlider->setBounds (getWidth() - 156, getHeight() - 33, 150, 24);
    cpuLabel->setBounds (getWidth() - 236, getHeight() - 33, 78, 24);
    playButton->setBounds (proportionOfWidth (0.5000f) - ((36) / 2), getHeight() - 38, 36, 36);
    rtzButton->setBounds ((proportionOfWidth (0.5000f) - ((36) / 2)) + 38, getHeight() - 32, 24, 24);
    tempoLabel->setBounds ((proportionOfWidth (0.5000f) - ((36) / 2)) + -151, getHeight() - 33, 64, 24);
    tempoEditor->setBounds ((proportionOfWidth (0.5000f) - ((36) / 2)) + -87, getHeight() - 33, 52, 24);
    tapTempoButton->setBounds ((proportionOfWidth (0.5000f) - ((36) / 2)) + -31, getHeight() - 27, 10, 16);

    Component *field = viewport->getViewedComponent();

    if (field)
    {
        int x = field->getWidth();
        int y = field->getHeight();

        if (field->getWidth() < getWidth())
            x = getWidth();
        if (field->getHeight() < (getHeight()-40))
            y = getHeight()-40;

        field->setSize(x, y);
    }
}

void MainPanel::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == prevPatch.get())
    {
        commandManager->invokeDirectly(PatchPrevPatch, true);
    }
    else if (buttonThatWasClicked == nextPatch.get())
    {
        commandManager->invokeDirectly(PatchNextPatch, true);
    }
    else if (buttonThatWasClicked == playButton.get())
    {
        commandManager->invokeDirectly(TransportPlay, true);
    }
    else if (buttonThatWasClicked == rtzButton.get())
    {
        commandManager->invokeDirectly(TransportRtz, true);
    }
    else if (buttonThatWasClicked == tapTempoButton.get())
    {
        PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());
        TapTempoBox tempoBox(field, tempoEditor.get());

        CallOutBox callout(tempoBox, tapTempoButton->getBounds(), this);
        callout.runModalLoop();
    }
}

void MainPanel::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == patchComboBox.get())
    {
        // Add a new patch.
        if (patchComboBox->getSelectedItemIndex() == (patchComboBox->getNumItems()-1))
        {
            String tempstr;

            // Save current patch.
            savePatch();

            // Setup the new ComboBox stuff.
            tempstr << patchComboBox->getNumItems() << " - <untitled>";
            patchComboBox->changeItemText(patchComboBox->getNumItems(),
                                          tempstr);
            patchComboBox->addItem("<new patch>", patchComboBox->getNumItems()+1);
            patches.add(nullptr);

            // Make the new patch the current patch, clear it to the default
            // state.
            patchComboBox->setSelectedId(patchComboBox->getNumItems()-1, juce::dontSendNotification);
            switchPatch(patchComboBox->getNumItems()-2);
            savePatch();

            changed();
        }
        // Update the patch text if the user's changed it.
        else if (patchComboBox->getSelectedItemIndex() == -1)
        {
            patchComboBox->changeItemText(lastCombo,
                                          patchComboBox->getText());
            if (patches[currentPatch])
                patches[currentPatch]->setAttribute("name", patchComboBox->getText());

            changed();
        }
        // Switch to the new patch.
        else
        {
            switchPatch(patchComboBox->getSelectedItemIndex());
        }

        lastCombo = patchComboBox->getSelectedId();
    }
}

void MainPanel::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == cpuSlider.get())
    {
        // CPU meter is updated from the timer; nothing to do here.
    }
}

//==============================================================================
StringArray MainPanel::getMenuBarNames()
{
    StringArray retval;

    retval.add("File");
    retval.add("Edit");
    retval.add("Options");
    retval.add("Help");

    return retval;
}

//==============================================================================
PopupMenu MainPanel::getMenuForIndex(int topLevelMenuIndex,
                                     const String &menuName)
{
    PopupMenu retval;

    if (menuName == "File")
    {
        retval.addCommandItem(commandManager, FileNew);
        retval.addCommandItem(commandManager, FileOpen);
        retval.addSeparator();
        retval.addCommandItem(commandManager, FileSave);
        retval.addCommandItem(commandManager, FileSaveAs);
        retval.addSeparator();
        retval.addCommandItem(commandManager, FileSaveAsDefault);
        retval.addCommandItem(commandManager, FileResetDefault);
        retval.addSeparator();
        retval.addCommandItem(commandManager, FileExit);
    }
    else if (menuName == "Edit")
    {
        retval.addCommandItem(commandManager, EditDeleteConnection);
        retval.addSeparator();
        retval.addCommandItem(commandManager, EditOrganisePatches);
        retval.addCommandItem(commandManager, EditUserPresetManagement);
    }
    else if (menuName == "Options")
    {
        retval.addCommandItem(commandManager, OptionsAudio);
        retval.addCommandItem(commandManager, OptionsPluginList);
        retval.addCommandItem(commandManager, OptionsPreferences);
        retval.addCommandItem(commandManager, OptionsColourSchemes);
        retval.addSeparator();
        retval.addCommandItem(commandManager, OptionsKeyMappings);
    }
    else if (menuName == "Help")
    {
        retval.addCommandItem(commandManager, HelpDocumentation);
        retval.addCommandItem(commandManager, HelpLog);
        retval.addSeparator();
        retval.addCommandItem(commandManager, HelpAbout);
    }

    return retval;
}

//==============================================================================
void MainPanel::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    ignoreUnused(menuItemID, topLevelMenuIndex);
}

//==============================================================================
ApplicationCommandTarget *MainPanel::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

//==============================================================================
void MainPanel::getAllCommands(Array<CommandID> &commands)
{
    const CommandID ids[] = { FileNew,
                              FileOpen,
                              FileSave,
                              FileSaveAs,
                              FileSaveAsDefault,
                              FileResetDefault,
                              FileExit,
                              EditDeleteConnection,
                              EditOrganisePatches,
                              EditUserPresetManagement,
                              OptionsPreferences,
                              OptionsAudio,
                              OptionsPluginList,
                              OptionsColourSchemes,
                              OptionsKeyMappings,
                              HelpAbout,
                              HelpDocumentation,
                              HelpLog,
                              PatchNextPatch,
                              PatchPrevPatch,
                              TransportPlay,
                              TransportRtz,
                              TransportTapTempo
                            };
    commands.addArray(ids, numElementsInArray(ids));
}

//==============================================================================
void MainPanel::getCommandInfo(const CommandID commandID,
                               ApplicationCommandInfo &result)
{
    const String fileCategory("File");
    const String editCategory("Edit");
    const String optionsCategory("Options");
    const String helpCategory("Help");
    const String patchCategory("Patch");
    const String transportCategory("Main Transport");

    switch(commandID)
    {
        case FileNew:
            result.setInfo("New",
                           "Creates a new pedalboard file to work from.",
                           fileCategory,
                           0);
            result.addDefaultKeypress('n', ModifierKeys::commandModifier);
            break;
        case FileOpen:
            result.setInfo("Open...",
                           "Opens an existing pedalboard file from disk.",
                           fileCategory,
                           0);
            result.addDefaultKeypress('o', ModifierKeys::commandModifier);
            break;
        case FileSave:
            result.setInfo("Save",
                           "Saves the current pedalboard file to disk.",
                           fileCategory,
                           0);
            result.addDefaultKeypress('s', ModifierKeys::commandModifier);
            break;
        case FileSaveAs:
            result.setInfo("Save As...",
                           "Saves the current pedalboard file to a new file on disk.",
                           fileCategory,
                           0);
            result.addDefaultKeypress('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
            break;
        case FileSaveAsDefault:
            result.setInfo("Save As Default",
                           "Saves the current pedalboard file as the default file to load.",
                           fileCategory,
                           0);
            break;
        case FileResetDefault:
            result.setInfo("Reset Default",
                           "Resets the default pedalboard file to its original state.",
                           fileCategory,
                           0);
            break;
        case FileExit:
            result.setInfo("Exit",
                           "Quits the program.",
                           fileCategory,
                           0);
            break;
        case EditDeleteConnection:
            result.setInfo("Delete selected connection(s)",
                           "Deletes the selected connection(s).",
                           editCategory,
                           0);
            result.addDefaultKeypress(KeyPress::deleteKey, ModifierKeys::noModifiers);
            result.addDefaultKeypress(KeyPress::backspaceKey, ModifierKeys::noModifiers);
            break;
        case EditOrganisePatches:
            result.setInfo("Organise patches",
                           "Opens the patch organiser.",
                           editCategory,
                           0);
            break;
        case EditUserPresetManagement:
            result.setInfo("User Preset Management",
                           "Opens the user preset management window.",
                           editCategory,
                           0);
            break;
        case OptionsPreferences:
            result.setInfo("Misc Settings",
                           "Displays miscellaneous settings.",
                           optionsCategory,
                           0);
            break;
        case OptionsAudio:
            result.setInfo("Audio Settings",
                           "Displays soundcard settings.",
                           optionsCategory,
                           0);
            break;
        case OptionsPluginList:
            result.setInfo("Plugin List",
                           "Options to scan and remove plugins.",
                           optionsCategory,
                           0);
            break;
        case OptionsColourSchemes:
            result.setInfo("Colour Schemes",
                           "Load and edit alternate colour schemes.",
                           optionsCategory,
                           0);
            break;
        case OptionsKeyMappings:
            result.setInfo("Application Mappings",
                           "Change the application mappings.",
                           optionsCategory,
                           0);
            break;
        case HelpDocumentation:
            result.setInfo("Documentation",
                           "Loads the documentation in your default browser.",
                           helpCategory,
                           0);
            result.addDefaultKeypress(KeyPress::F1Key, ModifierKeys::noModifiers);
            break;
        case HelpLog:
            result.setInfo("Event Log",
                           "Displays an event log for the program.",
                           helpCategory,
                           0);
            break;
        case HelpAbout:
            result.setInfo("About",
                           "Shows some details about the program.",
                           helpCategory,
                           0);
            break;
        case PatchNextPatch:
            result.setInfo("Next Patch",
                           "Switches to the next patch.",
                           patchCategory,
                           0);
            break;
        case PatchPrevPatch:
            result.setInfo("Previous Patch",
                           "Switches to the previous patch.",
                           patchCategory,
                           0);
            break;
        case TransportPlay:
            result.setInfo("Play/Pause",
                           "Plays/pauses the main transport.",
                           transportCategory,
                           0);
            result.addDefaultKeypress(KeyPress::spaceKey, ModifierKeys::noModifiers);
            break;
        case TransportRtz:
            result.setInfo("Return to Zero",
                           "Returns the main transport to the zero position.",
                           transportCategory,
                           0);
            break;
        case TransportTapTempo:
            result.setInfo("Tap Tempo",
                           "Used to set the tempo by 'tapping'.",
                           transportCategory,
                           0);
            break;
    }
}

//==============================================================================
bool MainPanel::perform(const InvocationInfo &info)
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());

    switch(info.commandID)
    {
        case FileNew:
        {
            File defaultFile = JuceHelperStuff::getAppDataFolder().getChildFile("default.pdl");

            // Delete all the patches.
            for (auto* p : patches)
                delete p;
            patches.clear();

            // Clear the PluginField.
            if (defaultFile.existsAsFile())
                loadDocument(defaultFile);
            else
            {
                if (field)
                    field->clear();

                // Load the default patch into patches.
                if (field)
                    patches.add(field->getXml().release());

                patchComboBox->clear(juce::dontSendNotification);
                patchComboBox->addItem("1 - <untitled>", 1);
                patchComboBox->addItem("<new patch>", 2);
                patchComboBox->setSelectedId(1, juce::sendNotification);
                currentPatch = 0;

                changed();

                if (field)
                    field->clearDoubleClickMessage();
            }
        }
        break;
        case FileOpen:
            loadFromUserSpecifiedFile(true);
            if (field)
                field->clearDoubleClickMessage();
            break;
        case FileSave:
            save(true, true);
            break;
        case FileSaveAs:
            saveAsInteractive(true);
            break;
        case FileSaveAsDefault:
        {
            File defaultFile = JuceHelperStuff::getAppDataFolder().getChildFile("default.pdl");
            saveDocument(defaultFile);
        }
        break;
        case FileResetDefault:
        {
            File defaultFile = JuceHelperStuff::getAppDataFolder().getChildFile("default.pdl");

            if (defaultFile.existsAsFile())
                defaultFile.deleteFile();
        }
        break;
        case FileExit:
            JUCEApplication::getInstance()->systemRequestedQuit();
            break;
        case EditDeleteConnection:
            if (field)
            {
                field->deleteConnection();
                changed();
            }
            break;
        case EditOrganisePatches:
        {
            // Save the current patch.
            savePatch();

            // Open the organiser.
            PatchOrganiser patchOrganiser(this, patches);
            patchOrganiser.setSize(400, 300);

            JuceHelperStuff::showModalDialog("Patch Organiser",
                                             &patchOrganiser,
                                             0,
                                             ColourScheme::getInstance().colours["Window Background"],
                                             true,
                                             true);
        }
        break;
        case EditUserPresetManagement:
        {
            // Open the preset window.
            UserPresetWindow win(&pluginList);

            win.setSize(400, 300);

            JuceHelperStuff::showModalDialog("User Preset Management",
                                             &win,
                                             0,
                                             ColourScheme::getInstance().colours["Window Background"],
                                             true,
                                             true);
        }
        break;
        case OptionsPreferences:
        {
            PreferencesDialog dlg(this,
                                  String(oscPortNumber),
                                  oscMulticastAddress);

            dlg.setSize(560, 500);

            JuceHelperStuff::showModalDialog("Misc Settings",
                                             &dlg,
                                             0,
                                             ColourScheme::getInstance().colours["Window Background"],
                                             true,
                                             true);
        }
        break;
        case OptionsAudio:
        {
            AudioDeviceSelectorComponent win(deviceManager,
                                             1,
                                             16,
                                             1,
                                             16,
                                             true,
                                             false,
                                             false,
                                             false);
            win.setSize(380, 400);

            savePatch();

            JuceHelperStuff::showModalDialog("Audio Settings",
                                             &win,
                                             0,
                                             ColourScheme::getInstance().colours["Window Background"],
                                             true,
                                             true);
            switchPatch(patchComboBox->getSelectedId()-1, false, true);

            auto audioState = deviceManager.createStateXml();
            if (audioState)
                PropertiesSingleton::getInstance().getUserSettings()->setValue("audioDeviceState", audioState.get());
        }
        break;
        case OptionsPluginList:
            if (!listWindow)
            {
                listWindow = new PluginListWindow(pluginList, this);
                listWindow->toFront(true);
            }
            break;
        case OptionsColourSchemes:
        {
            auto *dlg = new ColourSchemeEditor();

            dlg->setSize(500, 375);
            dlg->addChangeListener(this);

            JuceHelperStuff::showNonModalDialog("Colour Schemes",
                                                dlg,
                                                0,
                                                ColourScheme::getInstance().colours["Window Background"],
                                                true,
                                                true);
        }
        break;
        case OptionsKeyMappings:
        {
            ApplicationMappingsEditor editor(commandManager,
                                             field ? field->getMidiManager() : nullptr,
                                             field ? field->getOscManager() : nullptr);

            editor.setSize(414, 524);
            JuceHelperStuff::showModalDialog("Application Mappings",
                                             &editor,
                                             this,
                                             ColourScheme::getInstance().colours["Window Background"],
                                             false,
                                             true);
        }
        break;
        case HelpAbout:
        {
            AboutPage dlg(IPAddress::getLocalAddress().toString());

            dlg.setSize(400, 250);

            JuceHelperStuff::showModalDialog("About",
                                             &dlg,
                                             0,
                                             ColourScheme::getInstance().colours["Window Background"],
                                             true,
                                             true);
        }
        break;
        case HelpDocumentation:
        {
            File docDir;
#ifdef JUCE_WINDOWS
            docDir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("documentation");
#elif JUCE_LINUX
            // No Linux-specific documentation path is configured.
#elif JUCE_MAC
            docDir = File::getSpecialLocation(File::currentApplicationFile).getChildFile("Contents").getChildFile("Resources").getChildFile("documentation");
#endif
            File docIndex(docDir.getChildFile("index.html"));

            if (docIndex.existsAsFile())
            {
                URL docUrl(docIndex.getFullPathName());
                docUrl.launchInDefaultBrowser();
            }
            else
            {
                AlertWindow::showMessageBox(AlertWindow::WarningIcon,
                                            "Documentation Missing",
                                            "Could not find documentation/index.html");
            }
        }
        break;
        case HelpLog:
        {
            LogDisplay *dlg = new LogDisplay();

            dlg->setSize(600, 400);

            JuceHelperStuff::showNonModalDialog("Event Log",
                                                dlg,
                                                0,
                                                ColourScheme::getInstance().colours["Window Background"],
                                                true,
                                                true,
                                                false,
                                                true);
        }
        break;
        case PatchNextPatch:
            if (patchComboBox->getSelectedItemIndex() < (patchComboBox->getNumItems()-2))
                patchComboBox->setSelectedItemIndex(patchComboBox->getSelectedItemIndex()+1);
            else if (PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("LoopPatches", true))
                patchComboBox->setSelectedItemIndex(0);
            if (field)
                field->clearDoubleClickMessage();
            break;
        case PatchPrevPatch:
            if (patchComboBox->getSelectedItemIndex() > 0)
                patchComboBox->setSelectedItemIndex(patchComboBox->getSelectedItemIndex()-1);
            else if (PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("LoopPatches", true))
                patchComboBox->setSelectedItemIndex(patchComboBox->getNumItems()-2);
            if (field)
                field->clearDoubleClickMessage();
            break;
        case TransportPlay:
            MainTransport::getInstance()->toggleState();
            break;
        case TransportRtz:
            MainTransport::getInstance()->setReturnToZero();
            break;
        case TransportTapTempo:
        {
            juce::int64 delta;
            double tempo;
            double seconds;
            juce::int64 ticks = Time::getHighResolutionTicks();

            if (lastTempoTicks > 0)
            {
                delta = ticks - lastTempoTicks;

                seconds = Time::highResolutionTicksToSeconds(delta);
                if (seconds > 0.0)
                {
                    tempo = (1.0 / seconds) * 60.0;
                    if (field)
                        field->setTempo(tempo);

                    std::stringstream converterString;
                    converterString.precision(2);
                    converterString.fill('0');
                    converterString << std::fixed << tempo;
                    tempoEditor->setText(String(converterString.str().c_str()), false);
                }
            }
            lastTempoTicks = ticks;
        }
        break;
    }
    return true;
}

//==============================================================================
void MainPanel::setCommandManager(ApplicationCommandManager *manager)
{
    commandManager = manager;
}

//==============================================================================
void MainPanel::invokeCommandFromOtherThread(CommandID commandID)
{
    midiAppFifo.writeID(commandID);
}

//==============================================================================
void MainPanel::updateTempoFromOtherThread(double tempo)
{
    midiAppFifo.writeTempo(tempo);
}

//==============================================================================
void MainPanel::switchPatch(int newPatch, bool savePrev, bool reloadPatch)
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());
    if (!field)
        return;

    if (doNotSaveNextPatch)
    {
        savePrev = false;
        doNotSaveNextPatch = false;
    }

    if (((newPatch != currentPatch) && !reloadPatch) || !savePrev)
    {
        std::unique_ptr<XmlElement> patchToSave;

        if (savePrev)
        {
            patchToSave = field->getXml();
            patchToSave->setAttribute("name", patchComboBox->getItemText(lastCombo-1));
        }

        if ((newPatch > -1) && (newPatch < patches.size()))
        {
            // Save current patch.
            if (patchToSave)
            {
                delete patches[currentPatch];
                patches.set(currentPatch, patchToSave.release());
            }

            // Load new patch if it exists.
            currentPatch = newPatch;
            programChangePatch = currentPatch;
            XmlElement *patch = patches[currentPatch];
            if (patch)
            {
                field->loadFromXml(patch);
                field->clearDoubleClickMessage();

                tempoEditor->setText(String(field->getTempo(), 2), false);
            }
            else
            {
                String tempstr;

                field->clear();
                auto newPatchXml = field->getXml();

                tempstr << (currentPatch+1) << " - <untitled>";
                newPatchXml->setAttribute("name", tempstr);

                delete patches[currentPatch];
                patches.set(currentPatch, newPatchXml.release());

                tempoEditor->setText("120.00", false);
            }
            lastTempoTicks = 0;
        }
    }
}

//==============================================================================
void MainPanel::timerCallback(int timerId)
{
    switch(timerId)
    {
        case CpuTimer:
            cpuSlider->setColour(Slider::thumbColourId, ColourScheme::getInstance().colours["CPU Meter Colour"]);
            cpuSlider->setValue(deviceManager.getCpuUsage(), juce::dontSendNotification);
            break;
        case MidiAppTimer:
            if (midiAppFifo.getNumWaitingID() > 0)
                commandManager->invokeDirectly(midiAppFifo.readID(), true);
            if (midiAppFifo.getNumWaitingTempo() > 0)
            {
                std::stringstream converterString;
                double tempo = midiAppFifo.readTempo();
                PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());

                Logger::writeToLog(String(tempo));

                if (field)
                    field->setTempo(tempo);

                converterString.precision(2);
                converterString.fill('0');
                converterString << std::fixed << tempo;
                tempoEditor->setText(String(converterString.str().c_str()), false);
            }
            if (midiAppFifo.getNumWaitingPatchChange() > 0)
            {
                int index = midiAppFifo.readPatchChange();

                if ((index > -1) && (index < patches.size()))
                {
                    patchComboBox->setSelectedItemIndex(index);

                    if (warningBox->isVisible())
                        warningBox->setVisible(false);
                }
                else
                {
                    warningText.setIndex(index);
                    if (!warningBox->isVisible())
                        warningBox->setVisible(true);
                    else
                        warningBox->repaint();
                    startTimer(ProgramChangeTimer, 5 * 1000); // 5 seconds.
                }
            }
            break;
        case ProgramChangeTimer:
            warningBox->setVisible(false);
            stopTimer(ProgramChangeTimer);
            break;
    }
}

//==============================================================================
void MainPanel::changeListenerCallback(ChangeBroadcaster *changedObject)
{
    ColourSchemeEditor *ed = dynamic_cast<ColourSchemeEditor *>(changedObject);

    if (changedObject == MainTransport::getInstance())
    {
        if (MainTransport::getInstance()->getState())
            playButton->setImages(pauseImage.get());
        else
            playButton->setImages(playImage.get());

        // To decrement the counter.
        MainTransport::getInstance()->getReturnToZero();
    }
    else if (changedObject == dynamic_cast<PluginField *>(viewport->getViewedComponent()))
    {
        changed();
    }
    else if (ed)
    {
        // The colour scheme editor has updated our colour scheme.
        repaint();
    }
    else
    {
        // Save the plugin list every time it gets changed, so that if we are
        // scanning and it crashes, we have still saved the previous ones.
        auto savedPluginList = pluginList.createXml();

        if (savedPluginList)
        {
            PropertiesSingleton::getInstance().getUserSettings()->setValue("pluginList", savedPluginList.get());
            PropertiesSingleton::getInstance().saveIfNeeded();
        }
    }
}

//==============================================================================
void MainPanel::textEditorTextChanged(TextEditor &editor)
{
    if (&editor == tempoEditor.get())
    {
        PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());

        if (field)
            field->setTempo(tempoEditor->getText().getDoubleValue());
    }
}

//==============================================================================
void MainPanel::textEditorReturnKeyPressed(TextEditor &editor)
{
    if (&editor == tempoEditor.get())
    {
        PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());

        if (field)
            field->setTempo(tempoEditor->getText().getDoubleValue());
    }
    playButton->grabKeyboardFocus();
}

//==============================================================================
bool MainPanel::isInterestedInFileDrag(const StringArray& files)
{
    for (int i = 0; i < files.size(); ++i)
    {
        if (files[i].endsWith(".pdl") || files[i].endsWith(".filtergraph"))
            return true;
    }

    return false;
}

//==============================================================================
void MainPanel::filesDropped(const StringArray& files, int x, int y)
{
    ignoreUnused(x, y);

    for (int i = 0; i < files.size(); ++i)
    {
        File phil(files[i]);

        if (files[i].endsWith(".pdl"))
        {
            if (phil.existsAsFile())
                loadDocument(phil);
            else
            {
                String tempstr;
                tempstr << "Could not locate file: " << files[i];
                AlertWindow::showMessageBox(AlertWindow::WarningIcon,
                                            "File error",
                                            tempstr);
            }
        }
        else if (files[i].endsWith(".filtergraph"))
        {
            if (phil.existsAsFile())
            {
                PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());
                if (field && field->getFilterGraph())
                    field->getFilterGraph()->loadDocument(phil);
            }
            else
            {
                String tempstr;
                tempstr << "Could not locate file: " << files[i];
                AlertWindow::showMessageBox(AlertWindow::WarningIcon,
                                            "File error",
                                            tempstr);
            }
        }
    }
}

//==============================================================================
void MainPanel::setSocketPort(const String& port)
{
    int tempVal = port.getIntValue();

    if (tempVal > 0 && tempVal < 65536)
    {
        oscReceiver.disconnect();

        if (oscReceiver.connect(tempVal))
        {
            oscPortNumber = tempVal;
            PropertiesSingleton::getInstance().getUserSettings()->setValue("OSCPort", port);
        }
    }
}

//==============================================================================
void MainPanel::setSocketMulticast(const String& address)
{
    // JUCE's OSCReceiver does not support multicast directly.
    oscMulticastAddress = address;
    PropertiesSingleton::getInstance().getUserSettings()->setValue("OSCMulticastAddress", address);
}

//==============================================================================
void MainPanel::enableAudioInput(bool val)
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());

    if (field)
        field->enableAudioInput(val);

    PropertiesSingleton::getInstance().getUserSettings()->setValue("AudioInput", val);
}

//==============================================================================
void MainPanel::enableMidiInput(bool val)
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());

    if (field)
        field->enableMidiInput(val);

    PropertiesSingleton::getInstance().getUserSettings()->setValue("MidiInput", val);
}

//==============================================================================
void MainPanel::enableOscInput(bool val)
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());

    if (field)
        field->enableOscInput(val);

    if (val)
    {
        String port = PropertiesSingleton::getInstance().getUserSettings()->getValue("OSCPort", "5678");
        if (port == "")
            port = "5678";

        int p = port.getIntValue();
        if (p > 0 && p < 65536)
        {
            oscReceiver.disconnect();
            if (oscReceiver.connect(p))
                oscPortNumber = p;
        }
    }
    else
    {
        oscReceiver.disconnect();
    }

    PropertiesSingleton::getInstance().getUserSettings()->setValue("OscInput", val);
}

//==============================================================================
void MainPanel::setAutoMappingsWindow(bool val)
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());

    if (field)
        field->setAutoMappingsWindow(val);

    PropertiesSingleton::getInstance().getUserSettings()->setValue("AutoMappingsWindow", val);
}

//==============================================================================
void MainPanel::oscMessageReceived(const OSCMessage& message)
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());

    if (field && field->getOscManager())
        field->getOscManager()->messageReceived(message);
}

//==============================================================================
void MainPanel::oscBundleReceived(const OSCBundle& bundle)
{
    for (const auto& element : bundle)
    {
        if (element.isMessage())
            oscMessageReceived(element.getMessage());
        else if (element.isBundle())
            oscBundleReceived(element.getBundle());
    }
}

//==============================================================================
String MainPanel::getDocumentTitle()
{
    return "Pedalboard3 Patch File";
}

//==============================================================================
Result MainPanel::loadDocument (const File& file)
{
    auto root = XmlDocument(file).getDocumentElement();

    if (root)
    {
        if (root->hasTagName("Pedalboard2PatchFile") ||
            root->hasTagName("Pedalboard3PatchFile"))
        {
            // Clear existing patches.
            for (auto* p : patches)
                delete p;
            patches.clear();

            // Clear patchComboBox.
            patchComboBox->clear(juce::dontSendNotification);

            // If there are audio settings saved in this file and
            // pdlAudioSettings is set, load them.
            if (PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("pdlAudioSettings"))
            {
                XmlElement *deviceXml = root->getChildByName("DEVICESETUP");

                if (deviceXml)
                {
                    String err = deviceManager.initialise(2, 2, deviceXml, true);

                    if (err.isNotEmpty())
                    {
                        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                                         "Audio Device Error",
                                                         "Could not initialise audio settings loaded from .pdl file");
                    }
                }
            }

            // Load any xml patches into patches.
            for (int i = root->getNumChildElements() - 1; i >= 0; --i)
            {
                XmlElement *patch = root->getChildElement(i);

                if (patch->hasTagName("Patch"))
                {
                    patches.add(patch);
                    root->removeChildElement(patch, false);
                }
            }

            // Load the current patch.
            switchPatch(0, false);

            // Fill out patchComboBox.
            for (int i = 0; i < patches.size(); ++i)
                patchComboBox->addItem(patches[i]->getStringAttribute("name"), i+1);
            patchComboBox->addItem("<new patch>", patches.size()+1);
            patchComboBox->setSelectedId(1, juce::sendNotification);
        }
    }

    return Result::ok();
}

//==============================================================================
Result MainPanel::saveDocument (const File& file)
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());
    if (!field)
        return Result::fail("No plugin field available");

    auto main = std::make_unique<XmlElement>("Pedalboard3PatchFile");
    auto patch = field->getXml();

    // Save the current patch.
    patch->setAttribute("name", patchComboBox->getText());

    delete patches[currentPatch];
    patches.set(currentPatch, patch.release());

    for (auto* p : patches)
        main->addChildElement(p);

    if (PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("pdlAudioSettings"))
    {
        auto audioState = deviceManager.createStateXml();
        if (audioState)
            main->addChildElement(audioState.release());
    }

    main->writeToFile(file, "");

    // Remove the child "Patch" elements so they do not get deleted.
    for (int i = main->getNumChildElements() - 1; i >= 0; --i)
    {
        XmlElement *child = main->getChildElement(i);
        if (child->hasTagName("Patch"))
            main->removeChildElement(child, false);
    }

    return Result::ok();
}

//==============================================================================
File MainPanel::getLastDocumentOpened()
{
    return lastDocument;
}

//==============================================================================
void MainPanel::setLastDocumentOpened (const File& file)
{
    lastDocument = file;
}

//==============================================================================
void MainPanel::addPatch(XmlElement *patch)
{
    patches.add(patch);

    patchComboBox->changeItemText(patchComboBox->getNumItems(),
                                  patch->getStringAttribute("name"));
    patchComboBox->addItem("<new patch>", patchComboBox->getNumItems()+1);

    changed();
}

//==============================================================================
void MainPanel::savePatch()
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());
    if (!field)
        return;

    auto patch = field->getXml();
    patch->setAttribute("name", patchComboBox->getItemText(lastCombo-1));
    delete patches[currentPatch];
    patches.set(currentPatch, patch.release());
}

//==============================================================================
void MainPanel::duplicatePatch(int index)
{
    jassert((index > -1) && (index < patches.size()));

    // Save current patch.
    savePatch();

    // Setup the new ComboBox stuff.
    String tempstr;
    tempstr << patches[index]->getStringAttribute("name") << " (copy)";
    patchComboBox->changeItemText(patchComboBox->getNumItems(), tempstr);
    patchComboBox->addItem("<new patch>", patchComboBox->getNumItems()+1);

    // Copy the indexed patch to the new one.
    auto patch = std::make_unique<XmlElement>(*patches[index]);
    patch->setAttribute("name", tempstr);
    patches.add(patch.release());

    changed();
}

//==============================================================================
void MainPanel::nextSwitchDoNotSavePrev()
{
    doNotSaveNextPatch = true;
}

//==============================================================================
void MainPanel::switchPatchFromProgramChange(int newPatch)
{
    midiAppFifo.writePatchChange(newPatch);
}

//==============================================================================
MidiMappingManager *MainPanel::getMidiMappingManager()
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());
    if (field)
        return field->getMidiManager();
    return nullptr;
}

//==============================================================================
OscMappingManager *MainPanel::getOscMappingManager()
{
    PluginField *field = dynamic_cast<PluginField *>(viewport->getViewedComponent());
    if (field)
        return field->getOscManager();
    return nullptr;
}
