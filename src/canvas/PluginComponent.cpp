// PluginComponent.cpp - Component representing a plugin/filter in the graph.
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

#include "PedalboardProcessors.h"
#include "PropertiesSingleton.h"
#include "BypassableInstance.h"
#include "JuceHelperStuff.h"
#include "PluginComponent.h"
#include "MappingsDialog.h"
#include "ColourScheme.h"
#include "PluginField.h"
#include "PresetBar.h"
#include "Vectors.h"
#include "Images.h"

#include <memory>

/// Generic editor that fills the background with the correct colour.
class NiallsGenericEditor : public juce::GenericAudioProcessorEditor
{
  public:
    /// Constructor.
    NiallsGenericEditor(AudioProcessor * const owner) :
        juce::GenericAudioProcessorEditor(owner)
    {
    }

    /// Fill the background the correct colour.
    void paint(juce::Graphics& g) override
    {
        g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
    }
};

PluginComponent::PluginComponent(AudioProcessorGraph::Node::Ptr n) :
    juce::Component(),
    node(n),
    pluginWindow(nullptr),
    beingDragged(false),
    dragX(0),
    dragY(0)
{
    BypassableInstance *bypassable = dynamic_cast<BypassableInstance *>(node->getProcessor());
    PedalboardProcessor *proc = nullptr;

    if (bypassable)
        proc = dynamic_cast<PedalboardProcessor *>(bypassable->getPlugin());

    pluginName = node->getProcessor()->getName();

    determineSize();

    titleLabel = std::make_unique<juce::Label>("titleLabel", pluginName);
    titleLabel->setBounds(5, 0, getWidth() - 10, 20);
    titleLabel->setInterceptsMouseClicks(false, false);
    titleLabel->setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    titleLabel->addListener(this);
    addAndMakeVisible(*titleLabel);

    if ((pluginName != "Audio Input") &&
        (pluginName != "Midi Input") &&
        (pluginName != "Audio Output") &&
        (pluginName != "OSC Input"))
    {
        std::unique_ptr<juce::Drawable> closeUp(JuceHelperStuff::loadSVGFromMemory(Vectors::closefilterbutton_svg,
                                                                                   Vectors::closefilterbutton_svgSize));
        std::unique_ptr<juce::Drawable> closeOver(JuceHelperStuff::loadSVGFromMemory(Vectors::closefilterbuttonover_svg,
                                                                                     Vectors::closefilterbuttonover_svgSize));
        std::unique_ptr<juce::Drawable> closeDown(JuceHelperStuff::loadSVGFromMemory(Vectors::closefilterbuttondown_svg,
                                                                                     Vectors::closefilterbuttondown_svgSize));
        std::unique_ptr<juce::Drawable> bypassOff(JuceHelperStuff::loadSVGFromMemory(Vectors::bypassbuttonoff_svg,
                                                                                     Vectors::bypassbuttonoff_svgSize));
        std::unique_ptr<juce::Drawable> bypassOn(JuceHelperStuff::loadSVGFromMemory(Vectors::bypassbuttonon_svg,
                                                                                    Vectors::bypassbuttonon_svgSize));

        // So the audio I/O etc. don't get their titles squeezed by the
        // non-existent close button.
        titleLabel->setBounds(5, 0, getWidth() - 17, 20);

        editButton = std::make_unique<juce::TextButton>("e", "Open plugin editor");
        editButton->setBounds(10, getHeight() - 30, 20, 20);
        editButton->addListener(this);
        addAndMakeVisible(*editButton);

        mappingsButton = std::make_unique<juce::TextButton>("m", "Open mappings editor");
        mappingsButton->setBounds(32, getHeight() - 30, 24, 20);
        mappingsButton->addListener(this);
        addAndMakeVisible(*mappingsButton);

        bypassButton = std::make_unique<juce::DrawableButton>("BypassFilterButton",
                                                               juce::DrawableButton::ImageOnButtonBackground);
        bypassButton->setImages(bypassOff.get(), nullptr, nullptr, nullptr, bypassOn.get());
        bypassButton->setClickingTogglesState(true);
        bypassButton->setBounds(getWidth() - 30, getHeight() - 30, 20, 20);
        bypassButton->addListener(this);
        addAndMakeVisible(*bypassButton);

        deleteButton = std::make_unique<juce::DrawableButton>("DeleteFilterButton",
                                                               juce::DrawableButton::ImageRaw);
        deleteButton->setImages(closeUp.get(), closeOver.get(), closeDown.get());
        deleteButton->setEdgeIndent(0);
        deleteButton->setBounds(getWidth() - 17, 5, 12, 12);
        deleteButton->addListener(this);
        addAndMakeVisible(*deleteButton);
    }

    if (proc)
    {
        juce::Component *comp = proc->getControls();
        juce::Point<int> compSize = proc->getSize();

        int tempint = (getWidth() / 2) - (compSize.getX() / 2);
        comp->setTopLeftPosition(tempint, 24);
        addAndMakeVisible(comp);
    }

    createPins();

    if (node->properties.getWithDefault("windowOpen", false))
        buttonClicked(editButton.get());
}

