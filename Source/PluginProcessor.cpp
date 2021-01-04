#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessor::ImogenAudioProcessor():
    AudioProcessor(makeBusProperties()),
    tree(*this, nullptr, "PARAMETERS", createParameters()),
    wetBuffer(2, MAX_BUFFERSIZE), dryBuffer(2, MAX_BUFFERSIZE),
    limiterIsOn(true), inputGainMultiplier(1.0f), outputGainMultiplier(1.0f),
    prevDryPan(64), prevideb(0.0f), prevodeb(0.0f),
    modulatorInput(ModulatorInputSource::left)
{
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    dryPan             = dynamic_cast<AudioParameterInt*>  (tree.getParameter("dryPan"));                   jassert(dryPan);
    dryWet             = dynamic_cast<AudioParameterInt*>  (tree.getParameter("masterDryWet"));             jassert(dryWet);
    inputChan          = dynamic_cast<AudioParameterInt*>  (tree.getParameter("inputChan"));                jassert(inputChan);
    adsrAttack         = dynamic_cast<AudioParameterFloat*>(tree.getParameter("adsrAttack"));               jassert(adsrAttack);
    adsrDecay          = dynamic_cast<AudioParameterFloat*>(tree.getParameter("adsrDecay"));                jassert(adsrDecay);
    adsrSustain        = dynamic_cast<AudioParameterFloat*>(tree.getParameter("adsrSustain"));              jassert(adsrSustain);
    adsrRelease        = dynamic_cast<AudioParameterFloat*>(tree.getParameter("adsrRelease"));              jassert(adsrRelease);
    adsrToggle         = dynamic_cast<AudioParameterBool*> (tree.getParameter("adsrOnOff"));                jassert(adsrToggle);
    quickKillMs        = dynamic_cast<AudioParameterInt*>  (tree.getParameter("quickKillMs"));              jassert(quickKillMs);
    quickAttackMs      = dynamic_cast<AudioParameterInt*>  (tree.getParameter("quickAttackMs"));            jassert(quickAttackMs);
    stereoWidth        = dynamic_cast<AudioParameterInt*>  (tree.getParameter("stereoWidth"));              jassert(stereoWidth);
    lowestPanned       = dynamic_cast<AudioParameterInt*>  (tree.getParameter("lowestPan"));                jassert(lowestPanned);
    velocitySens       = dynamic_cast<AudioParameterInt*>  (tree.getParameter("midiVelocitySensitivity"));  jassert(velocitySens);
    pitchBendUp        = dynamic_cast<AudioParameterInt*>  (tree.getParameter("PitchBendUpRange"));         jassert(pitchBendUp);
    pitchBendDown      = dynamic_cast<AudioParameterInt*>  (tree.getParameter("PitchBendDownRange"));       jassert(pitchBendDown);
    pedalPitchIsOn     = dynamic_cast<AudioParameterBool*> (tree.getParameter("pedalPitchToggle"));         jassert(pedalPitchIsOn);
    pedalPitchThresh   = dynamic_cast<AudioParameterInt*>  (tree.getParameter("pedalPitchThresh"));         jassert(pedalPitchThresh);
    pedalPitchInterval = dynamic_cast<AudioParameterInt*>  (tree.getParameter("pedalPitchInterval"));       jassert(pedalPitchInterval);
    descantIsOn        = dynamic_cast<AudioParameterBool*> (tree.getParameter("descantToggle"));            jassert(descantIsOn);
    descantThresh      = dynamic_cast<AudioParameterInt*>  (tree.getParameter("descantThresh"));            jassert(descantThresh);
    descantInterval    = dynamic_cast<AudioParameterInt*>  (tree.getParameter("descantInterval"));          jassert(descantInterval);
    concertPitchHz     = dynamic_cast<AudioParameterInt*>  (tree.getParameter("concertPitch"));             jassert(concertPitchHz);
    voiceStealing      = dynamic_cast<AudioParameterBool*> (tree.getParameter("voiceStealing"));            jassert(voiceStealing);
    latchIsOn          = dynamic_cast<AudioParameterBool*> (tree.getParameter("latchIsOn"));                jassert(latchIsOn);
    inputGain          = dynamic_cast<AudioParameterFloat*>(tree.getParameter("inputGain"));                jassert(inputGain);
    outputGain         = dynamic_cast<AudioParameterFloat*>(tree.getParameter("outputGain"));               jassert(outputGain);
    limiterToggle      = dynamic_cast<AudioParameterBool*> (tree.getParameter("limiterIsOn"));              jassert(limiterToggle);
    limiterThresh      = dynamic_cast<AudioParameterFloat*>(tree.getParameter("limiterThresh"));            jassert(limiterThresh);
    limiterRelease     = dynamic_cast<AudioParameterInt*>  (tree.getParameter("limiterRelease"));           jassert(limiterRelease);
    
    initialize(44100.0, MAX_BUFFERSIZE, 12);
};

