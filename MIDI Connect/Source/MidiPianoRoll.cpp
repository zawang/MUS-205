//==============================================================================

#include "MidiPianoRoll.h"

//==============================================================================
// PianoRollNote

PianoRollNote::PianoRollNote(int key, int vel) {
    keynum = key;
    velocity = vel;
}

PianoRollNote::~PianoRollNote() {
}

void PianoRollNote::paint(Graphics& gr) {
    gr.fillAll(Colours::red);
}

//==============================================================================
// MidiPianoRoll

MidiPianoRoll::MidiPianoRoll() {
    setFramesPerSecond(FPS);
}

MidiPianoRoll::~MidiPianoRoll() {
    clear();
}

void MidiPianoRoll::clear() {
    notes.clear();
}

void MidiPianoRoll::paint(Graphics& gr) {
    gr.fillAll(Colours::black);
}

void MidiPianoRoll::update() {
    for (auto it = notes.begin(); it != notes.end();) {
        std::unique_ptr<PianoRollNote> &note = *it;
        
        if (note->haveNoteOff) {
            // If the note has received its note off, increment its X position
            note->setTopLeftPosition(note->getX() + PPF, note->getY());
        } else {
            // If the note has not received its note off, increment its width
            note->setSize(note->getWidth() + PPF, note->getHeight());
        }
        
        // If the note's X position is beyond the right side of the piano roll, delete it from notes
        if (note->getX() > getWidth()) {
            it = notes.erase(it);
        } else {
            ++it;
        }
    }
}

void MidiPianoRoll::addMidiMessage (const MidiMessage& msg) {
    if (msg.isNoteOn()) {
        notes.push_back(std::make_unique<PianoRollNote>(msg.getNoteNumber(), msg.getVelocity()));
        auto &note = notes.back();
        auto pianoRollHeight = getHeight();
        auto noteHeight = pianoRollHeight/128.0;
        note->setBounds(0, (127 - note->keynum) * noteHeight, 0, noteHeight);
        addAndMakeVisible(*note);
    } else if (msg.isNoteOff()) {
        for (auto& note : notes) {
            if (note->keynum == msg.getNoteNumber()) {
                note->haveNoteOff = true;
            }
        }
    }
}