PluginComponent::~PluginComponent()
{
    deleteAllChildren();
    if (pluginWindow)
        delete pluginWindow;
}

void PluginComponent::paint(juce::Graphics& g)
{
    int i;
    std::map<juce::String, juce::Colour>& colours = ColourScheme::getInstance().colours;

    // Draw slight black outline.
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(1.0f,
                           1.0f,
                           static_cast<float>(getWidth()) - 2.0f,
                           static_cast<float>(getHeight()) - 2.0f,
                           5.0f,
                           1.0f);

    // Fill Component background.
    g.setColour(colours["Plugin Background"]);
    g.fillRoundedRectangle(2.0f,
                           2.0f,
                           static_cast<float>(getWidth()) - 4.0f,
                           static_cast<float>(getHeight()) - 4.0f,
                           5.0f);

    // Fill Component outline and text background.
    g.setColour(colours["Plugin Border"]);
    g.drawRoundedRectangle(3.0f,
                           3.0f,
                           static_cast<float>(getWidth()) - 6.0f,
                           static_cast<float>(getHeight()) - 6.0f,
                           5.0f,
                           4.0f);
    g.fillRoundedRectangle(1.0f,
                           1.0f,
                           static_cast<float>(getWidth()) - 2.0f,
                           16.0f,
                           5.0f);
    g.fillRect(1.0f, 16.0f, static_cast<float>(getWidth()) - 2.0f, 5.0f);

    // Draw the plugin name.
    g.setColour(colours["Text Colour"]);
    // nameText.draw(g);

    // Draw the input channels.
    for (i = 0; i < inputText.size(); ++i)
        inputText[i]->draw(g);

    // Draw the output channels.
    for (i = 0; i < outputText.size(); ++i)
        outputText[i]->draw(g);
}

void PluginComponent::moved()
{
    sendChangeMessage();
}

void PluginComponent::timerUpdate()
{
    BypassableInstance *bypassable = dynamic_cast<BypassableInstance *>(node->getProcessor());

    if (bypassable)
        bypassButton->setToggleState(bypassable->getBypass(), juce::dontSendNotification);
}

void PluginComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.y < 21)
    {
        if (e.getNumberOfClicks() == 2)
            titleLabel->showEditor();
        else
        {
            beginDragAutoRepeat(30);
            beingDragged = true;
            dragX = e.getPosition().getX();
            dragY = e.getPosition().getY();
            toFront(true);
        }
    }
}

void PluginComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (beingDragged)
    {
        juce::MouseEvent eField = e.getEventRelativeTo(getParentComponent());

        // parent = PluginField => parent = Viewport's contentHolder =>
        // parent = Viewport.
        juce::Viewport *viewport = dynamic_cast<juce::Viewport *>(getParentComponent()->getParentComponent()->getParentComponent());

        if (viewport)
        {
            juce::MouseEvent tempEv = e.getEventRelativeTo(viewport);

            viewport->autoScroll(tempEv.x, tempEv.y, 20, 4);
        }

        setTopLeftPosition(eField.x - dragX, eField.y - dragY);
        if (getX() < 0)
            setTopLeftPosition(0, getY());
        if (getY() < 0)
            setTopLeftPosition(getX(), 0);
        node->properties.set("x", getX());
        node->properties.set("y", getY());
        sendChangeMessage();
    }
}

