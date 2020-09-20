#include "Transport.h"

/// Constructor.
Transport::Transport(const MediaManagerData& mmd) {
    // Set the local managerData member to the copy passed in and add this transport as a listener.
    managerData = mmd;
    managerData.addListener(this);
    
    // Make playPauseButton behave as a toggle button.
    playPauseButton.setClickingTogglesState(true);
    // Draw the playPauseButton, add and make it visible, then add this transport as its button listener.
    drawPlayButton(playPauseButton);
    addAndMakeVisible(playPauseButton);
    playPauseButton.addListener(this);
    
    // Draw the goToStartButton, add and make it visible, then add this transport as its button listener.
    drawGoToStartButton(goToStartButton);
    addAndMakeVisible(goToStartButton);
    goToStartButton.addListener(this);
    
    // Get the gain setting from the manager data, draw the gainButton using that gain setting, then add and make it visible, and add this transport as its button listener.
    auto gain = managerData.getTransportGain();
    drawGainButton(gainButton, gain);
    addAndMakeVisible(gainButton);
    gainButton.addListener(this);
    
    // Set the gainSlider range to 0.0-1.0, initialize it with the manager's gain setting, add and make it visible and add this transport as a slider listener.
    gainSlider.setRange(0.0, 1.0);
    gainSlider.setValue(gain, juce::dontSendNotification);
    addAndMakeVisible(gainSlider);
    gainSlider.addListener(this);
    
    // Add the tempo slider, set its style to Slider::LinearBar and its text value suffix to " bpm".
    // Its range should be set to 40 to 208 and its initial value to the manager's tempo setting.
    tempoSlider.setSliderStyle(Slider::LinearBar);
    tempoSlider.setTextValueSuffix(" bpm");
    tempoSlider.setRange(40, 208);
    tempoSlider.setValue(managerData.getTransportTempo());
    tempoSlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(tempoSlider);
    tempoSlider.addListener(this);
    
    // Configure the start time label to use Justification::centeredRight, and a font size of 12.0. Add and make visible.
    currentTimeLabel.setJustificationType(juce::Justification(Justification::centredRight));
    currentTimeLabel.setFont(juce::Font(12.0));
    addAndMakeVisible(currentTimeLabel);
    currentTimeLabel.setText("00:00", dontSendNotification);
    
    // Configure the end time label to use Justification::centeredLeft and a font size of 12.0. Add and make visible.
    endTimeLabel.setJustificationType(juce::Justification(Justification::centredLeft));
    endTimeLabel.setFont(juce::Font(12.0));
    addAndMakeVisible(endTimeLabel);
    endTimeLabel.setText("100:00", dontSendNotification);
    
    // Add the position slider, set its range to 1.0 and add this component as its listener.
    addAndMakeVisible(positionSlider);
    positionSlider.setRange(0.0, 1.0);
    positionSlider.addListener(this);
    
    // Set the size of the transport to 250 width and 78 height.
    // Set the component visible.
    setSize(250, 78);
    setVisible(true);
}

/// Destructor.
Transport::~Transport() {
    // Remove the transport as a listener of managerData.
    managerData.removeListener(this);
}

void Transport::paint(Graphics& gr) {
    gr.setColour(Colours::grey);
    gr.drawRoundedRectangle(getLocalBounds().toFloat(), 8.0, 2.0);

    // Handy for debugging
//    gr.setColour(Colours::red);
//    gr.drawRect(playPauseButton.getBounds());
//    gr.drawRect(goToStartButton.getBounds());
//    gr.drawRect(gainButton.getBounds());
//    gr.drawRect(tempoSlider.getBounds());
//    gr.drawRect(gainSlider.getBounds());
//    gr.drawRect(currentTimeLabel.getBounds());
//    gr.drawRect(endTimeLabel.getBounds());
//    gr.drawRect(positionSlider.getBounds());
}

