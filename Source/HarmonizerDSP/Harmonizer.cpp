/*
  ==============================================================================

    Harmonizer.cpp
    Created: 13 Dec 2020 7:53:39pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "Harmonizer.h"


HarmonizerVoice::HarmonizerVoice(Harmonizer* h): parent(h), isFading(false), noteTurnedOff(true), currentlyPlayingNote(-1), currentOutputFreq(-1.0f), currentVelocityMultiplier(0.0f), lastRecievedVelocity(0.0f), noteOnTime(0), keyIsDown(false), currentMidipan(64)
{
	panningMults[0] = 0.5f;
	panningMults[1] = 0.5f;
	
	adsr.setSampleRate(44100.0);
	quickRelease.setSampleRate(44100.0);

	ADSR::Parameters initParams;
	initParams.attack = 0.035f;
	initParams.decay = 0.06f;
	initParams.sustain = 0.8f;
	initParams.release = 0.01f;
	adsr.setParameters(initParams);
	
	ADSR::Parameters qrinit;
	qrinit.attack = 0.01f;
	qrinit.decay = 0.01f;
	qrinit.sustain = 1.0f;
	qrinit.release = 0.015f;
	quickRelease.setParameters(qrinit);
	
	tempBuffer.setSize(1, MAX_BUFFERSIZE);
};


HarmonizerVoice::~HarmonizerVoice()
{ };


void HarmonizerVoice::renderNextBlock(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices)
{
	if(! (parent->sustainPedalDown || parent->sostenutoPedalDown))
		if(! keyIsDown)
			stopNote(1.0f, false);
	
	// don't want to just use the ADSR to tell if the voice is currently active, bc if the user has turned the ADSR off, the voice would remain active for the release phase of the ADSR...
	bool voiceIsOnRightNow;
	if(parent->adsrIsOn)
		voiceIsOnRightNow = adsr.isActive();
	else
		voiceIsOnRightNow = isFading ? true : !noteTurnedOff;
	
	if(voiceIsOnRightNow)
	{
		// puts shifted samples into the tempBuffer
		esola(inputAudio, inputChan, numSamples, tempBuffer, epochIndices,
			  	(1.0f / (1.0f + ((parent->currentInputFreq - currentOutputFreq)/currentOutputFreq))) ); // shifting ratio
		
		AudioBuffer<float> subBuffer(outputBuffer.getArrayOfWritePointers(), 2, 0, numSamples);
		
		subBuffer.addFrom(0, 0, tempBuffer, 0, 0, numSamples, panningMults[0]);
		subBuffer.addFrom(1, 0, tempBuffer, 0, 0, numSamples, panningMults[1]);
		
		subBuffer.applyGain(0, numSamples, currentVelocityMultiplier);
	
		if(parent->adsrIsOn) // only apply the envelope if the ADSR on/off user toggle is ON
			adsr.applyEnvelopeToBuffer(subBuffer, 0, numSamples);
		
		// quick fade out for stopNote() w/ allowTailOff = false:
		if(isFading)
		{
			if(! quickRelease.isActive()) { quickRelease.noteOn(); quickRelease.noteOff(); }  // ??
			quickRelease.applyEnvelopeToBuffer(subBuffer, 0, numSamples);
			adsr.reset();
			isFading = false;
			clearCurrentNote();
		}
	}
	else
	{
		clearCurrentNote();
		parent->panner.panValTurnedOff(currentMidipan);
		currentMidipan = 64;
		quickRelease.noteOn();
		adsr.reset();
	}
};


// MIDI -----------------------------------------------------------------------------------------------------------

void HarmonizerVoice::startNote(const int midiPitch, const float velocity)
{
	currentlyPlayingNote = midiPitch;
	lastRecievedVelocity = velocity;
	currentOutputFreq = parent->pitchConverter.mtof(parent->bendTracker.newNoteRecieved(midiPitch));
	currentVelocityMultiplier = parent->velocityConverter.floatVelocity(velocity);
	adsr.noteOn();
	if(! quickRelease.isActive()) { quickRelease.noteOn(); }
	isFading = false;
	noteTurnedOff = false;
};

void HarmonizerVoice::changeNote(const int midiPitch, const float velocity)
{
	currentlyPlayingNote = midiPitch;
	lastRecievedVelocity = velocity;
	currentOutputFreq = parent->pitchConverter.mtof(parent->bendTracker.newNoteRecieved(midiPitch));
	currentVelocityMultiplier = parent->velocityConverter.floatVelocity(velocity);
	if(! adsr.isActive()) { adsr.noteOn(); }
	if(! quickRelease.isActive()) { quickRelease.noteOn(); }
	isFading = false;
	noteTurnedOff = false;
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
		isFading = true;
		quickRelease.noteOff();
	}
	lastRecievedVelocity = 0.0f;
	noteTurnedOff = true;
};

void HarmonizerVoice::aftertouchChanged(const int) { };

void HarmonizerVoice::channelPressureChanged(const int) { };

void HarmonizerVoice::controllerMoved(const int controllerNumber, const int newControllerValue) { };


void HarmonizerVoice::setPan(const int newPan) 
{
	jassert(isPositiveAndBelow(newPan, 128));
	
	if(currentMidipan != newPan)
	{
		if(newPan == 64) // save time for the simplest case
		{
			panningMults[0] = 0.5f;
			panningMults[1] = 0.5f;
		}
		else if(currentMidipan != newPan)
		{
			const float Rpan = newPan / 127.0f;
			panningMults[1] = Rpan; // R channel
			panningMults[0] = 1.0f - Rpan; // L channel
		}
		currentMidipan = newPan;
	}
};


bool HarmonizerVoice::isPlayingButReleased() const noexcept
{
	return isVoiceActive() && ! (keyIsDown || parent->sostenutoPedalDown || parent->sustainPedalDown);
};


void HarmonizerVoice::esola(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices, const float shiftingRatio)
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

Harmonizer::Harmonizer(): lastPitchWheelValue(64), pitchConverter(440, 69, 12), bendTracker(2, 2), velocityConverter(100), latchIsOn(false), latchManager(MAX_POSSIBLE_NUMBER_OF_VOICES), adsrIsOn(true), currentInputFreq(0.0f), sampleRate(44100.0), shouldStealNotes(true), lastNoteOnCounter(0), lowestPannedNote(0), sustainPedalDown(false), sostenutoPedalDown(false)
{
	currentlyActiveNotes.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	currentlyActiveNotes.clearQuick();
	currentlyActiveNotes.add(-1);
	
	currentlyActiveNoReleased.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	currentlyActiveNoReleased.clearQuick();
	currentlyActiveNoReleased.add(-1);
	
	desired.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	previous.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	unLatched.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	
	adsrParams.attack = 0.035f;
	adsrParams.decay = 0.06f;
	adsrParams.sustain = 0.8f;
	adsrParams.release = 0.01f;
	
	quickReleaseParams.attack = 0.01f;
	quickReleaseParams.decay = 0.005f;
	quickReleaseParams.sustain = 1.0f;
	quickReleaseParams.release = 0.015f;
};

Harmonizer::~Harmonizer()
{
	voices.clear();
};


// audio rendering-----------------------------------------------------------------------------------------------------------------------------------

void Harmonizer::renderVoices (AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices)
{
	int actives = 0;
	for (auto* voice : voices)
	{
		voice->renderNextBlock (inputAudio, inputChan, numSamples, outputBuffer, epochIndices);
		++actives;
	}
	if(actives > 0)
	{
		const float gain = 1.0f / actives;
		outputBuffer.applyGain(0, 0, numSamples, gain);
		outputBuffer.applyGain(1, 0, numSamples, gain);
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
		{
			voice->adsr.setSampleRate(newRate);
			voice->quickRelease.setSampleRate(newRate);
			voice->adsr.setParameters(adsrParams);
			voice->quickRelease.setParameters(quickReleaseParams);
		}
	}
};

void Harmonizer::setConcertPitchHz(const int newConcertPitchhz)
{
	jassert(newConcertPitchhz > 0);
	
	if(pitchConverter.getCurrentConcertPitchHz() != newConcertPitchhz)
	{
		const ScopedLock sl (lock);
		
		pitchConverter.setConcertPitchHz(newConcertPitchhz);
		
		for(auto* voice : voices)
			if(voice->isVoiceActive())
				voice->currentOutputFreq = pitchConverter.mtof(bendTracker.newNoteRecieved(voice->currentlyPlayingNote));
	}
};


// stereo width -------------------------------------------------------------------------------------------------------------------------------------

void Harmonizer::updateStereoWidth(const int newWidth)
{
	if(panner.getCurrentStereoWidth() != newWidth)
	{
		const ScopedLock sl (lock);
		
		panner.updateStereoWidth(newWidth);
		
		for (auto* voice : voices)
		{
			if(voice->isVoiceActive() && voice->getCurrentlyPlayingNote() >= lowestPannedNote)
			{
				voice->setPan(panner.getClosestNewPanValFromOld(voice->currentMidipan));
				continue;
			}
			if(voice->isVoiceActive() && voice->getCurrentlyPlayingNote() < lowestPannedNote && voice->currentMidipan != 64)
			{
				panner.panValTurnedOff(voice->currentMidipan);
				voice->setPan(64);
			}
		}
	}
};

void Harmonizer::updateLowestPannedNote(const int newPitchThresh) noexcept
{
	if(lowestPannedNote != newPitchThresh)
	{
		const ScopedLock sl (lock);
		lowestPannedNote = newPitchThresh;
		for(auto* voice : voices)
		{
			if(voice->isVoiceActive() && voice->currentlyPlayingNote < newPitchThresh && voice->currentMidipan != 64)
			{
				panner.panValTurnedOff(voice->currentMidipan);
				voice->setPan(64);
			}
		}
	}
};


// MIDI events --------------------------------------------------------------------------------------------------------------------------------------

Array<int> Harmonizer::reportActiveNotes() const
{
	currentlyActiveNotes.clearQuick();
	
	for (auto* voice : voices)
		if (voice->isVoiceActive())
			currentlyActiveNotes.add(voice->getCurrentlyPlayingNote());
	
	if(! currentlyActiveNotes.isEmpty()) { currentlyActiveNotes.sort(); }
	else { currentlyActiveNotes.add(-1); }
	
	return currentlyActiveNotes;
};

void Harmonizer::setMidiLatch(const bool shouldBeOn, const bool allowTailOff)
{
	latchIsOn = shouldBeOn;
	
	if(! shouldBeOn)
	{
		unLatched = latchManager.turnOffLatch();
		if(! unLatched.isEmpty())
		{
			const float offVelocity = allowTailOff ? 0.0f : 1.0f;
			turnOffList(unLatched, offVelocity, allowTailOff);
		}
	}
};

void Harmonizer::updateMidiVelocitySensitivity(const int newSensitivity)
{
	if(const float newSens = newSensitivity/100.0f; velocityConverter.getCurrentSensitivity() != newSens)
	{
		const ScopedLock sl (lock);
		velocityConverter.setFloatSensitivity(newSens);
		for(auto* voice : voices)
			if(voice->isVoiceActive())
				voice->currentVelocityMultiplier = velocityConverter.floatVelocity(voice->lastRecievedVelocity);
	}
};


// functions for triggering chords ------------------------------------------------------------------------

void Harmonizer::playChord(Array<int>& chordNotes, const int velocity, const bool allowTailOffOfOld)
{
	desired = chordNotes;
	previous = reportActivesNoReleased();
	
	float offVelocity = allowTailOffOfOld ? 0.0f : 1.0f;
	
	if(previous.getUnchecked(0) != -1)
	{
		for(int i = 0; i < chordNotes.size(); ++i)
			if(const int note = chordNotes.getUnchecked(i); previous.contains(note))
			   desired.remove(desired.indexOf(note)); // what's left in desired will be the notes that need to be turned on
		
		for(int i = 0; i < previous.size(); ++i)
			if(chordNotes.contains(previous.getUnchecked(i)))
				previous.remove(i); // what's left in previous will be the notes that need to be turned off
	}
	
	previous.removeAllInstancesOf(-1);
	
	if(! desired.isEmpty())
		turnOnList(desired, velocity);
	
	if(! previous.isEmpty())
		turnOffList(previous, offVelocity, allowTailOffOfOld);
};

void Harmonizer::turnOnList(Array<int>& toTurnOn, const int velocity)
{
	if(! toTurnOn.isEmpty())
	{
		const ScopedLock sl (lock);
		const float floatVelocity = velocity / 127.0f;
		for(int i = 0; i < toTurnOn.size(); ++i)
			noteOn(toTurnOn.getUnchecked(i), floatVelocity);
	}
};

void Harmonizer::turnOffList(Array<int>& toTurnOff, const float velocity, const bool allowTailOff)
{
	if(! toTurnOff.isEmpty())
	{
		const ScopedLock sl (lock);
		for(int i = 0; i < toTurnOff.size(); ++i)
		{
			noteOff(toTurnOff.getUnchecked(i), velocity, allowTailOff);
		}
	}
};

Array<int> Harmonizer::reportActivesNoReleased() const
{
	currentlyActiveNoReleased.clearQuick();
	
	for (auto* voice : voices)
		if (voice->isVoiceActive() && !(voice->isPlayingButReleased()))
			currentlyActiveNoReleased.add(voice->getCurrentlyPlayingNote());
	
	if(! currentlyActiveNoReleased.isEmpty()) { currentlyActiveNoReleased.sort(); }
	else { currentlyActiveNoReleased.add(-1); }
	
	return currentlyActiveNoReleased;
};


// functions for propogating midi events to HarmonizerVoices -------------------------------------------------------

void Harmonizer::handleMidiEvent(const MidiMessage& m)
{
	if (m.isNoteOn())
		noteOn (m.getNoteNumber(), m.getFloatVelocity());
	else if (m.isNoteOff())
		noteOff (m.getNoteNumber(), m.getFloatVelocity(), true);
	else if (m.isAllNotesOff() || m.isAllSoundOff())
		allNotesOff (false);
	else if (m.isPitchWheel())
	{
		const int wheelPos = m.getPitchWheelValue();
		lastPitchWheelValue = wheelPos;
		handlePitchWheel (wheelPos);
	}
	else if (m.isAftertouch())
		handleAftertouch (m.getNoteNumber(), m.getAfterTouchValue());
	else if (m.isChannelPressure())
		handleChannelPressure (m.getChannelPressureValue());
	else if (m.isController())
		handleController (m.getControllerNumber(), m.getControllerValue());
};

void Harmonizer::noteOn(const int midiPitch, const float velocity)
{
	const ScopedLock sl (lock);
	
	// If hitting a note that's still ringing, stop it first (it could still be playing because of the sustain or sostenuto pedal).
	for (auto* voice : voices)
		if (voice->getCurrentlyPlayingNote() == midiPitch) { stopVoice(voice, 0.5f, true); }
	
	startVoice(findFreeVoice(midiPitch, shouldStealNotes), midiPitch, velocity);
	
	if(latchIsOn)
		latchManager.noteOnRecieved(midiPitch);
};

void Harmonizer::startVoice(HarmonizerVoice* voice, const int midiPitch, const float velocity)
{
	if(voice != nullptr)
	{
		voice->noteOnTime = ++lastNoteOnCounter;
		voice->setKeyDown (true);
		
		if(! voice->isVoiceActive())
		{
			if(midiPitch >= lowestPannedNote) { voice->setPan(panner.getNextPanVal()); }
			else { voice->setPan(64); } // don't need to report panner.panValTurnedOff() because voice was previously off!
		
			voice->startNote (midiPitch, velocity);
		}
		else
		{
			if(midiPitch < lowestPannedNote)
			{
				panner.panValTurnedOff(voice->currentMidipan);
				voice->setPan(64);
			}
			// don't assign a new pan value if midiPitch >= lowestPannedNote -- leave the voice in its place in the stereo field!
			
			voice->changeNote(midiPitch, velocity);
		}
	}
};

void Harmonizer::noteOff (const int midiNoteNumber, const float velocity, const bool allowTailOff)
{
	const ScopedLock sl (lock);
	
	if(latchIsOn)
		latchManager.noteOffRecieved(midiNoteNumber);
	else
	{
		for (auto* voice : voices)
		{
			if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
			{
				voice->setKeyDown (false);
				if (! (sustainPedalDown || sostenutoPedalDown))
					stopVoice (voice, velocity, allowTailOff);
			}
		}
	}
};

void Harmonizer::stopVoice (HarmonizerVoice* voice, const float velocity, const bool allowTailOff)
{
	if(voice != nullptr)
	{
		voice->stopNote (velocity, allowTailOff);
		if(! allowTailOff)
			panner.panValTurnedOff(voice->currentMidipan);
	}
};

void Harmonizer::allNotesOff(const bool allowTailOff)
{
	const ScopedLock sl (lock);
	
	for (auto* voice : voices)
		if(voice->isVoiceActive())
			voice->stopNote (1.0f, allowTailOff);
	
	panner.reset(false);
	latchManager.reset();
};

void Harmonizer::handlePitchWheel(const int wheelValue)
{
	if(lastPitchWheelValue != wheelValue)
	{
		const ScopedLock sl (lock);
		
		lastPitchWheelValue = wheelValue;
		
		bendTracker.newPitchbendRecieved(wheelValue);
		for (auto* voice : voices)
			if(voice->isVoiceActive())
				voice->currentOutputFreq = pitchConverter.mtof(bendTracker.newNoteRecieved(voice->currentlyPlayingNote));
	}
};

void Harmonizer::updatePitchbendSettings(const int rangeUp, const int rangeDown)
{
	if(bendTracker.getCurrentRangeUp() != rangeUp || bendTracker.getCurrentRangeDown() != rangeDown)
	{
		bendTracker.setRange(rangeUp, rangeDown);
		if(lastPitchWheelValue != 64)
		{
			const ScopedLock sl (lock);
			for(auto* voice : voices)
				if(voice->isVoiceActive())
					voice->currentOutputFreq = pitchConverter.mtof(bendTracker.newNoteRecieved(voice->currentlyPlayingNote));
		}
	}
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
	
	sustainPedalDown = isDown;
	
	if(! isDown)
	{
		for (auto* voice : voices)
		{
			if (! (voice->isKeyDown() || ! sustainPedalDown))
				stopVoice (voice, 1.0f, true);
		}
	}
};

void Harmonizer::handleSostenutoPedal(const bool isDown)
{
	const ScopedLock sl (lock);
	
	sostenutoPedalDown = isDown;
	
	for (auto* voice : voices)
	{
		if (! isDown && sostenutoPedalDown)
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

	if(adsrParams.attack != attack || adsrParams.decay != decay || adsrParams.sustain != sustain || adsrParams.release != release)
	{
		const ScopedLock sl (lock);
		
		adsrParams.attack = attack;
		adsrParams.decay = decay;
		adsrParams.sustain = sustain;
		adsrParams.release = release;
	
		for (auto* voice : voices)
			voice->adsr.setParameters(adsrParams);
	}
};

void Harmonizer::updateQuickReleaseMs(const int newMs)
{
	jassert(newMs > 0);
	
	if(const float desiredR = newMs / 1000.0f; quickReleaseParams.release != desiredR)
	{
		const ScopedLock sl (lock);
		
		quickReleaseParams.release = desiredR;
		for(auto* voice : voices)
			voice->quickRelease.setParameters(quickReleaseParams);
	}
};


// functions for management of HarmonizerVoices------------------------------------------------------------------------------------------------------

HarmonizerVoice* Harmonizer::addVoice(HarmonizerVoice* newVoice)
{
	const ScopedLock sl (lock);
	
	panner.setNumberOfVoices(voices.size() + 1);
	
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
