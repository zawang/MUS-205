#include "MediaManager.h"
#include "MainApplication.h"

MediaManager::MediaManager() {
    // Initialize the audioDeviceManager
    auto error = audioDeviceManager.initialise(0, 2, nullptr, true);
    // ^INCLUDE A JASSERT THAT error IS NOT EMPTY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    // Create the playbackThread, passing it this component, 100 ticks-per-beat tempo of 60.
    playbackThread = std::make_unique<MidiPlaybackThread>(this, 100, 60.0);
    
    // Start the playbackThread.
    playbackThread->startThread();
    
    // Add this MediaManager as a mangerData listener.
    managerData.addListener(this);
    
    // Register the default formats for the formatManager.
    formatManager.registerBasicFormats();
    
    // Add the audioSourePlayer to the AudioDeviceManager so it'll continuously stream data.
    audioDeviceManager.addAudioCallback(&audioSourcePlayer);
    
    // Make the transportSoure the source for the audioSourcePlayer.
    audioSourcePlayer.setSource(&transportSource);
    
    // We are a timer. Start us running every 50 ms
    startTimer(50);
}

MediaManager::~MediaManager() {
    // We are a timer, stop us!
    stopTimer();
    
    // Call removeListener() to stop this component from listening to managerData.
    managerData.removeListener(this);
    
    // Stop the playbackThread, allow 100ms for it to stop.
    playbackThread->stopThread(100);
    
    // Delete the playbackThread.
    playbackThread = nullptr;
    
    // Call sendAllSoundsOff() to make sure the synth stops playing any notes it already has.
    sendAllSoundsOff();
    
    // Close the midi output port
    closeMidiOutput();
    
    // Set the transportSoure's source to null, set the audioSource's source to null and remove the audioSourcePlayer from the AudioDevice manager.
    transportSource.setSource(nullptr);
    audioSourcePlayer.setSource(nullptr);
    audioDeviceManager.removeAudioCallback(&audioSourcePlayer);
}

///=============================================================================
/// The ManagerData callbacks

void MediaManager::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& ident) {
    auto mediaType = managerData.getLoadedMediaType();
    
    if (ident == MediaManagerData::TRANSPORT_PLAYING) {
//        DBG("TRANSPORT_PLAYING changed");
        if (managerData.getTransportPlaying()) {
//            DBG("play");
            // If the Transport is playing call either playMidi() or playAudio() depending on the value of mediaType.
            if (mediaType == MediaManagerData::MEDIA_MIDI) {
                playMidi();
            } else if (mediaType == MediaManagerData::MEDIA_AUDIO) {
                playAudio();
            }
        } else {
//            DBG("pause");
            // else (the Transport is pausing) call either pauseMidi() or pauseAudio() depending on the value of mediaType.
            if (mediaType == MediaManagerData::MEDIA_MIDI) {
                pauseMidi();
            } else if (mediaType == MediaManagerData::MEDIA_AUDIO) {
                pauseAudio();
            }
        }
    } else if (ident == MediaManagerData::TRANSPORT_GAIN) {
        if (mediaType == MediaManagerData::MEDIA_MIDI) {
            setMidiGain(managerData.getTransportGain());
        } else if (mediaType == MediaManagerData::MEDIA_AUDIO) {
            setAudioGain(managerData.getTransportGain());
        }
    } else if (ident == MediaManagerData::TRANSPORT_TEMPO) {
//        DBG("TRANSPORT_TEMPO changed");
        if (mediaType == MediaManagerData::MEDIA_MIDI) {
            setMidiTempo(managerData.getTransportTempo());
        } else if (mediaType == MediaManagerData::MEDIA_AUDIO) {
            setAudioTempo(managerData.getTransportTempo());
        }
    } else if (ident == MediaManagerData::TRANSPORT_REWIND) {
//        DBG("TRANSPORT_REWIND changed");
        if (managerData.getTransportPlaying()) {
             // If the Transport is playing call managerData.clickPlayPause().
            managerData.clickPlayPause();
        }
        // Call either rewindMidi() or rewindAudio() depending on the value of mediaType.
        if (mediaType == MediaManagerData::MEDIA_MIDI) {
            rewindMidi();
        } else if (mediaType == MediaManagerData::MEDIA_AUDIO) {
            rewindAudio();
        }
    } else if (ident == MediaManagerData::TRANSPORT_POSITION) {
        // Get the current Transport playback position.
        auto pos = managerData.getTransportPosition();
        
        if (mediaType == MediaManagerData::MEDIA_MIDI) {
            setMidiPlaybackPosition(pos);
        } else if (mediaType == MediaManagerData::MEDIA_AUDIO) {
            setAudioPlaybackPosition(pos);
        }
    } else if (ident == MediaManagerData::MIDI_OUTPUT_OPEN_ID) {
        // Call managerData.getMidiOutputOpenID() and if its zero call closeMidiOutput()
        // otherwise all openMidiOutput() and be sure to subtract 1 from it
        // the open port id so it becomes a valid device index.
        auto devId = managerData.getMidiOutputOpenID();
        if (devId == 0) {
            closeMidiOutput();
        } else {
            openMidiOutput(devId - 1);
        }
    } else if (ident == MediaManagerData::LAUNCH_OPEN_MEDIA_DIALOG) {
        openMediaFile();
    } else if (ident == MediaManagerData::LAUNCH_MEDIA_INFO_DIALOG) {
        openMediaInfoDialog();
    }
}