ImogenAudioProcessor::~ImogenAudioProcessor()
{ };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock)
{
    if(host.isLogic() || host.isGarageBand())
        if ( getBusesLayout().getChannelSet(true, 1) == AudioChannelSet::disabled() )
        {
            // give user a warning that sidechain must be enabled
        }
    
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    updateSampleRate(sampleRate);
    
    dspSpec.sampleRate = sampleRate;
    dspSpec.maximumBlockSize = MAX_BUFFERSIZE;
    dspSpec.numChannels = 2;
    
    // limiter
    limiter.prepare(dspSpec);
    
    // dry wet mixer
    dryWetMixer.prepare(dspSpec);
    dryWetMixer.setWetLatency(2); // latency in samples of the ESOLA algorithm
    
    updateAllParameters();
    harmonizer.resetNoteOnCounter(); // ??
};


void ImogenAudioProcessor::releaseResources()
{
    clearBuffers();
    harmonizer.resetNoteOnCounter();
};


void ImogenAudioProcessor::reset()
{
    harmonizer.allNotesOff(false);
    harmonizer.resetNoteOnCounter();
    clearBuffers();
    dryWetMixer.reset();
    limiter.reset();
};


// audio rendering ----------------------------------------------------------------------------------------------------------------------------------

void ImogenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if( (host.isLogic() || host.isGarageBand()) && (getBusesLayout().getChannelSet(true, 1) == AudioChannelSet::disabled()) )
    {
        // give user a warning that sidechain audio input must be enabled
        return;
    }
    
    AudioBuffer<float> inBus  = AudioProcessor::getBusBuffer(buffer, true, (host.isLogic() || host.isGarageBand()));
    AudioBuffer<float> output = AudioProcessor::getBusBuffer(buffer, false, 0);

    if (isSuspended())
    {
        output = inBus;
        return;
    }
    
    updateSampleRate(getSampleRate());
    updateAllParameters();
    
    const int totalNumSamples = inBus.getNumSamples();
    
    int inputChannelIndexInInputBuffer;
    // TO DO: if host is Logic or Garageband, is this unnecessary? would the sidechain input always be channel 0 of inBus?
    switch(modulatorInput)
    {
        case ModulatorInputSource::left:
            inputChannelIndexInInputBuffer = 0;
            break;
        case ModulatorInputSource::right:
            inputChannelIndexInInputBuffer = 1;
            break;
        case ModulatorInputSource::mixToMono:
        {
            jassert(inBus.getNumChannels() > 0);
            const int channelToUse = 0;
            AudioBuffer<float> destProxy (inBus.getArrayOfWritePointers() + channelToUse, 1, totalNumSamples);
            
            for(int chan = 0; chan < inBus.getNumChannels(); ++chan)
                destProxy.addFrom(0, 0, inBus, chan, 0, totalNumSamples);
            
            destProxy.applyGain(1 / inBus.getNumChannels());
            inputChannelIndexInInputBuffer = channelToUse;
            break;
        }
    }
    
    AudioBuffer<float> input (inBus.getArrayOfWritePointers() + inputChannelIndexInInputBuffer, 1, totalNumSamples);
    // input needs to be a MONO buffer containing the input modulator signal... so whether it needs to get channel 0 or 1 of the input bus, or sum the 2 channels to mono, either way this buffer needs to contain THAT data

    auto midiIterator = midiMessages.findNextSamplePosition(0);

    int  numSamples  = totalNumSamples;
    int  startSample = 0;
    bool firstEvent  = true;

    for (; numSamples > 0; ++midiIterator)
    {
        if (midiIterator == midiMessages.cend())
        {
            processBlockPrivate(input, output, startSample, numSamples);
            return;
        }
        
        const auto metadata = *midiIterator;
        const int  samplesToNextMidiMessage = metadata.samplePosition - startSample;
        
        if (samplesToNextMidiMessage >= numSamples)
        {
            processBlockPrivate(input, output, startSample, numSamples);
            harmonizer.handleMidiEvent(metadata.getMessage());
            break;
        }
        
        if (firstEvent && samplesToNextMidiMessage == 0)
        {
            harmonizer.handleMidiEvent(metadata.getMessage());
            continue;
        }
        
        firstEvent = false;
        
        processBlockPrivate(input, output, startSample, samplesToNextMidiMessage);
        
        harmonizer.handleMidiEvent(metadata.getMessage());
        
        startSample += samplesToNextMidiMessage;
        numSamples  -= samplesToNextMidiMessage;
    }

    std::for_each (midiIterator,
                   midiMessages.cend(),
                   [&] (const MidiMessageMetadata& meta) { harmonizer.handleMidiEvent (meta.getMessage()); } );
};