void PluginComponent::mouseUp(const juce::MouseEvent& /*e*/)
{
    beingDragged = false;
}

void PluginComponent::buttonClicked(juce::Button *button)
{
    if ((button == editButton.get()) && !pluginWindow)
    {
        juce::AudioProcessorEditor *editor;

        // JUCE 8: createEditor() is private; use createEditorAndMakeActive()
        // which returns a raw pointer (or nullptr).
        editor = node->getProcessor()->createEditorAndMakeActive();

        // Create generic ui.
        if (!editor)
            editor = new NiallsGenericEditor(node->getProcessor());

        if (editor)
        {
            editor->setName(node->getProcessor()->getName());
            pluginWindow = new PluginEditorWindow(editor, this);
        }

        if (pluginWindow)
            node->properties.set("windowOpen", true);
    }
    else if (button == mappingsButton.get())
        openMappingsWindow();
    else if (button == bypassButton.get())
    {
        BypassableInstance *bypassable = dynamic_cast<BypassableInstance *>(node->getProcessor());

        if (bypassable)
            bypassable->setBypass(bypassButton->getToggleState());
    }
    else if (button == deleteButton.get())
    {
        PluginField *parent = dynamic_cast<PluginField *>(getParentComponent());

        if (pluginWindow)
            pluginWindow->closeButtonPressed();

        if (parent)
            parent->deleteFilter(node);

        delete this;
    }
}

void PluginComponent::labelTextChanged(juce::Label *label)
{
    int i, y;
    PluginField *parent = dynamic_cast<PluginField *>(getParentComponent());

    pluginName = label->getText();

    parent->updateProcessorName(node->nodeID.uid, pluginName);

    // Reset the Component's size/layout.
    determineSize(true);
    titleLabel->setBounds(5, 0, getWidth() - 17, 20);
    if (deleteButton)
        deleteButton->setBounds(getWidth() - 17, 5, 12, 12);
    if (bypassButton)
        bypassButton->setBounds(getWidth() - 30, getHeight() - 30, 20, 20);

    y = 25;
    for (i = 0; i < outputPins.size(); ++i)
    {
        juce::Point<int> pinPos;

        pinPos.setXY(getWidth() - 5, y);
        outputPins[i]->setTopLeftPosition(pinPos.getX(), pinPos.getY());

        y += 12;
    }

    for (i = 0; i < paramPins.size(); ++i)
    {
        juce::Point<int> pinPos;

        pinPos.setXY(getWidth() - 5, y);
        if (paramPins[i]->getX() > 0)
        {
            paramPins[i]->setTopLeftPosition(pinPos.getX(), pinPos.getY());

            y += 12;
        }
    }
}

void PluginComponent::setUserName(const juce::String& val)
{
    titleLabel->setText(val, juce::sendNotification);
}

void PluginComponent::setWindow(PluginEditorWindow *val)
{
    pluginWindow = val;
    if (pluginWindow)
        node->properties.set("windowOpen", true);
    else
        node->properties.set("windowOpen", false);
}

void PluginComponent::saveWindowState()
{
    if (pluginWindow)
    {
        node->properties.set("uiLastX", pluginWindow->getX());
        node->properties.set("uiLastY", pluginWindow->getY());
        node->properties.set("windowOpen", true);
    }
    else
        node->properties.set("windowOpen", false);
}

void PluginComponent::openMappingsWindow()
{
    juce::String tempstr;
    PluginField *parent = dynamic_cast<PluginField *>(getParentComponent());
    MappingsDialog dlg(parent->getMidiManager(),
                       parent->getOscManager(),
                       node,
                       parent->getMappingsForPlugin(node->nodeID.uid),
                       parent);

    tempstr << node->getProcessor()->getName() << " Mappings";
    JuceHelperStuff::showModalDialog(tempstr,
                                     &dlg,
                                     getParentComponent(),
                                     juce::Colour(0xFFEEECE1),
                                     false,
                                     true);
}

