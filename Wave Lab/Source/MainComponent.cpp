//==============================================================================

#include "MainComponent.h"
#include "MainApplication.h"

MainComponent::MainComponent()
: deviceManager (MainApplication::getApp().audioDeviceManager), audioVisualizer(2) {
    addAndMakeVisible(playButton);
    addAndMakeVisible(settingsButton);
    addAndMakeVisible(waveformMenu);
    addAndMakeVisible(levelLabel);
    addAndMakeVisible(levelSlider);
    addAndMakeVisible(freqLabel);
    addAndMakeVisible(freqSlider);
    addAndMakeVisible(audioVisualizer);
    addAndMakeVisible(cpuUsage);
    addAndMakeVisible(cpuLabel);
    
    playButton.addListener(this);
    settingsButton.addListener(this);
    waveformMenu.addListener(this);
    levelSlider.addListener(this);
    freqSlider.addListener(this);
    
    levelSlider.setRange(0.0, 1.0);
    levelSlider.setSliderStyle(Slider::LinearHorizontal);
    levelSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 90, 22);
    
    freqSlider.setRange(0.0, 5000.0);
    freqSlider.setSkewFactorFromMidPoint(500.0);
    freqSlider.setSliderStyle(Slider::LinearHorizontal);
    freqSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 90, 22);
    
    // Populate waveform menu
    waveformMenu.addItemList(section1, WhiteNoise);
    waveformMenu.addSeparator();
    waveformMenu.addItemList(section2, SineWave);
    waveformMenu.addSeparator();
    waveformMenu.addItemList(section3, LF_ImpulseWave);
    waveformMenu.addSeparator();
    waveformMenu.addItemList(section4, BL_ImpulseWave);
    waveformMenu.addSeparator();
    waveformMenu.addItemList(section5, WT_SineWave);
    waveformMenu.setTextWhenNothingSelected("Waveforms");
    
    levelLabel.setJustificationType(Justification::right);
    freqLabel.setJustificationType(Justification::right);
    cpuLabel.setJustificationType(Justification::centredRight);
    cpuUsage.setJustificationType(Justification::centredRight);
    
    playButton.setEnabled(false);
    drawPlayButton(playButton, true);
    
//    createWaveTables();
    
    audioSourcePlayer.setSource(nullptr);
    
    deviceManager.addAudioCallback(&audioSourcePlayer);
    
    srate = 0.0;
    level = 0.0;
    freq = 0.0;
    
    startTimerHz(15);
}

MainComponent::~MainComponent() {
    audioSourcePlayer.setSource(nullptr);
    deviceManager.removeAudioCallback(&audioSourcePlayer);
    deviceManager.closeAudioDevice();
}

//==============================================================================
// Component overrides
//==============================================================================

void MainComponent::paint (Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    
    g.setColour(Colours::red);
    // Draw red outline around all components for debugging purposes
    g.drawRect(settingsButton.getBounds());
    g.drawRect(playButton.getBounds());
    g.drawRect(levelLabel.getBounds());
    g.drawRect(levelSlider.getBounds());
    g.drawRect(waveformMenu.getBounds());
    g.drawRect(freqLabel.getBounds());
    g.drawRect(freqSlider.getBounds());
    g.drawRect(audioVisualizer.getBounds());
    g.drawRect(cpuUsage.getBounds());
    g.drawRect(cpuLabel.getBounds());
}