void Transport::resized() {
    // All subcomponents should be indented 6 pixels from the bounds of of the transport component.
    auto padding = 6;
    auto playPauseButtonSize = 36;
    auto gtsGainButtonSize = 24;
    auto bounds = getLocalBounds().reduced(padding);
    auto timeLabelWidth = 44;
    
    // The playPause button should be centered in the transport with size 36.
    playPauseButton.setSize(playPauseButtonSize, playPauseButtonSize);
    auto topVerticalCenter = padding + playPauseButtonSize/2;
    playPauseButton.setCentrePosition(bounds.getCentreX(), topVerticalCenter);
    
    // The goToStartButton and gainButton are size 24, and positioned on opposite sides of the playPauseButton vertically centered within the playPause button's height.
    goToStartButton.setSize(gtsGainButtonSize, gtsGainButtonSize);
    goToStartButton.setCentrePosition(playPauseButton.getX()-(gtsGainButtonSize/2), topVerticalCenter);
    gainButton.setSize(gtsGainButtonSize, gtsGainButtonSize);
    gainButton.setCentrePosition(playPauseButton.getRight()+(gtsGainButtonSize/2), topVerticalCenter);
    
    // The tempoSlider is centered in height the same as the goToStartButton and occupies the remainer of the left-side space but keeping a 6 pixel indent.
    tempoSlider.setSize(goToStartButton.getX()-(2*padding), gtsGainButtonSize);
    tempoSlider.setCentrePosition(tempoSlider.getWidth()/2 + padding, topVerticalCenter);
    
    // The gainSlider is centered in height the same as the gainButton but starting 6 pixels to the right of the gainButton and taking the rest of the width keeping a 6 pixel indent from the right edge.
    gainSlider.setSize(bounds.getRight()-gainButton.getRight(), gtsGainButtonSize);
    gainSlider.setCentrePosition(gainButton.getRight() + gainSlider.getWidth()/2, topVerticalCenter);
    
    // Line 2: the currentTimeLabel and endTimeLabel are left/right justified in the component with width of 44 and height of 24.
    // The positionSlider should occupy the entire width between them.
    auto bottom = bounds.removeFromBottom(24);
    currentTimeLabel.setBounds(bottom.removeFromLeft(timeLabelWidth));
    endTimeLabel.setBounds(bottom.removeFromRight(timeLabelWidth));
    positionSlider.setBounds(bottom);
}

//============================================================================
// JUCE Component Callbacks

void Transport::buttonClicked(juce::Button* button) {
    if (button == &playPauseButton) {
        DBG("playPauseButton");
        // Flip managerData's play/pause state to its opposite.
        auto state = !managerData.getTransportPlaying();
        // Call managerData.setTransportPlaying() and pass it the new state value of the button.
        // Pass this component as the second value so we don't get retriggered by the callback.
        managerData.setTransportPlaying(state, this);
    } else if (button == &goToStartButton) {
        // Set the position slider value to to zero, used dontSendNotification as the second argument so it doesnt trigger listeners.
        positionSlider.setValue(0.0, dontSendNotification);
        // Call managerData.setTransportRewind().
        managerData.setTransportRewind();
    } else if (button == &gainButton) {
        // Clicking the gain button either mutes or sets the gainSlider to .5 gain. Pass juce::sendNotificationAsync as the second value.
        if (gainSlider.getValue() == 0.0) {
            gainSlider.setValue(0.5, sendNotificationAsync);
        } else {
            gainSlider.setValue(0.0, sendNotificationAsync);
        }
    }
}

void Transport::sliderValueChanged(juce::Slider* slider) {
    if (slider == &positionSlider) {
        // Pass the current slider position to managerData.setTransportPosition() passing the transport component ('this')
        // as the second argument to exclude the transport from being called again.
        managerData.setTransportPosition(slider->getValue(), this);
        // Draw the current time label in the Transport.
        drawCurrentTimeLabel();
    } else if (slider == &gainSlider) {
        // Pass the current slider position to managerData.setTransportGain() passing the transport component ('this') as the second argument to exclude the transport from being called again.
        managerData.setTransportGain(slider->getValue(), this);
        // Redraw gainButton so it shows the new state.
        drawGainButton(gainButton, managerData.getTransportGain());
    } else if (slider == &tempoSlider) {
        // Pass the current slider position to managerData.setTransportTempo() passing the transport component ('this')
        // as the second argument to exclude the transport from being called again.
        managerData.setTransportTempo(tempoSlider.getValue(), this);
    }
}

//============================================================================
// JUCE ValueTree callbacks (listening to the managerData changes)

void Transport::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& ident) {
    if (ident == MediaManagerData::TRANSPORT_ENABLED) {
        setEnabled(managerData.getTransportEnabled());
        // ^NEED IF ELSE STATEMENT???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
    } else if (ident == MediaManagerData::TRANSPORT_TEMPO_ENABLED) {
        tempoSlider.setEnabled(managerData.getTransportTempoEnabled());
        // ^NEED IF ELSE STATEMENT???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
    } else if (ident == MediaManagerData::TRANSPORT_GAIN) {
        gainSlider.setValue(managerData.getTransportGain(), dontSendNotification);
    } else if (ident == MediaManagerData::TRANSPORT_TEMPO) {
        tempoSlider.setValue(managerData.getTransportTempo(), dontSendNotification);
    } else if (ident == MediaManagerData::TRANSPORT_CLICK_PLAYPAUSE) {
        DBG("TRANSPORT_CLICK_PLAYPAUSE changed");
        playPauseButton.triggerClick();
    } else if (ident == MediaManagerData::TRANSPORT_POSITION) {
        positionSlider.setValue(managerData.getTransportPosition(), dontSendNotification);         // dontSendNotification to not trigger a callback
        drawCurrentTimeLabel();
    } else if (ident == MediaManagerData::TRANSPORT_PLAYBACK_DURATION) {
        drawEndTimeLabel();
    }
}