void PluginComponent::cacheCurrentPreset()
{
    auto preset = std::make_unique<juce::MemoryBlock>();

    node->getProcessor()->getCurrentProgramStateInformation(*preset);

    cachedPresets.insert(std::make_pair(node->getProcessor()->getCurrentProgram(),
                                        std::shared_ptr<juce::MemoryBlock>(preset.release())));
}

void PluginComponent::getCachedPreset(int index, juce::MemoryBlock& memBlock)
{
    std::map<int, std::shared_ptr<juce::MemoryBlock> >::iterator it;

    it = cachedPresets.find(index);

    // Make sure the cached preset actually exists.
    if (it != cachedPresets.end())
    {
        it->second->swapWith(memBlock);
        cachedPresets.erase(it);
    }
}

void PluginComponent::determineSize(bool onlyUpdateWidth)
{
    int i;
    juce::Rectangle<float> bounds;
    float nameWidth;
    float inputWidth = 0.0f;
    float outputWidth = 0.0f;
    int w = 150;
    int h = 100;
    float x;
    float y = 15.0f;
    int numInputPins = 0;
    int numOutputPins = 0;
    PedalboardProcessor *proc = nullptr;
    juce::Font tempFont(juce::FontOptions(14.0f, juce::Font::bold));
    AudioProcessor *plugin = node->getProcessor();
    BypassableInstance *bypassable = dynamic_cast<BypassableInstance *>(plugin);
    bool ignorePinNames = PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("IgnorePinNames", false);

    if (bypassable)
        proc = dynamic_cast<PedalboardProcessor *>(bypassable->getPlugin());

    nameText.clear();

    // Determine plugin name bounds.
    nameText.addLineOfText(tempFont, pluginName, 10.0f, y);
    bounds = nameText.getBoundingBox(0, -1, true);
    nameWidth = bounds.getWidth();

    // Add on space for the close button if necessary.
    if ((pluginName != "Audio Input") &&
        (pluginName != "Midi Input") &&
        (pluginName != "Audio Output") &&
        (pluginName != "OSC Input"))
    {
        nameWidth += 20.0f;
    }
    else
        nameWidth += 4.0f;

    inputText.clear();
    outputText.clear();

    if (!proc)
    {
        // Determine plugin input channel name bounds.
        y = 35.0f;
        tempFont.setHeight(12.0f);
        tempFont.setStyleFlags(juce::Font::plain);
        for (i = 0; i < plugin->getTotalNumInputChannels(); ++i)
        {
            if (!ignorePinNames)
            {
                auto g = new juce::GlyphArrangement;

                g->addLineOfText(tempFont, plugin->getInputChannelName(i), 10.0f, y);
                bounds = g->getBoundingBox(0, -1, true);

                inputText.add(g);

                if (bounds.getWidth() > inputWidth)
                    inputWidth = bounds.getWidth();
            }
            else
            {
                juce::String tempstr;
                auto g = new juce::GlyphArrangement;

                tempstr << "Input " << i + 1;
                g->addLineOfText(tempFont, tempstr, 10.0f, y);
                bounds = g->getBoundingBox(0, -1, true);

                inputText.add(g);

                if (bounds.getWidth() > inputWidth)
                    inputWidth = bounds.getWidth();
            }

            y += 13.0f;
            ++numInputPins;
        }

        // Add input parameter/midi name.
        if ((plugin->acceptsMidi() ||
             (plugin->getTotalNumInputChannels() > 0) ||
             (plugin->getTotalNumOutputChannels() > 0)) &&
             ((pluginName != "Audio Input") && (pluginName != "Audio Output")))
        {
            {
                auto g = new juce::GlyphArrangement;

                g->addLineOfText(tempFont, "param", 10.0f, y);
                bounds = g->getBoundingBox(0, -1, true);

                inputText.add(g);

                if (bounds.getWidth() > inputWidth)
                    inputWidth = bounds.getWidth();
            }

            y += 13.0f;
            ++numInputPins;
        }

        // Determine plugin output channel name bounds.
        y = 35.0f;
        for (i = 0; i < plugin->getTotalNumOutputChannels(); ++i)
        {
            if (!ignorePinNames)
            {
                auto g = new juce::GlyphArrangement;

                g->addLineOfText(tempFont,
                                 plugin->getOutputChannelName(i),
                                 0.0f,
                                 y);
                bounds = g->getBoundingBox(0, -1, true);

                outputText.add(g);

                if (bounds.getWidth() > outputWidth)
                    outputWidth = bounds.getWidth();
            }
            else
            {
                juce::String tempstr;
                auto g = new juce::GlyphArrangement;

                tempstr << "Output " << i + 1;
                g->addLineOfText(tempFont,
                                 tempstr,
                                 0.0f,
                                 y);
                bounds = g->getBoundingBox(0, -1, true);

                outputText.add(g);

                if (bounds.getWidth() > outputWidth)
                    outputWidth = bounds.getWidth();
            }

            y += 13.0f;
            ++numOutputPins;
        }

        // Add output parameter/midi name.
        if (plugin->producesMidi() || (plugin->getName() == "OSC Input"))
        {
            {
                auto g = new juce::GlyphArrangement;

                g->addLineOfText(tempFont,
                                 "param",
                                 0.0f,
                                 y);
                bounds = g->getBoundingBox(0, -1, true);

                outputText.add(g);

                if (bounds.getWidth() > outputWidth)
                    outputWidth = bounds.getWidth();
            }

            y += 13.0f;
            ++numOutputPins;
        }

        if (nameWidth > (inputWidth + outputWidth + 30.0f))
            w = static_cast<int>(nameWidth + 12.0f);
        else
            w = static_cast<int>(inputWidth + outputWidth + 30.0f);

        // Shift output texts to where they should be.
        {
            x = (w - outputWidth - 10.0f);

            for (i = 0; i < outputText.size(); ++i)
                outputText[i]->moveRangeOfGlyphs(0, -1, x, 0.0f);
        }

        h = juce::jmax(numInputPins, numOutputPins);
        h *= 13;

        if ((pluginName != "Audio Input") &&
            (pluginName != "Midi Input") &&
            (pluginName != "Audio Output") &&
            (pluginName != "OSC Input"))
        {
            h += 60;
        }
        else
            h += 34;
    }
    else
    {
        juce::Point<int> compSize = proc->getSize();

        if (nameWidth > (compSize.getX() + 24.0f))
            w = static_cast<int>(nameWidth + 20.0f);
        else
            w = static_cast<int>(compSize.getX() + 24.0f);

        h = compSize.getY() + 52;
    }

    if (onlyUpdateWidth)
        setSize(w, getHeight());
    else
        setSize(w, h);
}