void ImogenAudioProcessor::processBlockPrivate(AudioBuffer<float>& inBuffer, AudioBuffer<float>& outBuffer,
                                               const int startSample, const int numSamples)
{
    if (isNonRealtime())
    {
        AudioBuffer<float> inProxy  (inBuffer .getArrayOfWritePointers(), 1, startSample, numSamples);
        AudioBuffer<float> outProxy (outBuffer.getArrayOfWritePointers(), 2, startSample, numSamples);
        
        renderChunk(inProxy, outProxy);
    }
    else
    {
        int chunkStartSample = startSample;
        int samplesLeft      = numSamples;
    
        while(samplesLeft > 0)
        {
            const int chunkNumSamples = std::min(samplesLeft, MAX_BUFFERSIZE);
            
            AudioBuffer<float> inProxy  (inBuffer .getArrayOfWritePointers(), 1, chunkStartSample, chunkNumSamples);
            AudioBuffer<float> outProxy (outBuffer.getArrayOfWritePointers(), 2, chunkStartSample, chunkNumSamples);
            
            renderChunk(inProxy, outProxy);
            
            chunkStartSample += chunkNumSamples;
            samplesLeft      -= chunkNumSamples;
        }
    }
};


void ImogenAudioProcessor::renderChunk(AudioBuffer<float>& inBuffer, AudioBuffer<float>& outBuffer)
{
    // regardless of the input channel(s) setup, the inBuffer fed to this function should be a mono buffer with its audio content in channel 0
    // outBuffer should be a stereo buffer with the same length in samples as inBuffer
    // # of samples in the I/O buffers must be less than or equal to MAX_BUFFERSIZE
    
    const int numSamples = inBuffer.getNumSamples();
    
    if (isNonRealtime() && wetBuffer.getNumSamples() < numSamples)
        increaseBufferSizes(numSamples);
    
    updateSampleRate(getSampleRate());
    updateAllParameters();

    inBuffer.applyGain(inputGainMultiplier); // apply input gain
    
    writeToDryBuffer(inBuffer); // puts input samples into dryBuffer w/ proper panning applied
    
    AudioBuffer<float> dryProxy (dryBuffer.getArrayOfWritePointers(), 2, 0, numSamples);
    AudioBuffer<float> wetProxy (wetBuffer.getArrayOfWritePointers(), 2, 0, numSamples);
    
    dryWetMixer.pushDrySamples( dsp::AudioBlock<float>(dryProxy) );

    harmonizer.renderVoices(inBuffer, wetProxy); // puts the harmonizer's rendered stereo output into "wetProxy" (= "wetBuffer")
    
    dryWetMixer.mixWetSamples( dsp::AudioBlock<float>(wetProxy) ); // puts the mixed dry & wet samples into "wetProxy" (= "wetBuffer")
    
    outBuffer.makeCopyOf(wetProxy, true); // transfer from wetBuffer to output buffer
    
    outBuffer.applyGain(outputGainMultiplier); // apply master output gain
    
    // output limiter
    if(limiterIsOn)
    {
        dsp::AudioBlock<float> limiterBlock (outBuffer);
        limiter.process(dsp::ProcessContextReplacing<float>(limiterBlock));
    }
};


