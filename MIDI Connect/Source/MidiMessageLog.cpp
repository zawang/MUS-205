//==============================================================================
#include "MidiMessageLog.h"

MidiMessageLog::MidiMessageLog() {
    // Since our class is both a ListBox and ListBoxModel it needs to pass itself as the ListBoxModel to 'setModel()'.
    setModel(this);
    
    setColour(MidiMessageLog::backgroundColourId, Colours::black);
    setColour(MidiMessageLog::outlineColourId, Colours::black);
}

MidiMessageLog::~MidiMessageLog() {
}

//==============================================================================
/// ListBoxModel overrides.

int MidiMessageLog::getNumRows() {  
    return messageLog.size();
}

void MidiMessageLog::paintListBoxItem (int row, Graphics& g, int width, int height, bool)  {
    if (isPositiveAndBelow(row, messageLog.size())) {
        g.setColour(Colours::white);
        g.setFont(15);
        g.drawText(messageLog[row], Rectangle<int>(width, height).reduced(4, 0), Justification::centredLeft, true);
    }
    
//    g.setColour(Colours::white);
//    g.setFont(15);
//    g.drawText(messageLog[row], Rectangle<int>(width, height).reduced(4, 0), Justification::centredLeft, true);
}

//==============================================================================
/// AsyncUpdater override.

void MidiMessageLog::handleAsyncUpdate() {
    updateContent();
    scrollToEnsureRowIsOnscreen(getNumRows() - 1);
    repaint();
}

//==============================================================================
// MidiMessageLog methods.

void MidiMessageLog::addMidiMessage (const MidiMessage& message) {
    messageLog.add(midiMessageString(message));
    triggerAsyncUpdate();
}

void MidiMessageLog::clear() {
    messageLog.clear();
    triggerAsyncUpdate();
}

const String MidiMessageLog::midiMessageString(const MidiMessage& msg) {
    String timestamp = "[" + String(msg.getTimeStamp(), 2) + "] ";
    
    // If the message isn't a channel message (i.e. its channel == 0)
    if (msg.getChannel() == 0) {
        return timestamp + msg.getDescription();
    }
    
    // The message is a channel message
    String chanNum = "channel=" + String(msg.getChannel()) + ", ";
    String midiType = "";
    String specifics = "";              // String with information specific to the type of midi message
    if (msg.isNoteOn()) {
        midiType = "NoteOn: ";
        int keyNum = msg.getNoteNumber();
        if (msg.getChannel() == 10) {
            specifics = String(msg.getRhythmInstrumentName(keyNum)) + ", velocity=" + String(msg.getVelocity());
        } else {
            specifics = "key=" + String(keyNum) + ", velocity=" + String(msg.getVelocity()) + ", pitch=" + keynumToPitch(keyNum) + ", freq=" + String(keynumToFrequency(keyNum));
        }
    } else if (msg.isNoteOff()) {
        midiType = "NoteOff: ";
        int keyNum = msg.getNoteNumber();
        if (msg.getChannel() == 10) {
            specifics = String(msg.getRhythmInstrumentName(keyNum)) + ", velocity=" + String(msg.getVelocity());
        } else {
            specifics = "key=" + String(keyNum) + ", velocity=" + String(msg.getVelocity());
        }
    } else if (msg.isController()) {
        midiType = "ControlChange: ";
        specifics = "controller=" + String(msg.getControllerName(msg.getControllerNumber())) + ", value=" + String(msg.getControllerValue());
    } else if (msg.isProgramChange()) {
        midiType = "ProgramChange: ";
        specifics = "number=" + String(msg.getProgramChangeNumber());
    } else if (msg.isPitchWheel()) {
        midiType = "PitchWheel: ";
        specifics = "value=" + String(msg.getPitchWheelValue());
    } else if (msg.isAftertouch()) {
        midiType = "AfterTouch: ";
        specifics = "value=" + String(msg.getAfterTouchValue());
    } else if (msg.isChannelPressure()) {
        midiType = "ChannelPressure: ";
        specifics = "value=" + String(msg.getChannelPressureValue());
    }
    
    return timestamp + midiType + chanNum + specifics;
}

float MidiMessageLog::keynumToFrequency(int keynum) {
    return 440.0 * pow(2.0, (keynum-69.0) / 12.0);
}

String MidiMessageLog::keynumToPitch(int keynum) {
    StringArray notes {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};
    StringArray octaves {"00", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    
    return notes[keynum % 12] + octaves[(int) (keynum / 12)];
}

float MidiMessageLog::velocityToAmplitude(int velocity) {
    return 0.0;
}