//============================================================================
// Icon Button Drawing

// Draws the two state play/pause button using a 100x100 coordinate grid.
void Transport::drawPlayButton(juce::DrawableButton& button) {
    juce::DrawablePath triangleDrawable, rectangleDrawable;
    juce::Path trianglePath, rectanglePath;
    
    // Each state is drawn as a path then added to a drawable and assigned to the button using setImages.
    trianglePath.addTriangle(0, 0, 0, 100, 100*sqrt(3)/2, 50);
    triangleDrawable.setPath(trianglePath);
    triangleDrawable.setFill(iconColor);
    
    rectanglePath.addRectangle(0, 0, 42, 100);
    rectanglePath.addRectangle(100-42, 0, 42, 100);
    rectangleDrawable.setPath(rectanglePath);
    rectangleDrawable.setFill(iconColor);

    // Pass the triangle drawbale as the first argument to setImages (its 'normal' argument)
    // and the two-rectangle icon as its fifth argument (the 'normalOn' argument).
    button.setImages(&triangleDrawable, nullptr, nullptr, nullptr, &rectangleDrawable);
}

// Draws the goToStartButton as a left-side rectangle and a left-pointing triangle.
void Transport::drawGoToStartButton(juce::DrawableButton& b) {
    // Juce path drawing done in percentage (100x100)
    juce::DrawablePath image;
    juce::Path path;

    path.addTriangle(100, 0, 100, 100, 13.4, 50);
    path.addRectangle(13.4, 0, 13.4, 100);
    image.setPath(path);
    image.setFill(iconColor);
    b.setImages(&image);
}

void Transport::drawGainButton(juce::DrawableButton& button, double gain) {
  // Juce path drawing done in percentage (100x100)
  juce::DrawablePath drawable;
  juce::Path p;
  
  // speaker rect from 0 to 30
  p.addRectangle(0, 30, 30, 35);
  // speaker cone from 0 to 45
  p.addTriangle(0, 50, 40, 0, 40, 100);
  // waves start at x=55 spaced 15 apart
  if (gain > 0.1)
    p.addCentredArc(55, 50, 6, 20,  0, 0, 3.14159f, true);
  if (gain > 0.4)
    p.addCentredArc(70, 50, 5, 35,  0, 0, 3.14159f, true);
  if (gain > 0.7)
    p.addCentredArc(85, 50, 5, 50,  0, 0, 3.14159f, true);
  // this makes button image width 100 no matter how many arcs added
  p.startNewSubPath(100, 0);
  drawable.setPath(p);
  drawable.setFill(iconColor);
  button.setImages(&drawable);
}

// Updates the currentTimeLabel of the transport.
void Transport::drawCurrentTimeLabel() {
    // Get the floating point playback duration from managerData
    auto dur = managerData.getPlaybackDuration();
    
    // Scale it by the current value of the transport's positionSlider and call toFormattedTimeString() to convert it to a string
    const juce::String secs = toFormattedTimeString(dur * positionSlider.getValue());
    
    // Set currentTimeLabel to show that text
    currentTimeLabel.setText(secs, juce::dontSendNotification);
}

// Updates endTimeLabel of the transport with the playback duration value from the managerData.
void Transport::drawEndTimeLabel() {
    // Similar to drawCurrentTimeLabel()
    const juce::String secs = toFormattedTimeString(managerData.getPlaybackDuration());
    endTimeLabel.setText(secs, juce::dontSendNotification);
}

// Converts a time in floating point seconds in to a string formatted as "mm:ss".
const juce::String Transport::toFormattedTimeString(const double seconds) {
    // Use juce::roundToIntAccurate() to convert the float to integer minutes and seconds return them in the formatted string.
    div_t time = div(juce::roundToIntAccurate(seconds), 60);
    juce::String formattedTime;
    if (time.quot < 10) {
        formattedTime << "0";
    }
    formattedTime << time.quot << ":";
    if (time.rem < 10) {
        formattedTime << "0";
    }
    formattedTime << time.rem;
    
    return formattedTime;
}