void ImogenAudioProcessor::writeToDryBuffer(const AudioBuffer<float>& input)
{
    // writes from "input" into dryBuffer & applies panning
    
    const int numSamples = input.getNumSamples();
    
    dryBuffer.copyFrom (0, 0, input, 0, 0, numSamples);
    dryBuffer.copyFrom (1, 0, input, 0, 0, numSamples);
    dryBuffer.applyGain(0, 0, numSamples, dryvoxpanningmults[0]);
    dryBuffer.applyGain(1, 0, numSamples, dryvoxpanningmults[1]);
};


/*+++++++++++++++++++++++++++++++++++++
 DANGER!!!
 FOR NON REAL TIME ONLY!!!!!!!!
 ++++++++++++++++++++++++++++++++++++++*/
void ImogenAudioProcessor::increaseBufferSizes(const int newMaxBlocksize)
{
    suspendProcessing (true);
    
    wetBuffer.setSize(2, newMaxBlocksize);
    dryBuffer.setSize(2, newMaxBlocksize);
    harmonizer.increaseBufferSizes(newMaxBlocksize);
    
    dspSpec.maximumBlockSize = newMaxBlocksize;
    limiter.prepare(dspSpec);
    dryWetMixer.prepare(dspSpec);
    
    suspendProcessing (false);
};


/*===========================================================================================================================
 ============================================================================================================================*/


void ImogenAudioProcessor::processBlockBypassed (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ignoreUnused(buffer);
    ignoreUnused(midiMessages);
    
    // add latency compensation here
};


void ImogenAudioProcessor::clearBuffers()
{
    harmonizer.clearBuffers();
    wetBuffer.clear();
    dryBuffer.clear();
};


// functions for updating parameters ----------------------------------------------------------------------------------------------------------------

void ImogenAudioProcessor::updateAllParameters()
{
    updateIOgains();
    updateLimiter();
    updateDryVoxPan();
    updateDryWet();
    updateQuickKillMs();
    updateQuickAttackMs();
    updateNoteStealing();
    
    // these parameter functions have the potential to alter the pitch & other properties of currently playing harmonizer voices:
    updateConcertPitch();
    updatePitchbendSettings();
    updateStereoWidth();
    updateMidiVelocitySensitivity();
    updateAdsr();
    
    // these parameter functions have the potential to trigger or turn off midi notes / harmonizer voices:
    updateMidiLatch();
    updatePedalPitch();
    updateDescant();
};

void ImogenAudioProcessor::updateSampleRate(const double newSamplerate)
{
    if(harmonizer.getSamplerate() != newSamplerate)
        harmonizer.setCurrentPlaybackSampleRate(newSamplerate);
};

void ImogenAudioProcessor::updateDryVoxPan()
{
    const int newDryPan = dryPan->get();
    
    if(newDryPan != prevDryPan)
    {
        const float Rpan = newDryPan / 127.0f;
        dryvoxpanningmults[1] = Rpan;
        dryvoxpanningmults[0] = 1.0f - Rpan;
        prevDryPan = newDryPan;
    }
};

void ImogenAudioProcessor::updateDryWet()
{
    dryWetMixer.setWetMixProportion(dryWet->get() / 100.0f);
    
    // need to set latency!!!
};

void ImogenAudioProcessor::updateAdsr()
{
    harmonizer.updateADSRsettings(adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get());
    harmonizer.setADSRonOff      (adsrToggle->get());
};

void ImogenAudioProcessor::updateQuickKillMs()
{
    harmonizer.updateQuickReleaseMs(quickKillMs->get());
};

void ImogenAudioProcessor::updateQuickAttackMs()
{
    harmonizer.updateQuickAttackMs(quickAttackMs->get());
};

void ImogenAudioProcessor::updateStereoWidth()
{
    harmonizer.updateLowestPannedNote(lowestPanned->get());
    harmonizer.updateStereoWidth     (stereoWidth->get());
};

void ImogenAudioProcessor::updateMidiVelocitySensitivity()
{
    harmonizer.updateMidiVelocitySensitivity(velocitySens->get());
};