void MainComponent::resized() {
    auto padding = 8;
    auto area = getLocalBounds().reduced(padding, padding);
    auto height = 24; // height of all components unless stated otherwise
    auto settingsButtonWidth = 118;
    auto waveformMenuWidth = 118;
    auto playButtonWH = 2*height+padding;
    auto levelLabelWidth = 72;
    auto freqLabelWidth = 72;
    auto cpuUsageWidth = 66;
    auto cpuLabelWidth = 36;
    
    settingsButton.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(height).removeFromLeft(settingsButtonWidth));
    playButton.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(playButtonWH).removeFromRight(area.getWidth()-settingsButton.getRight()).removeFromLeft(playButtonWH));
    levelLabel.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(height).removeFromRight(area.getWidth()-playButton.getRight()).removeFromLeft(levelLabelWidth));
    levelSlider.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(height).removeFromRight(area.getWidth()+padding-levelLabel.getRight()));
    waveformMenu.setBounds(getLocalBounds().reduced(padding, 2*padding+height).removeFromTop(height).removeFromLeft(waveformMenuWidth));
    freqLabel.setBounds(getLocalBounds().reduced(padding, 2*padding+height).removeFromTop(height).removeFromRight(area.getWidth()-playButton.getRight()).removeFromLeft(freqLabelWidth));
    freqSlider.setBounds(getLocalBounds().reduced(padding, 2*padding+height).removeFromTop(height).removeFromRight(area.getWidth()+padding-freqLabel.getRight()));
    audioVisualizer.setBounds(getLocalBounds().withTrimmedBottom(height).withTrimmedLeft(padding).withTrimmedRight(padding).withTrimmedTop(waveformMenu.getBottom()+padding));
    cpuUsage.setBounds(getLocalBounds().withTrimmedRight(padding).removeFromBottom(height).removeFromRight(cpuUsageWidth));
    cpuLabel.setBounds(getLocalBounds().withTrimmedRight(padding+cpuUsageWidth).removeFromBottom(height).removeFromRight(cpuLabelWidth));
}

void MainComponent::drawPlayButton(juce::DrawableButton& button, bool showPlay) {
  juce::Path path;
  if (showPlay) {
    // draw the triangle
      path.addTriangle(0, 0, 0, 100, sqrt(3)/2*100, 50);
  }
  else {
    // draw the two bars
      path.addRectangle(0, 0, 42, 100);
      path.addRectangle(60, 0, 42, 100);
  }
  juce::DrawablePath drawable;
  drawable.setPath(path);
  juce::FillType fill (Colours::white);
  drawable.setFill(fill);
  button.setImages(&drawable);
}

//==============================================================================
// Listener overrides
//==============================================================================

void MainComponent::buttonClicked (Button *button) {
    if (button == &playButton) {
        if (isPlaying()) {
            audioSourcePlayer.setSource(nullptr);
        } else {
            audioSourcePlayer.setSource(this);
        }
        drawPlayButton(playButton, !isPlaying());
        
//        drawPlayButton(playButton, playButton.getToggleState());
//        playButton.setToggleState(!playButton.getToggleState(), dontSendNotification);
    } else if (button == &settingsButton) {
        openAudioSettings();
    }
}

void MainComponent::sliderValueChanged (Slider *slider) {
    if (slider == &levelSlider) {
        level = levelSlider.getValue();
    } else if (slider == &freqSlider) {
        freq = freqSlider.getValue();
        phaseDelta = freq/srate;
        
        for (auto & oscillator : oscillators) {
            oscillator->setFrequency(freq, srate);
        }
    }
}

void MainComponent::comboBoxChanged (ComboBox *menu) {
    if (menu == &waveformMenu) {
        waveformId = (WaveformId) menu->getSelectedId();
        // If the selected waveform is white noise or brown noise, disable the frequency slider.
        if (waveformId == WhiteNoise || waveformId == BrownNoise) {
            freqSlider.setEnabled(false);
        } else {
            freqSlider.setEnabled(true);
        }
    }
    playButton.setEnabled(waveformId > 0);
}

//==============================================================================
// Timer overrides
//==============================================================================

void MainComponent::timerCallback() {
    cpuUsage.setText(String(deviceManager.getCpuUsage() * 100, 2, false) + " %", dontSendNotification);
}

//==============================================================================
// AudioSource overrides
//==============================================================================

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
    srate = sampleRate;
    phase = 0;
    phaseDelta = freq/sampleRate; // in radians
    
    createWaveTables();
    
    for (auto & oscillator : oscillators) {
        oscillator->setFrequency(freq, srate);
    }
    
    audioVisualizer.setBufferSize(samplesPerBlockExpected);
    audioVisualizer.setSamplesPerBlock(8);
}