void PluginComponent::createPins()
{
    int i;
    int y;
    PluginPinComponent *pin;
    AudioProcessor *plugin = node->getProcessor();
    const AudioProcessorGraph::NodeID uid = node->nodeID;

    y = 25;
    for (i = 0; i < plugin->getTotalNumInputChannels(); ++i)
    {
        juce::Point<int> pinPos;

        pin = new PluginPinComponent(false, uid, i, false);
        pinPos.setXY(-5, y);
        pin->setTopLeftPosition(pinPos.getX(), pinPos.getY());
        addAndMakeVisible(pin);

        inputPins.add(pin);

        y += 12;
    }

    if ((plugin->acceptsMidi() ||
         (plugin->getTotalNumInputChannels() > 0) ||
         (plugin->getTotalNumOutputChannels() > 0)) &&
         ((pluginName != "Audio Input") && (pluginName != "Audio Output")))
    {
        juce::Point<int> pinPos;

        pin = new PluginPinComponent(false, uid, AudioProcessorGraph::midiChannelIndex, true);
        pinPos.setXY(-5, y);
        pin->setTopLeftPosition(pinPos.getX(), pinPos.getY());
        addAndMakeVisible(pin);

        paramPins.add(pin);

        y += 12;
    }

    y = 25;
    for (i = 0; i < plugin->getTotalNumOutputChannels(); ++i)
    {
        juce::Point<int> pinPos;

        pin = new PluginPinComponent(true, uid, i, false);
        pinPos.setXY(getWidth() - 5, y);
        pin->setTopLeftPosition(pinPos.getX(), pinPos.getY());
        addAndMakeVisible(pin);

        outputPins.add(pin);

        y += 12;
    }

    if (plugin->producesMidi() || (plugin->getName() == "OSC Input"))
    {
        juce::Point<int> pinPos;

        pin = new PluginPinComponent(true, uid, AudioProcessorGraph::midiChannelIndex, true);
        pinPos.setXY(getWidth() - 5, y);
        pin->setTopLeftPosition(pinPos.getX(), pinPos.getY());
        addAndMakeVisible(pin);

        paramPins.add(pin);

        y += 12;
    }
}