///=============================================================================
// PlaybackPositionTimer callback

// Timer callback for scrolling the transport's position.
void MediaManager::timerCallback() {
    auto mediaType = managerData.getLoadedMediaType();
    if (mediaType == MediaManagerData::MEDIA_MIDI) {
        scrollMidiPlaybackPosition();
    } else if (mediaType == MediaManagerData::MEDIA_AUDIO) {
        scrollAudioPlaybackPosition();
    }
}

//==============================================================================
// Generic Media Support
//==============================================================================

void MediaManager::openMediaFile() {
    // Pass (wildcard) matches of all the supported midi and audio file extensions to a juce FileChooser.
    // Midi wild types are "*.mid;*.midi" and audio file types are given by formatManager.getWildardForAllFormats().
    String midiFileTypes {"*.mid;*.midi"};
    String audioFileTypes = formatManager.getWildcardForAllFormats();
    String allowableFileTypes = midiFileTypes + ";" + audioFileTypes;
    
    // Call browseForFileToOpen() and if it returns true then call loadMidiFile() if the file is a midi file else call loadAudioFile().
    FileChooser fc ("Open Media File", File(), allowableFileTypes);
    if (fc.browseForFileToOpen()) {
        auto file = fc.getResult();
        if (midiFileTypes.contains(file.getFileExtension())) {
            loadMidiFile(file);
        } else {
            loadAudioFile(file);
        }
    }
}

// Opens a dialog showing information about the loaded midi or audio files.
void MediaManager::openMediaInfoDialog() {
    // Call getMediaInfo() to get information about the loaded media file and if the string is empty return.
    String info = getMediaInfo();
    if (info == "") {
        return;
    }
    
    // Otherwise do the following actions:
    // Create a TextEditor to hold the info. Configure the editor to be multiline, readonly, sized 400x200. Then give it the text string.
    auto textEditor = std::make_unique<TextEditor>("InfoDialog");
    textEditor->setMultiLine(true);
    textEditor->setReadOnly(true);
    textEditor->setSize(400, 200);
    textEditor->setText(info);
    
    // Allocate a juce::DialogWindow::LaunchOptions struct and fill it.
    // The dialog's titlebar should be native, it should be resizable, the title should be "Media Info" and the background color our application's backgroundColor.
    DialogWindow::LaunchOptions opt;
    opt.dialogTitle = "Media Info";
    opt.useNativeTitleBar = true;
    opt.resizable = true;
    opt.content.setOwned(textEditor.release());
    opt.launchAsync();
}

// Returns information about the current media file.
const String MediaManager::getMediaInfo() {
    auto mt = managerData.getLoadedMediaType();
    // If the managerData's loaded media type is MEDIA_MIDI then return getMidiInfo()
    // else if its MEDIA_AUDIO return getAudioInfo() else return a null string.
    if (mt == MediaManagerData::MEDIA_MIDI) {
        return getMidiInfo();
    } else if (mt == MediaManagerData::MEDIA_AUDIO) {
        return getAudioInfo();
    }
    return "";
}

//==============================================================================
// Audio playback support

void MediaManager::openAudioSettings() {
}