void MainComponent::releaseResources() {
//    std::cout << "releaseResources\n";
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) {
  bufferToFill.clearActiveBufferRegion();
  switch (waveformId) {
    case WhiteNoise:      whiteNoise(bufferToFill);   break;
    case DustNoise:       dust(bufferToFill);         break;
    case BrownNoise:      brownNoise(bufferToFill);   break;
    case SineWave:        sineWave(bufferToFill);     break;
    case LF_ImpulseWave:  LF_impulseWave(bufferToFill);  break;
    case LF_SquareWave:   LF_squareWave(bufferToFill);   break;
    case LF_SawtoothWave: LF_sawtoothWave(bufferToFill); break;
    case LF_TriangeWave:  LF_triangleWave(bufferToFill); break;
    case BL_ImpulseWave:  BL_impulseWave(bufferToFill);  break;
    case BL_SquareWave:   BL_squareWave(bufferToFill);   break;
    case BL_SawtoothWave: BL_sawtoothWave(bufferToFill); break;
    case BL_TriangeWave:  BL_triangleWave(bufferToFill); break;
    case WT_SineWave:
    case WT_ImpulseWave:
    case WT_SquareWave:
    case WT_SawtoothWave:
    case WT_TriangleWave:
      WT_wave(bufferToFill);
      break;
    case Empty:
      break;
  }
  audioVisualizer.pushBuffer(bufferToFill);
}

//==============================================================================
// Audio Utilities
//==============================================================================

double MainComponent::phasor() {
    return std::fmod(phase+phaseDelta, 1.0);
}

float MainComponent::ranSamp() {
  return 2*random.nextFloat() - 1;
}

float MainComponent::ranSamp(const float mul) {
  return (ranSamp() * mul);
}

float MainComponent::lowPass(const float value, const float prevout, const float alpha) {
    return prevout + alpha * (value - prevout);
}

bool MainComponent::isPlaying() {
    if (audioSourcePlayer.getCurrentSource()) {
        return true;
    }
    return false;
}

void MainComponent::openAudioSettings() {
    auto devcomp = std::make_unique<AudioDeviceSelectorComponent>(deviceManager, 0, 2, 0, 2, true, false, true, false);
    devcomp->setBounds(0, 0, 500, 500);
    
    DialogWindow::LaunchOptions opt;
    opt.dialogTitle = "Audio Settings";
    opt.dialogBackgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    opt.content.setOwned(devcomp.release());
    opt.launchAsync();
}

void MainComponent::createWaveTables() {
  createSineTable(sineTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(sineTable));
  createImpulseTable(impulseTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(impulseTable));
  createSquareTable(squareTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(squareTable));
  createSawtoothTable(sawtoothTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(sawtoothTable));
  createTriangleTable(triangleTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(triangleTable));
}

//==============================================================================
// Noise
//==============================================================================

// White Noise

void MainComponent::whiteNoise (const AudioSourceChannelInfo& bufferToFill) {
    // process every channel of data
    for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        // get the pointer to the first sample in the channel to process
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        // iterate the pointer over all the samples in the channel data
        for (int i = 0; i < bufferToFill.numSamples ; ++i)
        {
            channelData[i] = ranSamp(level); // assign a value to the current sample
        }
    }
}

// Dust

void MainComponent::dust (const AudioSourceChannelInfo& bufferToFill) {
    for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (int i = 0; i < bufferToFill.numSamples ; ++i)
        {
            if (random.nextFloat() <= freq/srate) {
                channelData[i] = ranSamp(level);
            }
        }
    }
}

// Brown Noise

void MainComponent::brownNoise (const AudioSourceChannelInfo& bufferToFill) {
    for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (int i = 0; i < bufferToFill.numSamples ; ++i)
        {
            channelData[i] = lowPass(1.5 * ranSamp(level), channelData[i-1], 0.05);
        }
    }
}

//==============================================================================
// Sine Wave
//==============================================================================

void MainComponent::sineWave (const AudioSourceChannelInfo& bufferToFill) {
    double initialPhase = phase;
    for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        // phase will have to be reset to its initial value in the function for every channel, that is,
        // phase should evolve the same way across all channels.
        phase = initialPhase;
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (int i = 0; i < bufferToFill.numSamples ; ++i)
        {
            // VERSION 1
//            channelData[i] = (float) std::sin(TwoPi * phase) * level;
//            phase += phaseDelta;
            
            // VERSION 2: using a phasor
            channelData[i] = (float) std::sin(TwoPi * phasor()) * level;
            phase += phaseDelta;
        }
    }
}

//==============================================================================
// Low Frequency Waveforms
//==============================================================================

/// Impulse wave