PluginPinComponent::PluginPinComponent(bool dir, AudioProcessorGraph::NodeID id, int chan, bool param) :
    juce::Component(),
    direction(dir),
    uid(id),
    channel(chan),
    parameterPin(param)
{
    setSize(10, 12);
}

PluginPinComponent::~PluginPinComponent()
{
}

void PluginPinComponent::paint(juce::Graphics& g)
{
    const float w = static_cast<float>(getWidth()) - 2;
    const float h = static_cast<float>(getHeight()) - 2;

    g.setColour(juce::Colours::black);
    g.drawEllipse(1, 1, w, h, 1.0f);

    if (!parameterPin)
        g.setColour(ColourScheme::getInstance().colours["Audio Connection"]);
    else
        g.setColour(ColourScheme::getInstance().colours["Parameter Connection"]);
    g.fillEllipse(1, 1, w, h);
}

void PluginPinComponent::mouseDown(const juce::MouseEvent& e)
{
    if (direction)
    {
        PluginField *field = findParentComponentOfClass<PluginField>();

        field->addConnection(this, (e.mods.isShiftDown() && !parameterPin));
    }
}

void PluginPinComponent::mouseDrag(const juce::MouseEvent& e)
{
    PluginField *field = findParentComponentOfClass<PluginField>();
    juce::MouseEvent e2 = e.getEventRelativeTo(field);

    field->dragConnection(e2.x - 5, e2.y);
}

void PluginPinComponent::mouseUp(const juce::MouseEvent& e)
{
    if (e.mods.testFlags(juce::ModifierKeys::leftButtonModifier))
    {
        PluginField *field = findParentComponentOfClass<PluginField>();
        juce::MouseEvent e2 = e.getEventRelativeTo(field);

        field->releaseConnection(e2.x, e2.y);
    }
}

PluginEditorWindow::PluginEditorWindow(juce::AudioProcessorEditor *editor,
                                       PluginComponent *c) :
    juce::DocumentWindow(c->getUserName(),
                         ColourScheme::getInstance().colours["Window Background"],
                         juce::DocumentWindow::minimiseButton | juce::DocumentWindow::maximiseButton | juce::DocumentWindow::closeButton),
    component(c)
{
    int x, y;

    centreWithSize(400, 300);

    setResizeLimits(396, 32, 10000, 10000);
    setUsingNativeTitleBar(true);
    setContentOwned(new EditorWrapper(editor, c), true);
    setAlwaysOnTop(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("WindowsOnTop", false));

    // Fix for my favourite synth being unable to handle being resizable.
    if ((c->getNode()->getProcessor()->getName() != "VAZPlusVSTi") &&
        !PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("fixedSizeWindows", true))
    {
        setResizable(true, false);
    }

    x = component->getNode()->properties.getWithDefault("uiLastX", getX());
    if (x < 10)
        x = 10;
    y = component->getNode()->properties.getWithDefault("uiLastY", getY());
    if (y < 10)
        y = 10;
    setTopLeftPosition(x, y);

    setVisible(true);
    getPeer()->setIcon(juce::ImageCache::getFromMemory(Images::icon512_png,
                                                       Images::icon512_pngSize));
}