// Returns information about the current audio file.
const String MediaManager::getAudioInfo() {
    auto file = managerData.getLoadedMediaFile();
    auto* reader = formatManager.createReaderFor(file);
    
    String pathName = "Audio file: " + file.getFullPathName() + "\n";
    String fileSize = "File size: " + file.descriptionOfSizeInBytes(file.getSize()) + "\n";
    String audioFormat = "Audio format: " + file.getFileExtension().substring(1).toUpperCase() + " file\n";
    String numChannels = "Channels: " + String(reader->numChannels) + "\n";
    String sampleRate = "Sample rate: " + String(reader->sampleRate) + "\n";
    String sampleFrames = "Sample frames: " + String(reader->lengthInSamples) + "\n";
    String bps = "Bits per sample: " + String(reader->bitsPerSample) + "\n";
    String fpd = reader->usesFloatingPointData == true ? "yes" : "no";
    fpd = "Floating point data: " + fpd + "\n";
    
    return pathName + fileSize + audioFormat + numChannels + sampleRate + sampleFrames + bps + fpd;
}

void MediaManager::loadAudioFile(File audioFile) {
    if (auto* reader = formatManager.createReaderFor(audioFile)) {
        loadIntoTransport(reader);
    }
    else {
        // else our format manager couldn't handle this file. open an AlertWindow telling
        // them that the audio file was an unsupported format and then zero out the audioFile.
        audioFile = File();         // empty file
    }
    // Call managerData.setLoadedMediaFile() and pass it the valid file
    // if it was loaded into the transport otherwise an empty file. e.g. File().
    managerData.setLoadedMediaFile(audioFile);
    // Call managerData.setLoadedMediaType() and pass it MEDIA_AUDIO if the
    // file was loaded into the transport, otherwise MEDIA_NONE.
    auto type = audioFile == File() ? MediaManagerData::MEDIA_NONE
    : MediaManagerData::MEDIA_AUDIO;
    managerData.setLoadedMediaType(type);
}

// Loads an AudioFormatReader into the transport and prepares it for playing.
void MediaManager::loadIntoTransport(AudioFormatReader* reader) {
    // Call the managerData method to stop the transport playing.
    managerData.setTransportPlaying(false);
    
    // Call the managerData method to set the transport's position to 0.
    managerData.setTransportPosition(0.0);
    
    // Call clearAudioPlaybackState() to zero out the MediaManager's playback data.
    clearAudioPlaybackState();
    
    // Create a new AudioFormatReaderSource for the reader.
    audioFileReaderSource.reset(new AudioFormatReaderSource(reader, true));
    
    // Pass that reader source to the transportSource using its setSource() method
    // (you will also need to pass it the reader's sampleRate value.)
    transportSource.setSource(audioFileReaderSource.get(), 0, 0, reader->sampleRate);
    
    // Call transportSource.getLengthInSeconds() and pass that value to the managerData method
    // that sets the total duration of playback.
    managerData.setPlaybackDuration(transportSource.getLengthInSeconds());
}

// Clears out any audio playback state.
void MediaManager::clearAudioPlaybackState() {
    transportSource.setSource(nullptr);
    
    audioFileReaderSource = nullptr;
}

//==============================================================================
// MIDI transport callbacks

void MediaManager::playMidi() {
    // If the playback thread's position is at the end then set it back to position 0.0.
    if (playbackThread->isPlaybackAtEnd()) {
        playbackThread->setPlaybackPosition(0.0, 0);
    }
    // Set the playback thread's tempo to managerData's tempo.
    playbackThread->setTempo(managerData.getTransportTempo());
    // Set the playback thread's gain to managerData's gain.
    playbackThread->setGain(managerData.getTransportGain());
    // Start the playback thread playing (call setPaused(false)).
    playbackThread->setPaused(false);
}

void MediaManager::pauseMidi() {
    playbackThread->setPaused(true);
}

void MediaManager::setMidiGain(double gain) {
    std::cout << "MediaManager::setMidiGain(" << gain << ")\n";
    playbackThread->setGain(gain);
}

void MediaManager::setMidiTempo(double tempo) {
//    DBG("setMidiTempo");
    playbackThread->setTempo(tempo);
}

void MediaManager::rewindMidi() {
    playbackThread->setPlaybackPosition(0.0, 0);
}

// Sets the playback position in the midi thread.
void MediaManager::setMidiPlaybackPosition(double position) {
//  std::cout << "MediaManager::setMidiPlaybackPosition("<< position <<")\n";
    
    // Convert the given transport position (a value between 0.0 and 1.0) into a "beat" position (in seconds) using midiFileDuration as the total duration of the playback.
    auto beat = position * midiFileDuration;
    
    // Determine the index position of the beat position in the sequence using MidiMessageSequence::getNextIndexAtTime().
    auto index = sequence.getNextIndexAtTime(beat);
    
    // If the playback thread is currently running, pause it.
    auto playing = playbackThread->isPlaying();
    if (playing) playbackThread->setPaused(true);
    
    // Clear the midi thread.
    playbackThread->clear();
    
    // Set the midi thread's playback position to the beat and index values.
    playbackThread->setPlaybackPosition(beat, index);
    
    // If the thread was running, start it running again.
    if (playing) playbackThread->setPaused(false);
}