void MainComponent::LF_impulseWave (const AudioSourceChannelInfo& bufferToFill) {
    double initialPhase = phase;
    for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        phase = initialPhase;
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (int i = 0; i < bufferToFill.numSamples ; ++i)
        {
            if (phasor() + phaseDelta > 1.0) {
                channelData[i] = level;
            }
            phase += phaseDelta;
        }
    }
}

/// Square wave

void MainComponent::LF_squareWave (const AudioSourceChannelInfo& bufferToFill) {
    double initialPhase = phase;
    for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        phase = initialPhase;
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (int i = 0; i < bufferToFill.numSamples ; ++i)
        {
            if (phasor() > 0.5) {
                channelData[i] = level;
            } else {
                channelData[i] = (float) -1 * level;
            }
            phase += phaseDelta;
        }
    }
}

/// Sawtooth wave

void MainComponent::LF_sawtoothWave (const AudioSourceChannelInfo& bufferToFill) {
    double initialPhase = phase;
    for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        phase = initialPhase;
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (int i = 0; i < bufferToFill.numSamples ; ++i)
        {
            channelData[i] = (float) (2*phasor() - 1) * level;
            phase += phaseDelta;
        }
    }
}

/// Triangle wave

void MainComponent::LF_triangleWave (const AudioSourceChannelInfo& bufferToFill) {
    double initialPhase = phase;
    for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        phase = initialPhase;
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (int i = 0; i < bufferToFill.numSamples ; ++i)
        {
            if (phasor() > 0.5) {
                channelData[i] = (float) (4*phasor() - 3) * level;
            } else {
                channelData[i] = (float) (-4*phasor() + 1) * level;
            }
            phase += phaseDelta;
        }
    }
}

//==============================================================================
// Band Limited Waveforms
//==============================================================================

/// Impulse (pulse) wave

/// Synthesized by summing sin() over frequency and all its harmonics at equal
/// amplitude. To make it band limited only include harmonics that are at or
/// below the nyquist limit.
void MainComponent::BL_impulseWave (const AudioSourceChannelInfo& bufferToFill) {
    float* const channel0Data = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    for (int i = 0; i < bufferToFill.numSamples ; ++i)
    {
        // In order to avoid divisions by 0, we only fill the buffer if the frequency is greater than 0.
        if (freq > 0) {
            auto p = phasor();
            auto numHarmonics = srate / 2 / freq;
            for (int h = 1; h <= numHarmonics; ++h) {
                channel0Data[i] += std::sin(TwoPi * p * h);
            }
            channel0Data[i] *= (level/numHarmonics);
            phase += phaseDelta;
        }
    }
    for (int chan = 1; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        std::memcpy(channelData, channel0Data, bufferToFill.numSamples * sizeof(float));
    }
}

/// Square wave

/// Synthesized by summing sin() over all ODD harmonics at 1/harmonic amplitude.
/// To make it band limited only include harmonics that are at or below the
/// nyquist limit.
void MainComponent::BL_squareWave (const AudioSourceChannelInfo& bufferToFill) {
    float* const channel0Data = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    for (int i = 0; i < bufferToFill.numSamples ; ++i)
    {
        // In order to avoid divisions by 0, we only fill the buffer if the frequency is greater than 0.
        if (freq > 0) {
            auto p = phasor();
            auto numHarmonics = srate / 2 / freq;
            for (int h = 1; h <= numHarmonics; ++h) {
                if (h % 2 == 1) {
                    channel0Data[i] += std::sin(TwoPi * p * h) * (level / h);
                }
            }
            phase += phaseDelta;
        }
    }
    for (int chan = 1; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        std::memcpy(channelData, channel0Data, bufferToFill.numSamples * sizeof(float));
    }
}

