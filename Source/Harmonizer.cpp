/*
  ==============================================================================

    Harmonizer.cpp
    Created: 13 Dec 2020 7:53:39pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "Harmonizer.h"


HarmonizerVoice::HarmonizerVoice(): adsrIsOn(true), quickReleaseMs(15), isFading(false), converter(440), currentlyPlayingNote(-1), currentOutputFreq(-1.0f), currentVelocityMultiplier(0.0f), pitchbendRangeUp(2), pitchbendRangeDown(2), lastRecievedPitchbend(64), lastRecievedVelocity(0), currentSampleRate(44100.0), noteOnTime(0), keyIsDown(false), sustainPedalDown(false), sostenutoPedalDown(false), midiVelocitySensitivity(1.0f), currentMidipan(64), currentInputFreq(0.0f)
{
	panningMults[0] = 0.5f;
	panningMults[1] = 0.5f;
	
	adsr.setSampleRate(44100.0);
	adsrParams.attack = 0.035f;
	adsrParams.decay = 0.06f;
	adsrParams.sustain = 0.8f;
	adsrParams.release = 0.01f;
	adsr.setParameters(adsrParams);
	
	quickRelease.setSampleRate(44100.0);
	quickReleaseParams.attack = 0.01f;
	quickReleaseParams.decay = 0.01f;
	quickReleaseParams.sustain = 1.0f;
	quickReleaseParams.release = 0.015f;
	quickRelease.setParameters(quickReleaseParams);
	
	tempBuffer.setSize(1, MAX_BUFFERSIZE);
};


HarmonizerVoice::~HarmonizerVoice()
{ };

void HarmonizerVoice::setCurrentPlaybackSamplerate(const double newRate)
{
	jassert(newRate > 0);
	
	if(currentSampleRate != newRate)
	{
		currentSampleRate = newRate;
		adsr.setSampleRate(newRate);
		adsr.setParameters(adsrParams);
		quickRelease.setSampleRate(newRate);
		quickRelease.setParameters(quickReleaseParams);
	}
};


void HarmonizerVoice::setConcertPitch(const int newConcertPitch)
{
	if(converter.getCurrentConcertPitchHz() != newConcertPitch)
	{
		converter.setConcertPitchHz(newConcertPitch);
		if(currentlyPlayingNote >= 0)
			currentOutputFreq = getOutputFreqFromMidinoteAndPitchbend(currentlyPlayingNote, lastRecievedPitchbend);
	}
}


void HarmonizerVoice::renderNextBlock(AudioBuffer<float>& inputAudio, const int inputChan, const int startSample, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices)
{

	if(! (sustainPedalDown || sostenutoPedalDown))
	{
		if(! keyIsDown)
			stopNote(1.0f, false);
	}
	
	if(adsr.isActive()) // use the adsr envelope to determine if the voice is on or not...
	{
		AudioBuffer<float> subBuffer(outputBuffer.getArrayOfWritePointers(), 2, startSample, numSamples);
	
		// puts shifted samples into the tempBuffer
		esola(inputAudio, inputChan, startSample, numSamples, tempBuffer, epochIndices,
			  	1.0f / (1.0f + ((currentInputFreq - currentOutputFreq)/currentOutputFreq)) ); // shifting ratio
		
		subBuffer.addFrom(1, startSample, tempBuffer, 1, startSample, numSamples, panningMults[0]);
		subBuffer.addFrom(2, startSample, tempBuffer, 2, startSample, numSamples, panningMults[1]);
		
		subBuffer.applyGain(startSample, numSamples, currentVelocityMultiplier);
	
		if(adsrIsOn) //...but only apply the envelope if the ADSR on/off user toggle is ON
			adsr.applyEnvelopeToBuffer(subBuffer, startSample, numSamples);
		
		// quick fade out for stopNote() w/ allowTailOff = false:
		if(isFading)
		{
			if(! quickRelease.isActive()) { quickRelease.noteOn(); quickRelease.noteOff(); }  // ??
			quickRelease.applyEnvelopeToBuffer(subBuffer, startSample, numSamples);
			adsr.reset();
			isFading = false;
			clearCurrentNote();
		}
		
	}
	else
	{
		clearCurrentNote();
		currentMidipan = 64;
		
		if(! isFading)
		{
			quickRelease.noteOn();
		}
	}
};


// MIDI -----------------------------------------------------------------------------------------------------------

float HarmonizerVoice::getOutputFreqFromMidinoteAndPitchbend(const int lastRecievedNote, const int pitchBend)
{
	jassert(lastRecievedNote >= 0);
	
	if(pitchBend == 64)
	{
		return converter.mtof(lastRecievedNote);
	}
	else if(pitchBend > 64)
	{
		return converter.mtof( ((pitchbendRangeUp * (pitchBend - 65)) / 62) + lastRecievedNote );
	}
	else
	{
		return converter.mtof( (((1 - pitchbendRangeDown) * pitchBend) / 63) + lastRecievedNote - pitchbendRangeDown );
	}
	
};

void HarmonizerVoice::setMidiVelocitySensitivity(const float newsensitity)
{
	midiVelocitySensitivity = newsensitity;
	
	if(currentlyPlayingNote >= 0)
		currentVelocityMultiplier = calcVelocityMultiplier(lastRecievedVelocity);
};

float HarmonizerVoice::calcVelocityMultiplier(const float inputVelocity)
{
	return ((1.0f - inputVelocity) * (1.0f - midiVelocitySensitivity) + inputVelocity);
};

void HarmonizerVoice::startNote(const int midiPitch, const float velocity, const int currentPitchWheelPosition)
{
	currentlyPlayingNote = midiPitch;
	lastRecievedPitchbend = currentPitchWheelPosition;
	lastRecievedVelocity = velocity;
	currentOutputFreq = getOutputFreqFromMidinoteAndPitchbend(midiPitch, currentPitchWheelPosition);
	currentVelocityMultiplier = calcVelocityMultiplier(velocity);
	adsr.setParameters(adsrParams);
	adsr.noteOn();
	if(! quickRelease.isActive()) { quickRelease.noteOn(); }
	isFading = false;
};

void HarmonizerVoice::changeNote(const int midiPitch, const float velocity, const int currentPitchWheelPosition)
{
	currentlyPlayingNote = midiPitch;
	lastRecievedPitchbend = currentPitchWheelPosition;
	lastRecievedVelocity = velocity;
	currentOutputFreq = getOutputFreqFromMidinoteAndPitchbend(midiPitch, currentPitchWheelPosition);
	currentVelocityMultiplier = calcVelocityMultiplier(velocity);
	if(! adsr.isActive()) { adsr.noteOn(); }
	if(! quickRelease.isActive()) { quickRelease.noteOn(); }
	isFading = false;
};

void HarmonizerVoice::stopNote(const float velocity, const bool allowTailOff)
{
	if (allowTailOff)
	{
		adsr.noteOff();
		isFading = false;
	}
	else
	{
		if(! quickRelease.isActive()) { quickRelease.noteOn(); }
		if(const float desiredR = quickReleaseMs / 1000.0f; quickReleaseParams.release != desiredR)
		{
			quickReleaseParams.release = desiredR;
			quickRelease.setParameters(quickReleaseParams);
		}
		isFading = true;
		quickRelease.noteOff();
	}
	lastRecievedVelocity = 0.0f;
};

void HarmonizerVoice::pitchWheelMoved(const int newPitchWheelValue)
{
	lastRecievedPitchbend = newPitchWheelValue;
	
	if(currentlyPlayingNote >= 0)
		currentOutputFreq = getOutputFreqFromMidinoteAndPitchbend(currentlyPlayingNote, newPitchWheelValue);
};

void HarmonizerVoice::aftertouchChanged(const int) { };

void HarmonizerVoice::channelPressureChanged(const int) { };

void HarmonizerVoice::controllerMoved(const int controllerNumber, const int newControllerValue) { };


// ADSR settings -------------------------------------------------------------------------------------------------------

void HarmonizerVoice::updateAdsrSettings(const float attack, const float decay, const float sustain, const float release)
{
	adsrParams.attack = attack;
	adsrParams.decay = decay;
	adsrParams.sustain = sustain;
	adsrParams.release = release;
	adsr.setParameters(adsrParams);
};

void HarmonizerVoice::setQuickReleaseMs(const int newMs) noexcept
{
	if(const float desiredR = newMs / 1000.0f; quickReleaseParams.release != desiredR)
		quickReleaseParams.release = desiredR;
		quickRelease.setParameters(quickReleaseParams);
};


void HarmonizerVoice::updatePitchbendSettings(const int rangeUp, const int rangeDown)
{
	pitchbendRangeUp = rangeUp;
	pitchbendRangeDown = rangeDown;
	if(currentlyPlayingNote >= 0)
		currentOutputFreq = getOutputFreqFromMidinoteAndPitchbend(currentlyPlayingNote, lastRecievedPitchbend);
};

void HarmonizerVoice::setPan(const int newPan) 
{
	jassert(isPositiveAndBelow(newPan, 128));
	
	if(newPan == 64) // save time for the simplest case
	{
		panningMults[0] = 0.5f;
		panningMults[1] = 0.5f;
		currentMidipan = 64;
	}
	else if(currentMidipan != newPan)
	{
		currentMidipan = newPan;
		const float Rpan = newPan / 127.0f;
		panningMults[1] = Rpan; // R channel
		panningMults[0] = 1.0f - Rpan; // L channel
	}
};




void HarmonizerVoice::esola(AudioBuffer<float>& inputAudio, const int inputChan, const int startSample, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices, const float shiftingRatio)
{
//	int targetLength = 0;
//	int highestIndexWrittenTo = -1;
//
//	const int numOfEpochsPerFrame = 3;
//
//	int lastEpochIndex = epochIndices.getUnchecked(0);
//	const int numOfEpochs = epochIndices.size();
//
//	if(synthesis.getNumSamples() != numSamples) {
//		synthesis.setSize(1, numSamples, false, false, true);
//	}
//
//	for(int i = 0; i < numOfEpochs - numOfEpochsPerFrame; ++i) {
//		const int hop = epochIndices.getUnchecked(i + 1) - epochIndices.getUnchecked(i);
//
//		if(targetLength >= highestIndexWrittenTo) {
//			const int frameLength = epochIndices.getUnchecked(i + numOfEpochsPerFrame) - epochIndices.getUnchecked(i) - 1;
//			window.clearQuick();
//			calcWindow(frameLength, window);
//			const int bufferIncrease = frameLength - highestIndexWrittenTo + lastEpochIndex;
//
//			if(bufferIncrease > 0) {
//				const float* reading = inputAudio.getReadPointer(inputChan);
//				float* writing = synthesis.getWritePointer(0);
//				int writingindex = highestIndexWrittenTo + 1;
//				int readingindex = epochIndices.getUnchecked(i) + frameLength - 1 - bufferIncrease;
//				int windowreading = frameLength - 1 - bufferIncrease;
//
//				for(int s = 0; s < bufferIncrease; ++s) {
//					writing[writingindex] = reading[readingindex] * window.getUnchecked(s);
//					++writingindex;
//					++readingindex;
//					finalWindow.add(window.getUnchecked(windowreading));
//					++windowreading;
//				}
//				highestIndexWrittenTo += frameLength - 1;
//			}
//
//			// OLA
//			{
//				int olaindex = epochIndices.getUnchecked(i);
//				const float* olar = synthesis.getReadPointer(0);
//				float* olaw = synthesis.getWritePointer(0);
//				int wolaindex = 0;
//
//				for(int s = lastEpochIndex; s < lastEpochIndex + frameLength - bufferIncrease; ++s) {
//					olaw[s] = olar[s] + olar[olaindex];
//					++olaindex;
//					const float newfinalwindow = finalWindow.getUnchecked(s) + finalWindow.getUnchecked(wolaindex);
//					finalWindow.set(s, newfinalwindow);
//					++wolaindex;
//				}
//			}
//
//			lastEpochIndex += hop;
//		}
//		targetLength += ceil(hop * scalingFactor);
//	}
//
//	// normalize & write to output
//	const float* r = synthesis.getReadPointer(0);
//	float* w = outputBuffer.getWritePointer(0);
//
//	for(int s = 0; s < numSamples; ++s) {
//		if(s < finalWindow.size()) {
//			w[s] = r[s] / std::max<float>(finalWindow.getUnchecked(s), 1e-4);
//		} else {
//			w[s] = r[s] / 1e-4;
//		}
//	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Harmonizer::Harmonizer(): lastPitchWheelValue(0), currentInputFreq(0.0f), sampleRate(44100.0), shouldStealNotes(true), lastNoteOnCounter(0), minimumSubBlockSize(32), subBlockSubdivisionIsStrict(false), lowestPannedNote(0)
{
	currentlyActiveNotes.ensureStorageAllocated(NUMBER_OF_VOICES);
	currentlyActiveNotes.clearQuick();
	currentlyActiveNotes.add(-1);
	epochIndices.ensureStorageAllocated(NUMBER_OF_VOICES);
	epochIndices.clearQuick();
};


Harmonizer::~Harmonizer()
{
	voices.clear();
};


void Harmonizer::updateStereoWidth(const int newWidth) 
{
	const ScopedLock sl (lock);
	
	panner.updateStereoWidth(newWidth);
	
	for (auto* voice : voices)
	{
		if(voice->isVoiceActive() && voice->getCurrentlyPlayingNote() >= lowestPannedNote)
		{
			const int newPanVal = panner.getClosestNewPanValFromOld(voice->getPan());
			voice->setPan(newPanVal);
			continue;
		}
		if(voice->isVoiceActive() && voice->getCurrentlyPlayingNote() < lowestPannedNote && voice->getPan() != 64)
		{
			panner.panValTurnedOff(voice->getPan());
			voice->setPan(64);
		}
	}
};

// audio rendering-----------------------------------------------------------------------------------------------------------------------------------

void Harmonizer::renderNextBlock(AudioBuffer<float>& inputAudio, const int inputChan, int startSample, int numSamples, AudioBuffer<float>& outputBuffer, const MidiBuffer& inputMidi)
{
	jassert (sampleRate > 0);
	
	auto midiIterator = inputMidi.findNextSamplePosition(startSample);
	
	bool firstEvent = true;
	
	const ScopedLock sl (lock);
	
	for (; numSamples > 0; ++midiIterator)
	{
		if (midiIterator == inputMidi.cend())
		{
			renderVoices(inputAudio, inputChan, startSample, numSamples, outputBuffer);
			return;
		}
		
		const auto metadata = *midiIterator;
		const int samplesToNextMidiMessage = metadata.samplePosition - startSample;
		
		if (samplesToNextMidiMessage >= numSamples)
		{
			renderVoices(inputAudio, inputChan, startSample, numSamples, outputBuffer);
			handleMidiEvent(metadata.getMessage());
			break;
		}
		
		if (samplesToNextMidiMessage < ((firstEvent && ! subBlockSubdivisionIsStrict) ? 1 : minimumSubBlockSize))
		{
			handleMidiEvent(metadata.getMessage());
			continue;
		}
		
		firstEvent = false;
		
		renderVoices(inputAudio, inputChan, startSample, samplesToNextMidiMessage, outputBuffer);
		
		handleMidiEvent(metadata.getMessage());
		startSample += samplesToNextMidiMessage;
		numSamples  -= samplesToNextMidiMessage;
	}
	
	std::for_each (midiIterator,
				   inputMidi.cend(),
				   [&] (const MidiMessageMetadata& meta) { handleMidiEvent (meta.getMessage()); });
	
};

void Harmonizer::renderVoices (AudioBuffer<float>& inputAudio, const int inputChan, const int startSample, const int numSamples, AudioBuffer<float>& outputBuffer)
{
	epochIndices = epochs.extractEpochSampleIndices(inputAudio, inputChan, startSample, numSamples, sampleRate);
	currentInputFreq = pitch.findPitch(inputAudio, inputChan, startSample, numSamples, sampleRate);
	
	for (auto* voice : voices)
	{
		voice->updateInputFreq(currentInputFreq);
		voice->renderNextBlock (inputAudio, inputChan, startSample, numSamples, outputBuffer, epochIndices);
	}
};

void Harmonizer::setCurrentPlaybackSampleRate(const double newRate)
{
	jassert(newRate > 0);
	
	if (sampleRate != newRate)
	{
		const ScopedLock sl (lock);
		allNotesOff (false);
		sampleRate = newRate;
		
		for (auto* voice : voices)
			voice->setCurrentPlaybackSamplerate (newRate);
	}
};

void Harmonizer::setMinimumRenderingSubdivisionSize (const int numSamples, const bool shouldBeStrict) noexcept
{
	// Sets a minimum limit on the size to which audio sub-blocks will be divided when rendering. When rendering, the audio blocks that are passed into renderNextBlock() will be split up into smaller blocks that lie between all the incoming midi messages, and it is these smaller sub-blocks that are rendered with multiple calls to renderVoices(). Obviously in a pathological case where there are midi messages on every sample, then renderVoices() could be called once per sample and lead to poor performance, so this setting allows you to set a lower limit on the block size. The default setting is 32, which means that midi messages are accurate to about < 1ms accuracy, which is probably fine for most purposes, but you may want to increase or decrease this value for your synth. If shouldBeStrict is true, the audio sub-blocks will strictly never be smaller than numSamples. If shouldBeStrict is false (default), the first audio sub-block in the buffer is allowed to be smaller, to make sure that the first MIDI event in a buffer will always be sample-accurate (this can sometimes help to avoid quantisation or phasing issues).
	jassert (numSamples > 0); // it wouldn't make much sense for this to be less than 1
	minimumSubBlockSize = numSamples;
	subBlockSubdivisionIsStrict = shouldBeStrict;
};


void Harmonizer::setConcertPitchHz(const int newConcertPitchhz)
{
	jassert(newConcertPitchhz > 0);
	
	const ScopedLock sl (lock);
	
	for(auto* voice : voices)
		voice->setConcertPitch(newConcertPitchhz);
}


// MIDI events---------------------------------------------------------------------------------------------------------------------------------------

void Harmonizer::handleMidiEvent(const MidiMessage& m)
{
	if (m.isNoteOn())
	{
		noteOn (m.getNoteNumber(), m.getFloatVelocity());
	}
	else if (m.isNoteOff())
	{
		noteOff (m.getNoteNumber(), m.getFloatVelocity(), true);
	}
	else if (m.isAllNotesOff() || m.isAllSoundOff())
	{
		allNotesOff (false); 
	}
	else if (m.isPitchWheel())
	{
		const int wheelPos = m.getPitchWheelValue();
		lastPitchWheelValue = wheelPos;
		handlePitchWheel (wheelPos);
	}
	else if (m.isAftertouch())
	{
		handleAftertouch (m.getNoteNumber(), m.getAfterTouchValue());
	}
	else if (m.isChannelPressure())
	{
		handleChannelPressure (m.getChannelPressureValue());
	}
	else if (m.isController())
	{
		handleController (m.getControllerNumber(), m.getControllerValue());
	}
};

void Harmonizer::updateMidiVelocitySensitivity(const int newSensitivity)
{
	const ScopedLock sl (lock);
	
	const float sensitivity = newSensitivity / 100.0f;
	
	for(auto* voice : voices)
		voice->setMidiVelocitySensitivity(sensitivity);
}

Array<int> Harmonizer::reportActiveNotes() const
{
	const ScopedLock sl (lock);
	
	currentlyActiveNotes.clearQuick();
	
	for (auto* voice : voices)
	{
		if (voice->isVoiceActive())
			currentlyActiveNotes.add(voice->getCurrentlyPlayingNote());
	}
	
	if(! currentlyActiveNotes.isEmpty()) { currentlyActiveNotes.sort(); }
	else { currentlyActiveNotes.add(-1); }

	return currentlyActiveNotes;
};

void Harmonizer::noteOn(const int midiPitch, const float velocity)
{
	const ScopedLock sl (lock);
	
	// If hitting a note that's still ringing, stop it first (it could still be playing because of the sustain or sostenuto pedal).
	for (auto* voice : voices)
	{
		if (voice->getCurrentlyPlayingNote() == midiPitch) { stopVoice(voice, 0.5f, true); }
	}
	
	startVoice(findFreeVoice(midiPitch, shouldStealNotes), midiPitch, velocity);
	
};

void Harmonizer::startVoice(HarmonizerVoice* voice, const int midiPitch, const float velocity)
{
	if(voice != nullptr)
	{
		voice->noteOnTime = ++lastNoteOnCounter;
		voice->setKeyDown (true);
		voice->setSostenutoPedalDown (false);
		
		if(! voice->isVoiceActive())
		{
			if(midiPitch >= lowestPannedNote) { voice->setPan(panner.getNextPanVal()); }
			else { voice->setPan(64); } // don't need to report panner.panValTurnedOff() because voice was previously off!
		
			voice->startNote (midiPitch, velocity, lastPitchWheelValue);
		}
		else
		{
			if(midiPitch < lowestPannedNote)
			{
				panner.panValTurnedOff(voice->getPan());
				voice->setPan(64);
			}
			// don't assign a new pan value if midiPitch >= lowestPannedNote -- leave the voice in its place in the stereo field!
			
			voice->changeNote(midiPitch, velocity, lastPitchWheelValue);
		}
	}
};

void Harmonizer::noteOff (const int midiNoteNumber, const float velocity, const bool allowTailOff)
{
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
	{
		if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
		{
			voice->setKeyDown (false);
			if (! (voice->isSustainPedalDown() || voice->isSostenutoPedalDown()))
				stopVoice (voice, velocity, allowTailOff);
		}
	}
};

void Harmonizer::stopVoice (HarmonizerVoice* voice, const float velocity, const bool allowTailOff)
{
	if(voice != nullptr)
	{
		panner.panValTurnedOff(voice->getPan());
		voice->stopNote (velocity, allowTailOff);
	}
};

void Harmonizer::allNotesOff(const bool allowTailOff)
{
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
		if(voice->isVoiceActive())
			voice->stopNote (1.0f, allowTailOff);
	
	panner.reset(false);
};

void Harmonizer::handlePitchWheel(const int wheelValue)
{
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
		voice->pitchWheelMoved (wheelValue);
};

void Harmonizer::updatePitchbendSettings(const int rangeUp, const int rangeDown)
{
	const ScopedLock sl (lock);
	for(auto* voice : voices)
		voice->updatePitchbendSettings(rangeUp, rangeDown);
};

void Harmonizer::handleAftertouch(const int midiNoteNumber, const int aftertouchValue)
{
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
		if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
			voice->aftertouchChanged (aftertouchValue);
};

void Harmonizer::handleChannelPressure(const int channelPressureValue)
{
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
		voice->channelPressureChanged (channelPressureValue);
};

void Harmonizer::handleController(const int controllerNumber, const int controllerValue)
{
	switch (controllerNumber)
	{
		case 0x40:  handleSustainPedal   (controllerValue >= 64); return;
		case 0x42:  handleSostenutoPedal (controllerValue >= 64); return;
		case 0x43:  handleSoftPedal      (controllerValue >= 64); return;
		default:    break;
	}
	
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
		voice->controllerMoved (controllerNumber, controllerValue);
};

void Harmonizer::handleSustainPedal(const bool isDown)
{
	const ScopedLock sl (lock);
	
	if (isDown)
	{
		for (auto* voice : voices)
			voice->setSustainPedalDown (true);
	}
	else
	{
		for (auto* voice : voices)
		{
			voice->setSustainPedalDown (false);
			
			if (! (voice->isKeyDown() || voice->isSostenutoPedalDown()))
				stopVoice (voice, 1.0f, true);
		}
	}
};

void Harmonizer::handleSostenutoPedal(const bool isDown)
{
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
	{
		if (isDown)
			voice->setSostenutoPedalDown (true);
		else if (voice->isSostenutoPedalDown())
			stopVoice (voice, 1.0f, true);
	}
};

void Harmonizer::handleSoftPedal(const bool isDown)
{
	ignoreUnused(isDown);
};


// voice allocation----------------------------------------------------------------------------------------------------------------------------------
HarmonizerVoice* Harmonizer::findFreeVoice (const int midiNoteNumber, const bool stealIfNoneAvailable) const
{
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
		if (! voice->isVoiceActive())
			return voice;
	
	if (stealIfNoneAvailable)
		return findVoiceToSteal (midiNoteNumber);
	
	return nullptr;
};

HarmonizerVoice* Harmonizer::findVoiceToSteal (const int midiNoteNumber) const
{
	// This voice-stealing algorithm applies the following heuristics:
	// - Re-use the oldest notes first
	// - Protect the lowest & topmost notes, even if sustained, but not if they've been released.
	
	jassert (! voices.isEmpty());
	
	// These are the voices we want to protect (ie: only steal if unavoidable)
	HarmonizerVoice* low = nullptr; // Lowest sounding note, might be sustained, but NOT in release phase
	HarmonizerVoice* top = nullptr; // Highest sounding note, might be sustained, but NOT in release phase
	
	// this is a list of voices we can steal, sorted by how long they've been running
	Array<HarmonizerVoice*> usableVoices;
	usableVoices.ensureStorageAllocated (voices.size());
	
	for (auto* voice : voices)
	{
		if(voice->isVoiceActive())
		{
			usableVoices.add (voice);
		
			// NB: Using a functor rather than a lambda here due to scare-stories about compilers generating code containing heap allocations..
			struct Sorter
			{
				bool operator() (const HarmonizerVoice* a, const HarmonizerVoice* b) const noexcept { return a->wasStartedBefore (*b); }
			};
		
			std::sort (usableVoices.begin(), usableVoices.end(), Sorter());
		
			if (! voice->isPlayingButReleased()) // Don't protect released notes
			{
				auto note = voice->getCurrentlyPlayingNote();
				
				if (low == nullptr || note < low->getCurrentlyPlayingNote())
					low = voice;
				
				if (top == nullptr || note > top->getCurrentlyPlayingNote())
					top = voice;
			}
		}
	}
	
	// Eliminate pathological cases (ie: only 1 note playing): we always give precedence to the lowest note(s)
	if (top == low)
		top = nullptr;
	
	// The oldest note that's playing with the target pitch is ideal..
	for (auto* voice : usableVoices)
		if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
			return voice;
	
	// Oldest voice that has been released (no finger on it and not held by sustain pedal)
	for (auto* voice : usableVoices)
		if (voice != low && voice != top && voice->isPlayingButReleased())
			return voice;
	
	// Oldest voice that doesn't have a finger on it:
	for (auto* voice : usableVoices)
		if (voice != low && voice != top && (! voice->isKeyDown()))
			return voice;
	
	// Oldest voice that isn't protected
	for (auto* voice : usableVoices)
		if (voice != low && voice != top)
			return voice;
	
	// Duophonic synth: give priority to the bass note:
	if (top != nullptr)
		return top;
	
	return low;
};


// update ADSR settings------------------------------------------------------------------------------------------------------------------------------
void Harmonizer::updateADSRsettings(const float attack, const float decay, const float sustain, const float release)
{
	// attack/decay/release time in SECONDS; sustain ratio 0.0 - 1.0
	
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
		voice->updateAdsrSettings(attack, decay, sustain, release);
};

void Harmonizer::setADSRonOff(const bool shouldBeOn)
{
	const ScopedLock sl (lock);
	
	for(auto* voice : voices)
		voice->setAdsrOnOff(shouldBeOn);
};

void Harmonizer::updateQuickReleaseMs(const int newMs)
{
	jassert(newMs > 0);
	
	const ScopedLock sl (lock);
	
	for(auto* voice : voices)
		voice->setQuickReleaseMs(newMs);
}


// functions for management of HarmonizerVoices------------------------------------------------------------------------------------------------------
HarmonizerVoice* Harmonizer::addVoice(HarmonizerVoice* newVoice)
{
	const ScopedLock sl (lock);
	
	panner.setNumberOfVoices(voices.size() + 1);
	
	newVoice->setCurrentPlaybackSamplerate(sampleRate);
	return voices.add(newVoice);
};

void Harmonizer::removeVoice(const int index)
{
	const ScopedLock sl (lock);
	voices.remove(index);
	
	if(voices.size() > 0) { panner.setNumberOfVoices(voices.size()); }
};

HarmonizerVoice* Harmonizer::getVoice(const int index) const
{
	const ScopedLock sl (lock);
	return voices[index];
};

void Harmonizer::deleteAllVoices()
{
	const ScopedLock sl (lock);
	voices.clear();
	panner.setNumberOfVoices(1);  // panner's numVoices must be >0
};


void Harmonizer::removeNumVoices(const int voicesToRemove)
{
	const ScopedLock sl (lock);
	
	int voicesRemoved = 0;
	while(voicesRemoved < voicesToRemove)
	{
		int indexToRemove = -1;
		for(auto* voice : voices)
		{
			if(! voice->isVoiceActive())
			{
				indexToRemove = voices.indexOf(voice);
				break;
			}
		}
		
		if(indexToRemove > -1)
			voices.remove(indexToRemove);
		else
			voices.remove(0);
		
		++voicesRemoved;
	}
};