// Scrolls the transport's position slider during playback without triggering a TRANSPORT_POSITION callback back to this manager.
void MediaManager::scrollMidiPlaybackPosition() {
//  std::cout << "MediaManager::scrollMidiPlaybackPosition()\n";
    
    // If the transport is currently paused, return.
    auto playing = managerData.getTransportPlaying();
    if (!playing) {
        return;
    }
    
    double sec = playbackThread->getPlaybackBeat();
    double dur = midiFileDuration;
    double pos = sec/dur;
    managerData.setTransportPosition(pos, this);
    
    if (pos >= 1.0) {
        managerData.clickPlayPause();
    }
}

/// Audio transport callbacks

void MediaManager::playAudio() {
//  std::cout << "MediaManager::playAudio()\n";
  // If the transportSource has finished playing set its position to 0.0
  if (transportSource.hasStreamFinished()) {
    transportSource.setPosition(0.0);
  }
  // Set the transportSource's gain to the managerData's gain.
  transportSource.setGain(managerData.getTransportGain());
  // Start the transport source.
  transportSource.start();
}

void MediaManager::pauseAudio() {
//  std::cout << "MediaManager::pauseAudio()\n";
  // Stop the transportSource.
  transportSource.stop();
}

void MediaManager::setAudioGain(double gain) {
  std::cout << "MediaManager::setAudioGain(" << gain << ")\n";
  // Set the transportSource's gain to the managerData's gain.
  transportSource.setGain(managerData.getTransportGain());
}

void MediaManager::setAudioTempo(double tempo) {
  std::cout << "MediaManager::setAudioTempo(" << tempo << ")\n";
  // nothing to do!
}

void MediaManager::rewindAudio() {
  std::cout << "MediaManager::rewindAudio()\n";
  // set the transportSource's position back to 0.0.
  transportSource.setPosition(0.0);
}

/// Sets the audio transport player's position.
void MediaManager::setAudioPlaybackPosition(double pos) {
  auto playing = managerData.getTransportPlaying();
  std::cout << "media manager: receiving position:" << pos
  << ", playing:" << managerData.getTransportPlaying() << "\n" ;
  if (pos == 0) {
    // setSource(0) stops popping on rewind and play
    transportSource.setSource(0);
    transportSource.setSource(audioFileReaderSource.get());
    if (playing) transportSource.start();
  }
  else {
    // std::cout << "transport position=" << position << "\n";
    if (playing) transportSource.stop(); // not sure why this is necessary!
    transportSource.setPosition(pos * transportSource.getLengthInSeconds());
    if (playing) transportSource.start();
  }
}

// Scrolls the transport's position slider during playback.
void MediaManager::scrollAudioPlaybackPosition() {
    // If the transport is currently paused, return.
    auto playing = managerData.getTransportPlaying();
    if (!playing) {
        return;
    }
    
    double sec = transportSource.getCurrentPosition();
    double dur = transportSource.getLengthInSeconds();
    double pos = sec/dur;
    //  std::cout << "pbPos="<<sec<<", pbDur="<<dur<<", pbrat="<<pos<<"\n";
    managerData.setTransportPosition(pos, this);
    
    if (pos >= 1.0) {
        DBG("pos >= 1.0");
        managerData.clickPlayPause();
    }
}

///==============================================================================

void MediaManager::openMidiOutput(int dev) {
  // Call MidiOutput::openDevice and reset the midiOutputDevice to it.
  midiOutputDevice = MidiOutput::openDevice(dev);
  jassert(midiOutputDevice != nullptr);
}

void MediaManager::closeMidiOutput() {
  // Set the midiOutputDevice to nullptr.
  midiOutputDevice.reset(nullptr);
}

bool MediaManager::isInternalSynthAvailable() {
  return false;
}


///==============================================================================
/// MidiFile Functions