/// Sawtooth wave
///
/// Synthesized by summing sin() over all harmonics at 1/harmonic amplitude. To make
/// it band limited only include harmonics that are at or below the nyquist limit.
void MainComponent::BL_sawtoothWave (const AudioSourceChannelInfo& bufferToFill) {
    float* const channel0Data = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    for (int i = 0; i < bufferToFill.numSamples ; ++i)
    {
        // In order to avoid divisions by 0, we only fill the buffer if the frequency is greater than 0.
        if (freq > 0) {
            auto p = phasor();
            auto numHarmonics = srate / 2 / freq;
            for (int h = 1; h <= numHarmonics; ++h) {
                channel0Data[i] += std::sin(TwoPi * p * h) * (level / h);
            }
            /** The maximum amplitude value when level == 1.0 and freq == 5000.0 is 1.527, so divide by that.
                Note that this doesn't actually completely circumvent clipping (i.e. values < -1.0 or > 1.0),
                as amplitude values can go above 1.527 for certain frequencies, but for the sake of simplicity we'll just divide by 1.527. */
            channel0Data[i] /= 1.527;
            phase += phaseDelta;
        }
    }
//    std::cout << juce::findMaximum(channel0Data, bufferToFill.numSamples) << std::endl;
    for (int chan = 1; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        std::memcpy(channelData, channel0Data, bufferToFill.numSamples * sizeof(float));
    }
}

/// Triangle wave
///
/// Synthesized by summing sin() over all ODD harmonics at 1/harmonic**2 amplitude.
/// To make it band limited only include harmonics that are at or below the
/// Nyquist limit.
void MainComponent::BL_triangleWave (const AudioSourceChannelInfo& bufferToFill) {
    float* const channel0Data = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    for (int i = 0; i < bufferToFill.numSamples ; ++i)
    {
        // In order to avoid divisions by 0, we only fill the buffer if the frequency is greater than 0.
        if (freq > 0) {
            auto p = phasor();
            auto numHarmonics = srate / 2 / freq;
            for (int h = 1; h <= numHarmonics; ++h) {
                if (h % 2 == 1) {
                    channel0Data[i] += std::sin(TwoPi * p * h) * (level / pow(h, 2.0));
                }
            }
            phase += phaseDelta;
        }
    }
    for (int chan = 1; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        std::memcpy(channelData, channel0Data, bufferToFill.numSamples * sizeof(float));
    }
}

//==============================================================================
// WaveTable Synthesis
//==============================================================================

// The audio block loop
void inline MainComponent::WT_wave(const AudioSourceChannelInfo& bufferToFill) {
    float* const channel0Data = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    for (int i = 0; i < bufferToFill.numSamples ; ++i)
    {
        channel0Data[i] = oscillators[waveformId-WT_START]->getNextSample() * level;
    }
    for (int chan = 1; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        std::memcpy(channelData, channel0Data, bufferToFill.numSamples * sizeof(float));
    }
}

// Create a sine wave table
void MainComponent::createSineTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize (1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer (0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
    for (auto i = 0; i < tableSize; ++i) {
        samples[i] += std::sin(phase);
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0]; // make the last value in the wavetable equal to the first value so interpolation can take place properly
}

// Create an impulse wave table
void MainComponent::createImpulseTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize (1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer (0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
    for (auto i = 0; i < tableSize; ++i) {
        auto numHarmonics = srate / 2 / 512;
        for (int h = 1; h <= numHarmonics; ++h) {
            samples[i] += std::sin(phase*h);
        }
        samples[i] *= (1/numHarmonics);
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0]; // make the last value in the wavetable equal to the first value so interpolation can take place properly
}

// Create a square wave table
void MainComponent::createSquareTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize (1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer (0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
    for (auto i = 0; i < tableSize; ++i) {
        auto numHarmonics = srate / 2;
        for (int h = 1; h <= numHarmonics; ++h) {
            if (h % 2 == 1) {
                samples[i] += std::sin(phase*h) / h;
            }
        }
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0]; // make the last value in the wavetable equal to the first value so interpolation can take place properly
}

// Create a sawtooth wave table
void MainComponent::createSawtoothTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize (1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer (0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
    for (auto i = 0; i < tableSize; ++i) {
        auto numHarmonics = srate / 2;
        for (int h = 1; h <= numHarmonics; ++h) {
            samples[i] += std::sin(phase*h) / h;
        }
        samples[i] /= 1.6;
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0]; // make the last value in the wavetable equal to the first value so interpolation can take place properly
}

// Create a triagle wave table
void MainComponent::createTriangleTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize (1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer (0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
    for (auto i = 0; i < tableSize; ++i) {
        auto numHarmonics = srate / 2;
        for (int h = 1; h <= numHarmonics; ++h) {
            if (h % 2 == 1) {
                samples[i] += std::sin(phase*h) / pow(h, 2.0);
            }
        }
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0]; // make the last value in the wavetable equal to the first value so interpolation can take place properly
}
