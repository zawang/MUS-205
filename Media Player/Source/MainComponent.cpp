//==============================================================================

#include "MainComponent.h"
#include "MainApplication.h"

//==============================================================================
// Component overrides

MainComponent::MainComponent() : midiOutputMenu(*this) {
    // Set our managerData member variable to the MediaManager's managerData and add this component as a listener of our local copy.
    managerData = MainApplication::getApp().getMediaManager()->getManagerData();
    managerData.addListener(this);
    
    // Add the openButton and make it visible, add this component as its listener.
    addAndMakeVisible(openButton);
    openButton.addListener(this);
    
    // Add the midiOutputMenu and make visible. When nothing is selected is should display "MIDI Outputs". it and add this component as a listener.
    addAndMakeVisible(midiOutputMenu);
    midiOutputMenu.setTextWhenNothingSelected("MIDI Outputs");
    midiOutputMenu.setColour(ComboBox::textColourId, Colours::white);
    midiOutputMenu.addListener(this);
    
    // Add and make visible the infoButton. Call setEnabled(false) to disabled it and add this component as a listener.
    addAndMakeVisible(infoButton);
    infoButton.setEnabled(false);
    infoButton.addListener(this);
    
    // Create the Transport giving it our local managerData and make it visible. The transport's initial state should disabled and our component should be its listener.
    transport = std::make_unique<Transport>(managerData);
    transport->setEnabled(false);
    addAndMakeVisible(transport.get());
    // TODO: ADD THIS COMPONENT AS A LISTENER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

MainComponent::~MainComponent() {
    managerData.removeListener(this);
}

void MainComponent::paint (Graphics& gr) {
    gr.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    
    // Handy for debugging
//    gr.setColour(juce::Colours::red);
//    gr.drawRect(openButton.getBounds());
//    gr.drawRect(midiOutputMenu.getBounds());
//    gr.drawRect(infoButton.getBounds());
//    gr.drawRect(transport->getBounds());
}

void MainComponent::resized() {
    // All subcomponents are inset from the component's edges by 8 pixels.
    // The top line of buttons has a height of 24.
    // The widths of the openButton and infoButton buttons are 90.
    // The midiOutputMenu is center-justified with a width of 180.
    // The transport is already sized, center it in the remaining area 8 pixels below the top line.
    auto padding = 8;
    auto topLineHeight = 24;
    auto buttonWidth = 90;
    auto menuWidth = 180;
    
    auto bounds = getLocalBounds().reduced(padding);
    auto area = bounds.removeFromTop(topLineHeight);
    openButton.setBounds(area.removeFromLeft(buttonWidth));
    infoButton.setBounds(area.removeFromRight(buttonWidth));
    midiOutputMenu.setBounds(area.withTrimmedLeft(padding).removeFromLeft(menuWidth));
    bounds.removeFromTop(8);
    transport->setCentrePosition(bounds.getCentreX(), bounds.getCentreY());
}

//==============================================================================
// JUCE Listener overrides

void MainComponent::buttonClicked(Button* button) {
    if (button == &openButton) {
        managerData.launchOpenMediaDialog();
    } else if (button == &infoButton) {
        managerData.launchMediaInfoDialog();
    }
}

void MainComponent::comboBoxChanged(ComboBox* menu) {
    if (menu == &midiOutputMenu) {
        // Get the selected menu id from the midiOutputMenu and pass it to managerData.setMidiOutputOpenID() so the manager opens that port.
        auto id = menu->getSelectedId();
        managerData.setMidiOutputOpenID(id);
    }
}

//==============================================================================
// ValueTree::Listener overrides

void MainComponent::valueTreePropertyChanged(juce::ValueTree& tree,
					     const juce::Identifier& ident) {
    auto mediaType = managerData.getLoadedMediaType();
    
    if (ident == MediaManagerData::TRANSPORT_PLAYING) {
        // Use managerData.getTransportPlaying() to see if pausing.
        // If the transport is pausing then enable the openButton, otherwise disable it.
        // If the MEDIA_TYPE is MEDIA_MIDI enable the midiOutputMenu if the transport is pausing otherwise enable it.
        
        auto pausing = !managerData.getTransportPlaying();
        openButton.setEnabled(pausing);
        if (mediaType == MediaManagerData::MEDIA_MIDI) {
            midiOutputMenu.setEnabled(pausing);
        }
    } else if (ident == MediaManagerData::LOADED_MEDIA_TYPE) {
        auto midiOpen = managerData.getMidiOutputOpenID();
        if (mediaType == MediaManagerData::MEDIA_AUDIO) {
            infoButton.setEnabled(true);
            midiOutputMenu.setEnabled(false);
            managerData.setTransportEnabled(true);
            managerData.setTransportTempoEnabled(false);
        } else if (mediaType == MediaManagerData::MEDIA_MIDI) {
            infoButton.setEnabled(true);
            midiOutputMenu.setEnabled(true);
            managerData.setTransportEnabled(midiOpen);
            managerData.setTransportTempoEnabled(midiOpen);
        } else if (mediaType == MediaManagerData::MEDIA_NONE) {
            infoButton.setEnabled(false);
            midiOutputMenu.setEnabled(false);
            managerData.setTransportEnabled(false);
            managerData.setTransportTempoEnabled(false);
        }
    } else if (ident == MediaManagerData::MIDI_OUTPUT_OPEN_ID) {
        auto midiOpen = managerData.getMidiOutputOpenID();
        if (mediaType == MediaManagerData::MEDIA_MIDI) {
            managerData.setTransportEnabled(midiOpen);
            managerData.setTransportTempoEnabled(midiOpen);
        }
    }
}