PluginEditorWindow::~PluginEditorWindow()
{
    component->getNode()->properties.set("uiLastX", getX());
    component->getNode()->properties.set("uiLastY", getY());
}

void PluginEditorWindow::closeButtonPressed()
{
    component->setWindow(nullptr);
    delete this;
}

PluginEditorWindow::EditorWrapper::EditorWrapper(juce::AudioProcessorEditor *ed,
                                                 PluginComponent *comp) :
    editor(ed),
    component(comp)
{
    presetBar = new PresetBar(component);

    presetBar->setBounds(0, 0, 396, 32);
    addAndMakeVisible(presetBar);

    editor->setTopLeftPosition(0, 32);
    addAndMakeVisible(editor);

    if (editor->getWidth() < 396)
        setSize(396, 32 + editor->getHeight());
    else
        setSize(editor->getWidth(), 32 + editor->getHeight());
}

PluginEditorWindow::EditorWrapper::~EditorWrapper()
{
    deleteAllChildren();
}

void PluginEditorWindow::EditorWrapper::resized()
{
    presetBar->setSize(getWidth(), 32);
    editor->setSize(getWidth(), getHeight() - 32);
}

void PluginEditorWindow::EditorWrapper::childBoundsChanged(juce::Component *child)
{
    if (child == editor)
    {
        if (editor->getWidth() < 396)
            setSize(396, 32 + editor->getHeight());
        else
            setSize(editor->getWidth(), 32 + editor->getHeight());
    }
}

PluginConnection::PluginConnection(PluginPinComponent *s,
                                   PluginPinComponent *d,
                                   bool allOutputs) :
    juce::Component(),
    source(s),
    selected(false),
    representsAllOutputs(allOutputs)
{
    if (source)
    {
        juce::Point<int> tempPoint(source->getX() + 5, source->getY() + 6);
        PluginField *field = source->findParentComponentOfClass<PluginField>();

        tempPoint = field->getLocalPoint(source->getParentComponent(), tempPoint);
        setTopLeftPosition(tempPoint.getX(), tempPoint.getY());

        dynamic_cast<PluginComponent *>(source->getParentComponent())->addChangeListener(this);

        paramCon = source->getParameterPin();
    }

    if (d)
        setDestination(d);
    else
        destination = nullptr;
}

PluginConnection::~PluginConnection()
{
    if (source)
    {
        PluginComponent *sourceComp = dynamic_cast<PluginComponent *>(source->getParentComponent());
        if (sourceComp)
            sourceComp->removeChangeListener(this);
    }
    if (destination)
    {
        PluginComponent *destComp = dynamic_cast<PluginComponent *>(destination->getParentComponent());
        if (destComp)
            destComp->removeChangeListener(this);
    }
}

void PluginConnection::paint(juce::Graphics& g)
{
    juce::Colour tempCol;

    if (representsAllOutputs)
    {
        g.setColour(juce::Colours::red);
        g.strokePath(drawnCurve, juce::PathStrokeType(4.0f));
    }
    else
    {
        g.setColour(juce::Colours::black);
        g.strokePath(drawnCurve, juce::PathStrokeType(1.0f));
    }

    if (!paramCon)
        tempCol = ColourScheme::getInstance().colours["Audio Connection"];
    else
        tempCol = ColourScheme::getInstance().colours["Parameter Connection"];

    if (selected)
        g.setColour(tempCol.brighter(0.75f));
    else
        g.setColour(tempCol);
    g.fillPath(drawnCurve);
}

void PluginConnection::mouseDown(const juce::MouseEvent& /*e*/)
{
    selected = !selected;
    repaint();
}

bool PluginConnection::hitTest(int x, int y)
{
    bool retval = false;

    if (drawnCurve.contains(static_cast<float>(x), static_cast<float>(y)))
    {
        // Make sure clicking the source pin doesn't select this connection.
        if (x > 10)
            retval = true;
    }

    return retval;
}

