//==============================================================================

#include "MainComponent.h"
#include "MainApplication.h"

MainContentComponent::MainContentComponent()
: audioManager (MainApplication::getApp().getAudioDeviceManager()) {
    addAndMakeVisible(messageLogButton);
    addAndMakeVisible(pianoRollButton);
    addAndMakeVisible(clearButton);
    addAndMakeVisible(settingsButton);
    addAndMakeVisible(midiMessageLog);
    
    messageLogButton.addListener(this);
    pianoRollButton.addListener(this);
    clearButton.addListener(this);
    settingsButton.addListener(this);
    
    messageLogButton.setConnectedEdges(Button::ConnectedOnRight);
    pianoRollButton.setConnectedEdges(Button::ConnectedOnLeft);
    
    // Enabled these buttons to be in the same group of 'radio buttons' so that only one button can be 'on' at any given moment
    messageLogButton.setRadioGroupId(1);
    pianoRollButton.setRadioGroupId(1);
    
    messageLogButton.setToggleState(true, dontSendNotification);
    
    // Add our component as the keyboard state's listener.
    keyboardState.addListener(this);
    
    // Pass the keyboard state to the keyboard component.
    midiKeyboard = std::make_unique<MidiKeyboardComponent>(keyboardState, MidiKeyboardComponent::horizontalKeyboard);
    midiKeyboard->setOctaveForMiddleC(4);
    addAndMakeVisible(*midiKeyboard);
    
    midiPianoRoll.setVisible(true);
    
    audioManager.addMidiInputDeviceCallback("", this);
    
    // Create a new sfzero::SFZeroAudioProcessor (synthesizer) and assign it to the sfZeroAudioProcessor unique_ptr.
    sfZeroAudioProcessor.reset(new sfzero::SFZeroAudioProcessor());
    // Add the sfZeroAudioProcessor to the sfzero player for playback.
    sfZeroPlayer.setProcessor(sfZeroAudioProcessor.get());
    // Add the sfZeroPlayer as the audio callback for our audio device manager.
    audioManager.addAudioCallback(&sfZeroPlayer);
    
    auto soundFont = MainApplication::getApp().getRuntimeResourceDirectory().getChildFile("G800-A112-Piano1d-2-3f.sfz");
    loadSoundFont(soundFont);
}

MainContentComponent::~MainContentComponent() {
    keyboardState.removeListener(this);
    
    audioManager.removeMidiInputDeviceCallback("", this);
    
    sfZeroPlayer.setProcessor(nullptr);
}

//==============================================================================
// Component overrides

void MainContentComponent::paint (Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MainContentComponent::resized() {
    auto padding = 8;
    auto topLineHeight = 24;
    auto settingsButtonWidth = 120;
    auto clearButtonWidth = 60;
    auto messagePianoButtonWidth = 90;
    auto midiKeyboardHeight = 64;
    auto area = getLocalBounds().reduced(padding, padding);
    
    settingsButton.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(topLineHeight).removeFromLeft(settingsButtonWidth));
    clearButton.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(topLineHeight).removeFromRight(clearButtonWidth));
    messageLogButton.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(topLineHeight).removeFromLeft(area.getCentreX()-padding).removeFromRight(messagePianoButtonWidth));
    pianoRollButton.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(topLineHeight).removeFromRight(area.getCentreX()-padding).removeFromLeft(messagePianoButtonWidth));
    midiKeyboard->setBounds(getLocalBounds().reduced(padding, 2*padding+topLineHeight).removeFromTop(midiKeyboardHeight));
    midiMessageLog.setBounds(getLocalBounds().reduced(padding, padding).withTrimmedTop(midiKeyboard->getBottom()));
    midiPianoRoll.setBounds(getLocalBounds().reduced(padding, padding).withTrimmedTop(midiKeyboard->getBottom()));
}

//==============================================================================
// Listener overrides