void ImogenAudioProcessor::updatePitchbendSettings()
{
    harmonizer.updatePitchbendSettings(pitchBendUp->get(), pitchBendDown->get());
};

void ImogenAudioProcessor::updatePedalPitch()
{
    harmonizer.setPedalPitch           (pedalPitchIsOn->get());
    harmonizer.setPedalPitchUpperThresh(pedalPitchThresh->get());
    harmonizer.setPedalPitchInterval   (pedalPitchInterval->get());
};

void ImogenAudioProcessor::updateDescant()
{
    harmonizer.setDescant           (descantIsOn->get());
    harmonizer.setDescantLowerThresh(descantThresh->get());
    harmonizer.setDescantInterval   (descantInterval->get());
};

void ImogenAudioProcessor::updateConcertPitch()
{
    harmonizer.setConcertPitchHz(concertPitchHz->get());
};

void ImogenAudioProcessor::updateNoteStealing()
{
    harmonizer.setNoteStealingEnabled(voiceStealing->get());
};

void ImogenAudioProcessor::updateMidiLatch()
{
    const bool allowTailOff = true;
    harmonizer.setMidiLatch(latchIsOn->get(), allowTailOff);
};

void ImogenAudioProcessor::updateIOgains()
{
    const float newIn = inputGain->get();
    if(newIn != prevideb)
        inputGainMultiplier = Decibels::decibelsToGain(newIn);
    prevideb = newIn;
    
    const float newOut = outputGain->get();
    if(newOut != prevodeb)
        outputGainMultiplier = Decibels::decibelsToGain(newOut);
    prevodeb = newOut;
};

void ImogenAudioProcessor::updateLimiter()
{
    limiterIsOn = limiterToggle->get();
    limiter.setThreshold(limiterThresh->get());
    limiter.setRelease  (limiterRelease->get());
};

void ImogenAudioProcessor::updateNumVoices(const int newNumVoices)
{
    const int currentVoices = harmonizer.getNumVoices();
    
    if(currentVoices != newNumVoices)
    {
        if(newNumVoices > currentVoices)
        {
            for(int i = 0; i < newNumVoices - currentVoices; ++i)
                harmonizer.addVoice(new HarmonizerVoice(&harmonizer));
            
            if(newNumVoices > MAX_POSSIBLE_NUMBER_OF_VOICES)
                harmonizer.newMaxNumVoices(newNumVoices); // increases storage overheads for internal harmonizer functions dealing with arrays of notes, etc
        }
        else
            harmonizer.removeNumVoices(currentVoices - newNumVoices);
        
        // update GUI numVoices ComboBox
    }
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// functions for custom preset management system ----------------------------------------------------------------------------------------------------

void ImogenAudioProcessor::savePreset(juce::String presetName) 
{
    // this function can be used both to save new preset files or to update existing ones
    
    File writingTo = getPresetsFolder().getChildFile(presetName);
    
    auto xml(tree.copyState().createXml());
    xml->setAttribute("numberOfVoices", harmonizer.getNumVoices());
    xml->writeTo(writingTo);
    updateHostDisplay();
};


void ImogenAudioProcessor::loadPreset(juce::String presetName)
{
    File presetToLoad = getPresetsFolder().getChildFile(presetName);
    
    if(presetToLoad.existsAsFile())
    {
        auto xmlElement = juce::parseXML(presetToLoad);
        
        if(xmlElement.get() != nullptr && xmlElement->hasTagName (tree.state.getType()))
        {
            tree.replaceState(juce::ValueTree::fromXml (*xmlElement));
            updateNumVoices( xmlElement->getIntAttribute("numberOfVoices", 4) ); // TO DO : send notif to GUI to update numVoices comboBox
            updateHostDisplay();
        }
    }
};


void ImogenAudioProcessor::deletePreset(juce::String presetName) 
{
    File presetToDelete = getPresetsFolder().getChildFile(presetName);
    
    if(presetToDelete.existsAsFile())
        if(! presetToDelete.moveToTrash())
            presetToDelete.deleteFile();
    updateHostDisplay();
};


juce::File ImogenAudioProcessor::getPresetsFolder() const
{
    File rootFolder;
    
#ifdef JUCE_MAC
    rootFolder = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory);
    rootFolder = rootFolder.getChildFile("Audio").getChildFile("Presets").getChildFile("Ben Vining Music Software").getChildFile("Imogen");
#endif
    
#ifdef JUCE_WINDOWS
    rootFolder = File::getSpecialLocation(File::SpecialLocationType::UserDocumentsDirectory);
    rootFolder = rootFolder.getChildFile("Ben Vining Music Software").getChildFile("Imogen");
#endif
    
#ifdef JUCE_LINUX
    rootFolder = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory);
    rootFolder = rootFolder.getChildFile("Ben Vining Music Software").getChildFile("Imogen");