void PluginConnection::changeListenerCallback(juce::ChangeBroadcaster * /*changedObject*/)
{
    juce::Component *field = getParentComponent();

    if (source && destination)
    {
        juce::Point<int> sourcePoint(source->getX() + 5, source->getY() + 6);
        juce::Point<int> destPoint(destination->getX() + 5, destination->getY() + 6);
        sourcePoint = field->getLocalPoint(source->getParentComponent(), sourcePoint);
        destPoint = field->getLocalPoint(destination->getParentComponent(), destPoint);

        updateBounds(sourcePoint.getX(),
                     sourcePoint.getY(),
                     destPoint.getX(),
                     destPoint.getY());
    }
}

void PluginConnection::drag(int x, int y)
{
    juce::Component *field = getParentComponent();

    if (source)
    {
        juce::Point<int> sourcePoint(source->getX() + 5, source->getY() + 6);
        sourcePoint = field->getLocalPoint(source->getParentComponent(), sourcePoint);

        updateBounds(sourcePoint.getX(), sourcePoint.getY(), x, y);
    }
}

void PluginConnection::setDestination(PluginPinComponent *d)
{
    PluginField *field = source->findParentComponentOfClass<PluginField>();

    destination = d;
    if (destination)
        dynamic_cast<PluginComponent *>(destination->getParentComponent())->addChangeListener(this);

    if (source && destination)
    {
        juce::Point<int> sourcePoint(source->getX() + 5, source->getY() + 6);
        juce::Point<int> destPoint(destination->getX() + 5, destination->getY() + 6);
        sourcePoint = field->getLocalPoint(source->getParentComponent(), sourcePoint);
        destPoint = field->getLocalPoint(destination->getParentComponent(), destPoint);

        if (destPoint.getX() > sourcePoint.getX())
            updateBounds(sourcePoint.getX(), sourcePoint.getY(), destPoint.getX(), destPoint.getY());
        else
            updateBounds(destPoint.getX(), destPoint.getY(), sourcePoint.getX(), sourcePoint.getY());
    }
}

void PluginConnection::setRepresentsAllOutputs(bool val)
{
    representsAllOutputs = val;
}

void PluginConnection::getPoints(int& sX, int& sY, int& dX, int& dY)
{
    int tX, tY;

    if (dY < sY)
    {
        if (sX < dX)
        {
            dX -= sX;
            sX = 5;
            dX += 5;

            tX = dX;
            dX = sX;
            sX = tX;
        }
        else
        {
            sX -= dX;
            dX = 5;
            sX += 5;

            tX = dX;
            dX = sX;
            sX = tX;
        }

        sY -= dY;
        dY = 5;
        sY += 5;

        tY = dY;
        dY = sY;
        sY = tY;
    }
    else if (sX < dX)
    {
        dX -= sX;
        sX = 5;
        dX += 5;

        dY -= sY;
        sY = 5;
        dY += 5;
    }
    else
    {
        sX -= dX;
        dX = 5;
        sX += 5;

        tX = dX;
        dX = sX;
        sX = tX;

        sY -= dY;
        dY = 5;
        sY += 5;

        tY = dY;
        dY = sY;
        sY = tY;
    }
}

void PluginConnection::updateBounds(int sX, int sY, int dX, int dY)
{
    int left, top, width, height;

    juce::Path tempPath;
    juce::PathStrokeType drawnType(9.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded);

    if (sX < dX)
    {
        left = sX;
        width = dX - sX;
    }
    else
    {
        left = dX;
        width = sX - dX;
    }
    if (sY < dY)
    {
        top = sY;
        height = dY - sY;
    }
    else
    {
        top = dY;
        height = sY - dY;
    }

    getPoints(sX, sY, dX, dY);

    tempPath.startNewSubPath(static_cast<float>(sX), static_cast<float>(sY));
    tempPath.cubicTo((static_cast<float>(width) * 0.5f) + juce::jmin(sX, dX),
                     static_cast<float>(sY),
                     (static_cast<float>(width) * 0.5f) + juce::jmin(sX, dX),
                     static_cast<float>(dY),
                     static_cast<float>(dX),
                     static_cast<float>(dY));
    drawnType.createStrokedPath(drawnCurve, tempPath);

    setBounds(left - 5, top - 5, width + 10, height + 10);
}
