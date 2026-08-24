// MappingSlider.h - A two-tick Slider which can handle inverted values.
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

#ifndef MAPPINGSLIDER_H_
#define MAPPINGSLIDER_H_

#include <JuceHeader.h>

/// A two-tick Slider which can handle inverted values.
class JUCE_API MappingSlider : public Component,
                               public SettableTooltipClient,
                               public AsyncUpdater,
                               public juce::Button::Listener,
                               public juce::Label::Listener,
                               public juce::Value::Listener {
  public:
    /// Creates a slider.
    ///
    /// When created, you'll need to set up the slider's style and range with
    /// setMappingSliderStyle(), setRange(), etc.
    ///
    /// @param componentName The name to give the component.
    explicit MappingSlider(const String& componentName = {});

    /// Removes Value listeners and clears the popup display.
    ~MappingSlider() override;

    /// The types of slider available.
    enum MappingSliderStyle {
        LinearHorizontal,
        LinearVertical,
        LinearBar,
        Rotary,
        RotaryHorizontalDrag,
        RotaryVerticalDrag,
        IncDecButtons,
        TwoValueHorizontal,
        TwoValueVertical,
        ThreeValueHorizontal,
        ThreeValueVertical
    };

    /// Changes the type of slider interface being used.
    ///
    /// @param newStyle The new slider style to apply.
    void setMappingSliderStyle(MappingSliderStyle newStyle);

    /// Returns the slider's current style.
    MappingSliderStyle getMappingSliderStyle() const noexcept { return style; }

    /// Changes the properties of a rotary slider.
    ///
    /// @param startAngleRadians The start angle of the rotary arc in radians.
    /// @param endAngleRadians The end angle of the rotary arc in radians.
    /// @param stopAtEnd Whether the slider should stop at the end angles.
    void setRotaryParameters(float startAngleRadians, float endAngleRadians, bool stopAtEnd);

    /// Sets the distance the mouse has to move to drag the slider across
    /// the full extent of its range.
    ///
    /// @param distanceForFullScaleDrag The number of pixels for a full drag.
    void setMouseDragSensitivity(int distanceForFullScaleDrag);

    /// Returns the current sensitivity value.
    int getMouseDragSensitivity() const noexcept { return pixelsForFullDragExtent; }

    /// Changes the way the mouse is used when dragging the slider.
    ///
    /// @param isVelocityBased Whether to enable velocity-based mode.
    void setVelocityBasedMode(bool isVelocityBased);

    /// Returns true if velocity-based mode is active.
    bool getVelocityBasedMode() const noexcept { return isVelocityBased; }

    /// Changes aspects of the scaling used in velocity-sensitive mode.
    ///
    /// @param sensitivity The velocity sensitivity factor.
    /// @param threshold The velocity threshold in pixels.
    /// @param offset The velocity offset.
    /// @param userCanPressKeyToSwapMode Whether the user can press a key to
    ///        toggle velocity mode.
    void setVelocityModeParameters(double sensitivity = 1.0, int threshold = 1, double offset = 0.0,
                                   bool userCanPressKeyToSwapMode = true);

    double getVelocitySensitivity() const noexcept { return velocityModeSensitivity; }
    int getVelocityThreshold() const noexcept { return velocityModeThreshold; }
    double getVelocityOffset() const noexcept { return velocityModeOffset; }
    bool getVelocityModeIsSwappable() const noexcept { return userKeyOverridesVelocity; }

    /// Sets up a skew factor to alter the way values are distributed.
    ///
    /// @param factor The skew factor to apply.
    void setSkewFactor(double factor);

    /// Sets up a skew factor using a mid-point.
    ///
    /// @param sliderValueToShowAtMidPoint The value that should appear at the
    ///        mid-point of the slider.
    void setSkewFactorFromMidPoint(double sliderValueToShowAtMidPoint);

    /// Returns the current skew factor.
    double getSkewFactor() const noexcept { return skewFactor; }

    /// Used by setIncDecButtonsMode().
    enum IncDecButtonMode {
        incDecButtonsNotDraggable,
        incDecButtonsDraggable_AutoDirection,
        incDecButtonsDraggable_Horizontal,
        incDecButtonsDraggable_Vertical
    };

    /// When the style is IncDecButtons, this lets you turn on a mode where
    /// the mouse can be dragged on the buttons to drag the values.
    ///
    /// @param mode The inc/dec button drag mode to use.
    void setIncDecButtonsMode(IncDecButtonMode mode);

    /// The position of the slider's text-entry box.
    enum TextEntryBoxPosition { NoTextBox, TextBoxLeft, TextBoxRight, TextBoxAbove, TextBoxBelow };

    /// Changes the location and properties of the text-entry box.
    ///
    /// @param newPosition The new position of the text-entry box.
    /// @param isReadOnly Whether the text box should be read-only.
    /// @param textEntryBoxWidth The width of the text-entry box.
    /// @param textEntryBoxHeight The height of the text-entry box.
    void setTextBoxStyle(TextEntryBoxPosition newPosition, bool isReadOnly, int textEntryBoxWidth,
                         int textEntryBoxHeight);

    TextEntryBoxPosition getTextBoxPosition() const noexcept { return textBoxPos; }
    int getTextBoxWidth() const noexcept { return textBoxWidth; }
    int getTextBoxHeight() const noexcept { return textBoxHeight; }

    /// Makes the text-box editable.
    ///
    /// @param shouldBeEditable Whether the text box should be editable.
    void setTextBoxIsEditable(bool shouldBeEditable);

    /// Returns true if the text-box is read-only.
    bool isTextBoxEditable() const { return editableText; }

    /// Gives the text-box focus so the user can type directly into it.
    void showTextBox();

    /// Resets the text box and takes keyboard focus away from it.
    ///
    /// @param discardCurrentEditorContents Whether to discard the current
    ///        editor contents instead of committing them.
    void hideTextBox(bool discardCurrentEditorContents);

    /// Changes the slider's current value.
    ///
    /// @param newValue The new value to set.
    /// @param sendUpdateMessage Whether to send a change message to listeners.
    /// @param sendMessageSynchronously Whether to send the message
    ///        synchronously.
    void setValue(double newValue, bool sendUpdateMessage = true, bool sendMessageSynchronously = false);

    /// Returns the slider's current value.
    double getValue() const;

    /// Returns the Value object that represents the slider's current position.
    Value& getValueObject() { return currentValue; }

    /// Sets the limits that the slider's value can take.
    ///
    /// @param newMinimum The new minimum value.
    /// @param newMaximum The new maximum value.
    /// @param newInterval The step interval between values (0 for continuous).
    void setRange(double newMinimum, double newMaximum, double newInterval = 0);

    double getMaximum() const { return maximum; }
    double getMinimum() const { return minimum; }
    double getInterval() const { return interval; }

    /// For a slider with two or three thumbs, returns the lower value.
    double getMinValue() const;

    /// Returns the Value object for the minimum value.
    Value& getMinValueObject() noexcept { return valueMin; }

    /// For a slider with two or three thumbs, sets the lower value.
    ///
    /// @param newValue The new lower value to set.
    /// @param sendUpdateMessage Whether to send a change message to listeners.
    /// @param sendMessageSynchronously Whether to send the message
    ///        synchronously.
    /// @param allowNudgingOfOtherValues Whether to nudge the other thumb's
    ///        value to maintain the min/max relationship.
    void setMinValue(double newValue, bool sendUpdateMessage = true, bool sendMessageSynchronously = false,
                     bool allowNudgingOfOtherValues = false);

    /// For a slider with two or three thumbs, returns the higher value.
    double getMaxValue() const;

    /// Returns the Value object for the maximum value.
    Value& getMaxValueObject() noexcept { return valueMax; }

    /// For a slider with two or three thumbs, sets the higher value.
    ///
    /// @param newValue The new higher value to set.
    /// @param sendUpdateMessage Whether to send a change message to listeners.
    /// @param sendMessageSynchronously Whether to send the message
    ///        synchronously.
    /// @param allowNudgingOfOtherValues Whether to nudge the other thumb's
    ///        value to maintain the min/max relationship.
    void setMaxValue(double newValue, bool sendUpdateMessage = true, bool sendMessageSynchronously = false,
                     bool allowNudgingOfOtherValues = false);

    /// Sets the minimum and maximum thumb positions.
    ///
    /// @param newMinValue The new minimum value.
    /// @param newMaxValue The new maximum value.
    /// @param sendUpdateMessage Whether to send a change message to listeners.
    /// @param sendMessageSynchronously Whether to send the message
    ///        synchronously.
    void setMinAndMaxValues(double newMinValue, double newMaxValue, bool sendUpdateMessage = true,
                            bool sendMessageSynchronously = false);

    /// A class for receiving callbacks from a MappingSlider.
    class JUCE_API Listener {
      public:
        virtual ~Listener() {}

        /// Called when the slider's value is changed.
        ///
        /// @param slider The slider whose value changed.
        virtual void sliderValueChanged(MappingSlider* slider) = 0;

        /// Called when the slider is about to be dragged.
        ///
        /// @param slider The slider that is about to be dragged.
        virtual void sliderDragStarted(MappingSlider* slider);

        /// Called after a drag operation has finished.
        ///
        /// @param slider The slider whose drag has finished.
        virtual void sliderDragEnded(MappingSlider* slider);
    };

    /// Adds a listener to be called when this slider's value changes.
    ///
    /// @param listener The listener to add.
    void addListener(Listener* listener);

    /// Removes a previously-registered listener.
    ///
    /// @param listener The listener to remove.
    void removeListener(Listener* listener);

    /// Lets you choose whether double-clicking moves the slider to a
    /// given position.
    ///
    /// @param isDoubleClickEnabled Whether double-click is enabled.
    /// @param valueToSetOnDoubleClick The value to set on double-click.
    void setDoubleClickReturnValue(bool isDoubleClickEnabled, double valueToSetOnDoubleClick);

    /// Returns the values last set by setDoubleClickReturnValue().
    ///
    /// @param isEnabled Set to true if double-click is enabled.
    /// @return The value that would be set on double-click.
    double getDoubleClickReturnValue(bool& isEnabled) const;

    /// Tells the slider whether to keep sending change messages while
    /// the user is dragging.
    ///
    /// @param onlyNotifyOnRelease Whether to only notify on release.
    void setChangeNotificationOnlyOnRelease(bool onlyNotifyOnRelease);

    /// Changes whether the slider thumb jumps to the mouse position.
    ///
    /// @param shouldSnapToMouse Whether the thumb should snap to the mouse.
    void setMappingSliderSnapsToMousePosition(bool shouldSnapToMouse);

    /// Gives the slider a pop-up bubble while being dragged.
    ///
    /// @param isEnabled Whether the pop-up display is enabled.
    /// @param parentComponentToUse The parent component to add the pop-up to.
    void setPopupDisplayEnabled(bool isEnabled, Component* parentComponentToUse);

    /// Enables a right-click menu to change the way the slider works.
    ///
    /// @param menuEnabled Whether the popup menu is enabled.
    void setPopupMenuEnabled(bool menuEnabled);

    /// Stops the mouse scroll-wheel from moving the slider.
    ///
    /// @param enabled Whether the scroll wheel is enabled.
    void setScrollWheelEnabled(bool enabled);

    /// Returns which thumb is currently being dragged (0=main, 1=min, 2=max,
    /// -1=none).
    ///
    /// @return The index of the thumb being dragged, or -1 if none.
    int getThumbBeingDragged() const noexcept { return sliderBeingDragged; }

    /// Callback to indicate the user is about to start dragging.
    virtual void startedDragging();

    /// Callback to indicate the user has stopped dragging.
    virtual void stoppedDragging();

    /// Callback to indicate the user has moved the slider.
    virtual void valueChanged();

    /// Converts a text string to a value.
    ///
    /// @param text The text string to convert.
    /// @return The numeric value parsed from the text.
    virtual double getValueFromText(const String& text);

    /// Turns the slider's current value into a text string.
    ///
    /// @param value The value to convert.
    /// @return The text representation of the value.
    virtual String getTextFromValue(double value);

    /// Sets a suffix to append to the numeric value when displayed.
    ///
    /// @param suffix The suffix string to append.
    void setTextValueSuffix(const String& suffix);

    /// Returns the suffix that was set by setTextValueSuffix().
    String getTextValueSuffix() const;

    /// Allows a user-defined mapping of distance along the slider to value.
    ///
    /// @param proportion The proportion (0.0 to 1.0) along the slider length.
    /// @return The value corresponding to the given proportion.
    virtual double proportionOfLengthToValue(double proportion);

    /// Allows a user-defined mapping of value to position along the slider.
    ///
    /// @param value The value to convert.
    /// @return The proportion (0.0 to 1.0) along the slider length.
    virtual double valueToProportionOfLength(double value);

    /// Returns the X or Y coordinate of a value along the slider's length.
    ///
    /// @param value The value to locate.
    /// @return The pixel coordinate of the value along the slider.
    float getPositionOfValue(double value);

    /// Can be overridden to allow the slider to snap to user-definable values.
    ///
    /// @param attemptedValue The value the slider would naturally snap to.
    /// @param userIsDragging Whether the user is currently dragging.
    /// @return The value to use after snapping.
    virtual double snapValue(double attemptedValue, bool userIsDragging);

    /// Forces the text box to update its contents.
    void updateText();

    /// True if the slider moves horizontally.
    bool isHorizontal() const;

    /// True if the slider moves vertically.
    bool isVertical() const;

    /// A set of colour IDs to change the colour of various slider aspects.
    enum ColourIds {
        backgroundColourId = 0x1001200,
        thumbColourId = 0x1001300,
        trackColourId = 0x1001310,
        rotaryMappingSliderFillColourId = 0x1001311,
        rotaryMappingSliderOutlineColourId = 0x1001312,
        textBoxTextColourId = 0x1001400,
        textBoxBackgroundColourId = 0x1001500,
        textBoxHighlightColourId = 0x1001600,
        textBoxOutlineColourId = 0x1001700
    };

  protected:
    /// Applies the edited text box value to the slider.
    void labelTextChanged(Label*) override;
    /// Draws the slider using the current style and value.
    void paint(Graphics& g) override;
    /// Lays out the slider track, text box, and inc/dec buttons.
    void resized() override;
    /// Starts a drag or shows the popup menu depending on the click.
    void mouseDown(const MouseEvent& e) override;
    /// Ends the drag, restores the mouse, and sends change notifications.
    void mouseUp(const MouseEvent& e) override;
    /// Updates the dragged thumb's value based on mouse movement.
    void mouseDrag(const MouseEvent& e) override;
    /// Resets the slider to the double-click return value if enabled.
    void mouseDoubleClick(const MouseEvent& e) override;
    /// Changes the slider value in response to scroll wheel input.
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
    /// Toggles velocity-sensitive mode when modifier keys change.
    void modifierKeysChanged(const ModifierKeys& modifiers) override;
    /// Handles increment/decrement button clicks.
    void buttonClicked(Button* button) override;
    /// Recreates the text box and inc/dec buttons when the look changes.
    void lookAndFeelChanged() override;
    /// Repaints when the enabled state changes.
    void enablementChanged() override;
    /// Repaints when child focus changes.
    void focusOfChildComponentChanged(FocusChangeType cause) override;
    /// Notifies listeners of the current slider value.
    void handleAsyncUpdate() override;
    /// Refreshes the look when colours change.
    void colourChanged() override;
    /// Syncs the slider position when a Value object changes externally.
    void valueChanged(Value& value) override;

    /// Returns the best number of decimal places to use when displaying
    /// numbers, calculated from the slider's interval setting.
    int getNumDecimalPlacesToDisplay() const noexcept { return numDecimalPlaces; }

  private:
    Label* createMappingSliderTextBox(MappingSlider& slider);
    Button* createMappingSliderButton(const bool isIncrement);
    ImageEffectFilter* getMappingSliderEffect() { return nullptr; }

    void drawRotaryMappingSlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
                                 const float rotaryStartAngle, const float rotaryEndAngle, MappingSlider& slider);
    void drawLinearMappingSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos,
                                 float maxSliderPos, const MappingSlider::MappingSliderStyle style,
                                 MappingSlider& slider);
    void drawLinearMappingSliderBackground(Graphics& g, int x, int y, int width, int height, float sliderPos,
                                           float minSliderPos, float maxSliderPos,
                                           const MappingSlider::MappingSliderStyle style, MappingSlider& slider);
    int getMappingSliderThumbRadius(MappingSlider& slider) {
        return jmin(7, slider.getHeight() / 2, slider.getWidth() / 2) + 2;
    }
    void drawLinearMappingSliderThumb(Graphics& g, int x, int y, int width, int height, float sliderPos,
                                      float minSliderPos, float maxSliderPos,
                                      const MappingSlider::MappingSliderStyle style, MappingSlider& slider);

    ListenerList<Listener> listeners;
    Value currentValue, valueMin, valueMax;
    double lastCurrentValue, lastValueMin, lastValueMax;
    double minimum, maximum, interval, doubleClickReturnValue;
    double valueWhenLastDragged, valueOnMouseDown, skewFactor, lastAngle;
    double velocityModeSensitivity, velocityModeOffset, minMaxDiff;
    int velocityModeThreshold;
    float rotaryStart, rotaryEnd;
    int numDecimalPlaces;
    Point<int> mousePosWhenLastDragged;
    int mouseDragStartX, mouseDragStartY;
    int sliderRegionStart, sliderRegionSize;
    int sliderBeingDragged;
    int pixelsForFullDragExtent;
    Rectangle<int> sliderRect;
    String textSuffix;

    MappingSliderStyle style;
    TextEntryBoxPosition textBoxPos;
    int textBoxWidth, textBoxHeight;
    IncDecButtonMode incDecButtonMode;

    bool editableText : 1, doubleClickToValue : 1;
    bool isVelocityBased : 1, userKeyOverridesVelocity : 1, rotaryStop : 1;
    bool incDecButtonsSideBySide : 1, sendChangeOnlyOnRelease : 1, popupDisplayEnabled : 1;
    bool menuEnabled : 1, menuShown : 1, mouseWasHidden : 1, incDecDragged : 1;
    bool scrollWheelEnabled : 1, snapsToMousePos : 1;

    std::unique_ptr<Label> valueBox;
    std::unique_ptr<Button> incButton, decButton;

    class PopupDisplayComponent;
    friend class PopupDisplayComponent;
    std::unique_ptr<PopupDisplayComponent> popupDisplay;
    Component* parentForPopupDisplay;

    float getLinearMappingSliderPos(double value);
    void restoreMouseIfHidden();
    void sendDragStart();
    void sendDragEnd();
    double constrainedValue(double value) const;
    void triggerChangeMessage(bool synchronous);
    bool incDecDragDirectionIsHorizontal() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingSlider);
};

/// Compatibility typedef for old code. Newer code should use
/// MappingSlider::Listener directly.
typedef MappingSlider::Listener MappingSliderListener;

#endif