// Returns information about the current midi file.
const String MediaManager::getMidiInfo() {
    auto file = managerData.getLoadedMediaFile();
    
    String pathName = "Midi file: " + file.getFullPathName() + "\n";
    String fileSize = "File size: " + file.descriptionOfSizeInBytes(file.getSize()) + "\n";
    String fileFormat = midiFileNumTracks > 1 ? String(1) : String(0);
    fileFormat = "MIDI file format: level " + fileFormat + "\n";
    String numTracks = "Number of tracks: " + String(midiFileNumTracks) + "\n";
    String duration = "Duration: " + String(midiFileDuration) + "\n";
    String numMessages = "Number of Messages: " + String(midiFileLength) + "\n";
    
    return pathName + fileSize + fileFormat + numTracks + duration + numMessages;
}

// Clears the midi playback state.
void MediaManager::clearMidiPlaybackState() {
    midiFileDuration = 0;
    midiFileLength = 0;
    midiFileNumTracks = 0;
    midiFileTimeFormat = 0;
    sequence.clear();
    playbackThread->setPlaybackPosition(0.0, 0);
}

void MediaManager::loadMidiFile(File midiFile) {
    FileInputStream input (midiFile);
    MidiFile mf;
    if (input.openedOk() && mf.readFrom(input) && mf.getTimeFormat() > 0) {
        loadIntoPlayer(mf);
    } else {
        // In the case of failure, we put an empty file marker in our database (I think?)
        midiFile = File();
    }
    
    // Update our database
    managerData.setLoadedMediaFile(midiFile);
    managerData.setLoadedMediaType(midiFile == File() ? MediaManagerData::MEDIA_NONE : MediaManagerData::MEDIA_MIDI);
}

void MediaManager::loadIntoPlayer(MidiFile& midifile) {
//    DBG("in loadIntoPlayer");
    managerData.setTransportPlaying(false);
    managerData.setTransportPosition(0.0);
    clearMidiPlaybackState();
    midiFileNumTracks = midifile.getNumTracks();
    midiFileTimeFormat = midifile.getTimeFormat();
    midifile.convertTimestampTicksToSeconds();
    for (auto track = 0; track < midiFileNumTracks; ++track) {
        auto* seq = midifile.getTrack(track);   // pointer to the current track
        // Add track to our playback sequence
        sequence.addSequence(*seq, 0.0, 0.0, seq->getEndTime() + 1);
        sequence.updateMatchedPairs();
    }
    
    midiFileDuration = sequence.getEndTime();
    midiFileLength = sequence.getNumEvents();

    playbackThread->setPlaybackLimit(midiFileDuration, midiFileLength);
    
    managerData.setPlaybackDuration(midiFileDuration);
}

///==============================================================================
/// MidiMessage Functions

void MediaManager::sendMessage(const MidiMessage& message) {
  ScopedLock sl (sendLock);
  if (midiOutputDevice) {
    midiOutputDevice->sendMessageNow(message);
  }
  else if (isInternalSynthAvailable()) {
    playInternalSynth(message);
  }
}

void MediaManager::playInternalSynth(const MidiMessage& message) {
}

void MediaManager::sendAllSoundsOff() {
  std::cout << "Sending allSoundsOff, output port is "
  << (midiOutputDevice.get() ? "OPEN" : "CLOSED") << "\n";
  for (int i = 1; i <= 16; i++) {
    juce::MidiMessage msg = juce::MidiMessage::allSoundOff(i);
    sendMessage(msg);
  }
}

//==============================================================================
// MidiPlaybackClient callbacks

void MediaManager::handleMessage (const MidiMessage &message) {
    sendMessage(message);
}

void MediaManager::addMidiPlaybackMessages(MidiPlaybackThread::MidiMessageQueue& queue,
                                           MidiPlaybackThread::PlaybackPosition& position) {
  int index = position.index;
  for (; index < position.length; index++) {
    juce::MidiMessageSequence::MidiEventHolder* ev = sequence.getEventPointer(index);
    // skip over non-channel messages
    if (ev->message.getChannel() < 1)
      continue;
    // skip over noteOffs because we add by pairs with noteOns
    if (ev->message.isNoteOff())
      continue;
    // add every message that is at or earlier (e.g. late) than the current time
    if (ev->message.getTimeStamp() <= position.beat) {
      queue.addMessage(new juce::MidiMessage(ev->message));
      if (ev->noteOffObject) {
        queue.addMessage(new juce::MidiMessage(ev->noteOffObject->message));
      }
    }
    else
      break;
  }
  // index is now the index of the next (future) event or length
  position.index = index;
  if (position.isAtEnd())
    std::cout << "Midi playback at end!\n";
}