void MainContentComponent::buttonClicked(Button* button ) {
    /// * If the pianoRollButton was clicked (and it was not already toggled on) then
    /// take analogous steps as outlined for the messageLogButton.
    
    if (button == &settingsButton) {
        openAudioSettings();
    } else if (button == &clearButton) {
        // Clear the message log or the piano roll, whichever is currently active
        if (messageLogButton.getToggleState()) {
            midiMessageLog.clear();
        } else {
            midiPianoRoll.clear();
        }
    } else if (button == &messageLogButton) {
        if (!messageLogButton.getToggleState()) {
            messageLogButton.setToggleState(true, dontSendNotification);
            midiPianoRoll.clear();
            removeChildComponent(&midiPianoRoll);
            keyboardState.reset();
            addAndMakeVisible(midiMessageLog);
        }
    } else if (button == &pianoRollButton) {
        if (!pianoRollButton.getToggleState()) {
            pianoRollButton.setToggleState(true, dontSendNotification);
            midiMessageLog.clear();
            removeChildComponent(&midiMessageLog);
            keyboardState.reset();
            addAndMakeVisible(midiPianoRoll);
        }
    }
}

void MainContentComponent::showMidiMessage(const MidiMessage& message) {
    if (messageLogButton.getToggleState()) {
        midiMessageLog.addMidiMessage(message);
    } else {
        midiPianoRoll.addMidiMessage(message);
    }
}

void MainContentComponent::playMidiMessage(const MidiMessage& message) {
    sfZeroPlayer.getMidiMessageCollector().addMessageToQueue(message);
}

void MainContentComponent::handleNoteOn(MidiKeyboardState*, int chan, int note, float vel) {
    MidiMessage message = MidiMessage::noteOn(chan, note, vel);
    message.setTimeStamp(Time::getMillisecondCounterHiRes() / 1000.0);
    playMidiMessage(message);
    showMidiMessage(message);
}

void MainContentComponent::handleNoteOff(MidiKeyboardState*, int chan, int note, float vel) {
    MidiMessage message = MidiMessage::noteOff(chan, note, vel);
    message.setTimeStamp(Time::getMillisecondCounterHiRes() / 1000.0);
    playMidiMessage(message);
    showMidiMessage(message);
}

//==============================================================================
// MidiCallback overrides

# if 0
// Method 1: Block all other message thread activity during the update
void MainContentComponent::handleIncomingMidiMessage(MidiInput* source, const MidiMessage &message) {
    if (quitting || message.isActiveSense()) {
        return;
    }

    // Block the message thread during update
    const MessageManagerLock mmLock;
    if (message.isNoteOnOrOff()) {
        keyboardState.processNextMidiEvent(message);
    } else {
        showMidiMessage(message);
        playMidiMessage(message);
    }
}
#endif

# if 1
// Method 2: Evaluate the lambda function in the context of the message thread so there's no need to block message thread.
void MainContentComponent::handleIncomingMidiMessage(MidiInput* source, const MidiMessage &message) {
    if (quitting || message.isActiveSense()) {
        return;
    }
    
    // safePointer will automatically become null if the MainContentComponent is deleted (such as when the app quits)
    auto safePointer = Component::SafePointer<MainContentComponent>(this);
    auto lambda = [message, safePointer]() {
        // Check to make sure safePointer is not null (i.e. the MainContentComponent still exists) before proceeding
        if (safePointer) {
            if (message.isNoteOnOrOff()) {
                safePointer.getComponent()->keyboardState.processNextMidiEvent(message);
            } else {
                safePointer.getComponent()->showMidiMessage(message);
                safePointer.getComponent()->playMidiMessage(message);
            }
        }
    };
    juce::MessageManager::callAsync(lambda);
}
#endif

//==============================================================================
// Internal Synth code
//

void MainContentComponent::loadSoundFont(juce::File& fontFile) {
    // The sound font you are attempting to load cannot be found!
    jassert(fontFile.existsAsFile());
    
    // Load soundfont in its own thread
    sfZeroAudioProcessor->setSfzFileThreaded(&fontFile);
    std::cout << "*** Soundfont loaded from '" << fontFile.getFullPathName() << "'\n";
}

void MainContentComponent::openAudioSettings() {
    auto devcomp = std::make_unique<AudioDeviceSelectorComponent>(audioManager, 0, 2, 0, 2, true, false, true, false);
    devcomp->setBounds(0, 0, 500, 500);
    
    DialogWindow::LaunchOptions opt;
    opt.dialogTitle = "Audio Settings";
    opt.dialogBackgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    opt.content.setOwned(devcomp.release());
    opt.launchAsync();
}