#endif
    
    if(! rootFolder.isDirectory() && ! rootFolder.existsAsFile())
        rootFolder.createDirectory(); // creates the presets folder if it doesn't already exist
    
    return rootFolder;
};


void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto xml(tree.copyState().createXml());
    xml->setAttribute("numberOfVoices", harmonizer.getNumVoices());
    copyXmlToBinary (*xml, destData);
};


void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr && xmlState->hasTagName (tree.state.getType()))
    {
        tree.replaceState(juce::ValueTree::fromXml (*xmlState));
        const int newNumOfVoices = xmlState->getIntAttribute("numberOfVoices", 4);
        updateNumVoices(newNumOfVoices); // TO DO : send notif to GUI to update numVoices comboBox
    }
};


// standard and general-purpose functions -----------------------------------------------------------------------------------------------------------

AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters() const
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    
    // general
    params.push_back(std::make_unique<AudioParameterInt>	("dryPan", "Dry vox pan", 0, 127, 64));
    
    params.push_back(std::make_unique<AudioParameterInt>	("masterDryWet", "% wet", 0, 100, 100));
    
    params.push_back(std::make_unique<AudioParameterInt>	("inputChan", "Input channel", 0, 16, 0));
    
    // ADSR
    params.push_back(std::make_unique<AudioParameterFloat> 	("adsrAttack", "ADSR Attack", NormalisableRange<float> (0.001f, 1.0f, 0.001f), 0.035f));
    params.push_back(std::make_unique<AudioParameterFloat> 	("adsrDecay", "ADSR Decay", NormalisableRange<float> (0.001f, 1.0f, 0.001f), 0.06f));
    params.push_back(std::make_unique<AudioParameterFloat> 	("adsrSustain", "ADSR Sustain", NormalisableRange<float> (0.01f, 1.0f, 0.01f), 0.8f));
    params.push_back(std::make_unique<AudioParameterFloat> 	("adsrRelease", "ADSR Release", NormalisableRange<float> (0.001f, 1.0f, 0.001f), 0.1f));
    params.push_back(std::make_unique<AudioParameterBool>	("adsrOnOff", "ADSR on/off", true));
    params.push_back(std::make_unique<AudioParameterInt>	("quickKillMs", "Quick kill ms", 1, 250, 15));
    params.push_back(std::make_unique<AudioParameterInt>	("quickAttackMs", "Quick attack ms", 1, 250, 15));
    
    // stereo width
    params.push_back(std::make_unique<AudioParameterInt> 	("stereoWidth", "Stereo Width", 0, 100, 100));
    params.push_back(std::make_unique<AudioParameterInt>	("lowestPan", "Lowest panned midiPitch", 0, 127, 0));
    
    // midi settings
    params.push_back(std::make_unique<AudioParameterInt> 	("midiVelocitySensitivity", "MIDI Velocity Sensitivity", 0, 100, 100));
    params.push_back(std::make_unique<AudioParameterInt> 	("PitchBendUpRange", "Pitch bend range (up)", 0, 12, 2));
    params.push_back(std::make_unique<AudioParameterInt>	("PitchBendDownRange", "Pitch bend range (down)", 0, 12, 2));
    // pedal pitch
    params.push_back(std::make_unique<AudioParameterBool>	("pedalPitchToggle", "Pedal pitch on/off", false));
    params.push_back(std::make_unique<AudioParameterInt>	("pedalPitchThresh", "Pedal pitch upper threshold", 0, 127, 0));
    params.push_back(std::make_unique<AudioParameterInt>	("pedalPitchInterval", "Pedal pitch interval", 1, 12, 12));
    // descant
    params.push_back(std::make_unique<AudioParameterBool>	("descantToggle", "Descant on/off", false));
    params.push_back(std::make_unique<AudioParameterInt>	("descantThresh", "Descant lower threshold", 0, 127, 127));
    params.push_back(std::make_unique<AudioParameterInt>	("descantInterval", "Descant interval", 1, 12, 12));
    // concert pitch Hz
    params.push_back(std::make_unique<AudioParameterInt>	("concertPitch", "Concert pitch (Hz)", 392, 494, 440));
    // voice stealing
    params.push_back(std::make_unique<AudioParameterBool>	("voiceStealing", "Voice stealing", false));
    // midi latch
    params.push_back(std::make_unique<AudioParameterBool>	("latchIsOn", "MIDI latch on/off", false));
    
    // input & output gain
    params.push_back(std::make_unique<AudioParameterFloat>	("inputGain", "Input Gain", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<AudioParameterFloat>	("outputGain", "Output Gain", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -4.0f));
    
    // output limiter
    params.push_back(std::make_unique<AudioParameterBool>	("limiterIsOn", "Limiter on/off", true));
    params.push_back(std::make_unique<AudioParameterFloat>	("limiterThresh", "Limiter threshold", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -2.0f));
    params.push_back(std::make_unique<AudioParameterInt>	("limiterRelease", "limiter release (ms)", 1, 250, 10));
    
    return { params.begin(), params.end() };
};



void ImogenAudioProcessor::initialize(const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices)
{
    for (int i = 0; i < initNumVoices; ++i)
        harmonizer.addVoice(new HarmonizerVoice(&harmonizer));
    
    if(host.isLogic() || host.isGarageBand())
        if ( getBusesLayout().getChannelSet(true, 1) == AudioChannelSet::disabled() )
        {
            // give user a warning that sidechain must be enabled
        }
    
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    updateSampleRate(initSamplerate);
    
    clearBuffers();
    
    dspSpec.sampleRate = initSamplerate;
    dspSpec.maximumBlockSize = MAX_BUFFERSIZE;
    dspSpec.numChannels = 2;
    
    // limiter
    limiter.prepare(dspSpec);
    
    // dry wet mixer
    dryWetMixer.setMixingRule(dsp::DryWetMixingRule::linear);
    dryWetMixer.prepare(dspSpec);
    dryWetMixer.setWetLatency(2); // latency in samples of the ESOLA algorithm
    
    updateAllParameters();
};


double ImogenAudioProcessor::getTailLengthSeconds() const
{
    if(harmonizer.isADSRon())
        return double(adsrRelease->get()); // ADSR release time in seconds
    
    return double(quickKillMs->get() * 1000.0f); // "quick kill" time in seconds
};

int ImogenAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs, so this should be at least 1, even if you're not really implementing programs.
};

int ImogenAudioProcessor::getCurrentProgram() {
    return 0;
};

void ImogenAudioProcessor::setCurrentProgram (int index)
{ };

const juce::String ImogenAudioProcessor::getProgramName (int index) {
    return {};
};

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName)
{ };


AudioProcessor::BusesProperties ImogenAudioProcessor::makeBusProperties() const
{
    if (host.isLogic() || host.isGarageBand())
        return BusesProperties().withInput ("Input",     AudioChannelSet::stereo(), true)
                                .withInput ("Sidechain", AudioChannelSet::mono(),   true)
                                .withOutput("Output",    AudioChannelSet::stereo(), true);
    
    return     BusesProperties().withInput ("Input",     AudioChannelSet::stereo(), true)
                                .withOutput("Output",    AudioChannelSet::stereo(), true);
};


bool ImogenAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if ( layouts.getMainInputChannelSet()  == juce::AudioChannelSet::disabled() && (! (host.isLogic() || host.isGarageBand())) )
        return false;
    
    if ( layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled() )
        return false;
    
    if ( layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
      || layouts.getMainInputChannelSet()  != juce::AudioChannelSet::stereo() )
        return false;
    
    return true;
};


juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor()
{
    return new ImogenAudioProcessorEditor(*this);
};

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImogenAudioProcessor();
};



