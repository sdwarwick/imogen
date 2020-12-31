/*
  ==============================================================================

    Harmonizer.cpp
    Created: 13 Dec 2020 7:53:39pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "Harmonizer.h"


HarmonizerVoice::HarmonizerVoice(Harmonizer* h): parent(h), isFading(false), noteTurnedOff(true), currentlyPlayingNote(-1), currentOutputFreq(-1.0f), currentVelocityMultiplier(0.0f), lastRecievedVelocity(0.0f), noteOnTime(0), keyIsDown(false), currentMidipan(64), currentAftertouch(64)
{
	panningMults[0] = 0.5f;
	panningMults[1] = 0.5f;
	
	const double initSamplerate = parent->getSamplerate();
	
	adsr.setSampleRate			(initSamplerate);
	quickRelease.setSampleRate	(initSamplerate);
	quickAttack.setSampleRate	(initSamplerate);

	adsr.setParameters			(parent->getCurrentAdsrParams());
	quickRelease.setParameters	(parent->getCurrentQuickReleaseParams());
	quickAttack.setParameters	(parent->getCurrentQuickAttackParams());
	
	tempBuffer.setSize(1, MAX_BUFFERSIZE);
};


HarmonizerVoice::~HarmonizerVoice()
{ };


void HarmonizerVoice::renderNextBlock(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices)
{
	if(! (parent->sustainPedalDown || parent->sostenutoPedalDown) && !keyIsDown)
		stopNote(1.0f, false);
	
	// don't want to just use the ADSR to tell if the voice is currently active, bc if the user has turned the ADSR off, the voice would remain active for the release phase of the ADSR...
	bool voiceIsOnRightNow;
	if(!isFading && parent->adsrIsOn)
		voiceIsOnRightNow = adsr.isActive();
	else
		voiceIsOnRightNow = isFading ? quickRelease.isActive() : !noteTurnedOff;
	
	if(voiceIsOnRightNow)
	{
		tempBuffer.clear();
		
		// puts shifted samples into the tempBuffer
		esola(inputAudio, inputChan, numSamples, tempBuffer, epochIndices,
			  	(1.0f / (1.0f + ((parent->currentInputFreq - currentOutputFreq)/currentOutputFreq))) ); // shifting ratio
		
		tempBuffer.applyGain(0, 0, numSamples, currentVelocityMultiplier);
		
		AudioBuffer<float> subBuffer(outputBuffer.getArrayOfWritePointers(), 2, 0, numSamples);
		
		subBuffer.addFrom(0, 0, tempBuffer, 0, 0, numSamples, panningMults[0]);
		subBuffer.addFrom(1, 0, tempBuffer, 0, 0, numSamples, panningMults[1]);
		
		if(parent->adsrIsOn) // only apply the envelope if the ADSR on/off user toggle is ON
			adsr.applyEnvelopeToBuffer(subBuffer, 0, numSamples);
		else
			quickAttack.applyEnvelopeToBuffer(subBuffer, 0, numSamples); // to prevent pops at start of notes
		
		if(isFading) // quick fade out for stopNote() w/ allowTailOff = false:
			quickRelease.applyEnvelopeToBuffer(subBuffer, 0, numSamples);
	}
	else
		clearCurrentNote();
};


// this function is called to reset the HarmonizerVoice's internal state to neutral / initial
void HarmonizerVoice::clearCurrentNote() 
{
	currentlyPlayingNote = -1;
	setPan(64);
	lastRecievedVelocity 	  = 0.0f;
	currentVelocityMultiplier = 0.0f;
	noteOnTime = 0;
	isFading 	  = false;
	noteTurnedOff = true;
	keyIsDown 	  = false;
	
	if(! quickRelease.isActive())
		quickRelease.noteOn();
	
	if(adsr.isActive())
		adsr.reset();
	
	if(quickAttack.isActive())
		quickAttack.reset();
};


// MIDI -----------------------------------------------------------------------------------------------------------

void HarmonizerVoice::startNote(const int midiPitch, const float velocity)
{
	currentlyPlayingNote = midiPitch;
	lastRecievedVelocity = velocity;
	currentOutputFreq = parent->pitchConverter.mtof(parent->bendTracker.newNoteRecieved(midiPitch));
	currentVelocityMultiplier = parent->velocityConverter.floatVelocity(velocity);
	isFading 	  = false;
	noteTurnedOff = false;
	
	adsr.noteOn();
	quickAttack.noteOn();
	if(! quickRelease.isActive())
		quickRelease.noteOn();
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
		isFading = true;
		quickRelease.noteOff();
		if(! quickRelease.isActive())
			quickRelease.noteOn();
	}
	lastRecievedVelocity = 0.0f;
	noteTurnedOff = true;
};

void HarmonizerVoice::aftertouchChanged(const int newAftertouchValue)
{
	currentAftertouch = newAftertouchValue;
};


void HarmonizerVoice::setPan(const int newPan) 
{
	jassert(isPositiveAndBelow(newPan, 128));
	
	if(currentMidipan != newPan)
	{
		parent->panner.panValTurnedOff(currentMidipan);
		if(newPan == 64) // save time for the simplest case
		{
			panningMults[0] = 0.5f;
			panningMults[1] = 0.5f;
		}
		else
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


void HarmonizerVoice::updateSampleRate(const double newSamplerate)
{
	adsr.setSampleRate	      (newSamplerate);
	quickRelease.setSampleRate(newSamplerate);
	quickAttack.setSampleRate (newSamplerate);
	
	adsr.setParameters		  (parent->getCurrentAdsrParams());
	quickRelease.setParameters(parent->getCurrentQuickReleaseParams());
	quickAttack.setParameters (parent->getCurrentQuickAttackParams());
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Harmonizer::Harmonizer(): lastPitchWheelValue(64), pitchConverter(440, 69, 12), bendTracker(2, 2), velocityConverter(100), latchIsOn(false), latchManager(MAX_POSSIBLE_NUMBER_OF_VOICES), adsrIsOn(true), currentInputFreq(0.0f), sampleRate(44100.0), shouldStealNotes(true), lastNoteOnCounter(0), lowestPannedNote(0), sustainPedalDown(false), sostenutoPedalDown(false), pedalPitchIsOn(false), lastPedalPitch(-1), pedalPitchUpperThresh(0), pedalPitchInterval(12), descantIsOn(false), lastDescantPitch(-1), descantLowerThresh(127), descantInterval(12)
{
	currentlyActiveNotes.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	currentlyActiveNotes.clearQuick();
	currentlyActiveNotes.add(-1);
	
	currentlyActiveNoReleased.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	currentlyActiveNoReleased.clearQuick();
	currentlyActiveNoReleased.add(-1);
	
	unLatched.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	latching.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	
	adsrParams.attack  = 0.035f;
	adsrParams.decay   = 0.06f;
	adsrParams.sustain = 0.8f;
	adsrParams.release = 0.01f;
	
	quickReleaseParams.attack  = 0.01f;
	quickReleaseParams.decay   = 0.005f;
	quickReleaseParams.sustain = 1.0f;
	quickReleaseParams.release = 0.015f;
	
	quickAttackParams.attack  = 0.015f;
	quickAttackParams.decay   = 0.01f;
	quickAttackParams.sustain = 1.0f;
	quickAttackParams.release = 0.015f;
};

Harmonizer::~Harmonizer()
{
	voices.clear();
};


// audio rendering-----------------------------------------------------------------------------------------------------------------------------------

void Harmonizer::renderVoices (AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices)
{
	for (auto* voice : voices)
		if(voice->isVoiceActive())
			voice->renderNextBlock (inputAudio, inputChan, numSamples, outputBuffer, epochIndices);
};

int Harmonizer::getNumActiveVoices()
{
	int actives = 0;
	for(auto* voice : voices)
		if(voice->isVoiceActive())
			++actives;
	
	return actives;
};

bool Harmonizer::isPitchActive(const int midiPitch, const bool countRingingButReleased)
{
	for(auto* voice : voices)
	{
		if(voice->isVoiceActive() && voice->currentlyPlayingNote == midiPitch)
		{
			if(countRingingButReleased)
				return true;
			else
				if(! voice->isPlayingButReleased())
					return true;
		}
	}
	return false;
};

void Harmonizer::setCurrentPlaybackSampleRate(const double newRate)
{
	jassert(newRate > 0);
	
	if (sampleRate != newRate)
	{
		const ScopedLock sl (lock);
		
		allNotesOff (false); // ??
		sampleRate = newRate;
		
		for (auto* voice : voices)
			voice->updateSampleRate(newRate);
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
	jassert(isPositiveAndBelow(newWidth, 101));
	
	if(panner.getCurrentStereoWidth() != newWidth)
	{
		const ScopedLock sl (lock);
		
		panner.updateStereoWidth(newWidth);
		
		for (auto* voice : voices)
		{
			if(voice->isVoiceActive())
			{
				if(voice->getCurrentlyPlayingNote() >= lowestPannedNote)
				{
					voice->setPan(panner.getClosestNewPanValFromOld(voice->currentMidipan));
					continue;
				}
				else if(voice->currentMidipan != 64)
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
			if(voice->isVoiceActive())
			{
				if(voice->currentlyPlayingNote < newPitchThresh && voice->currentMidipan != 64)
		 		{
					voice->setPan(64);
					continue;
				}
				else if(voice->currentlyPlayingNote >= newPitchThresh && voice->currentMidipan == 64)
					voice->setPan(panner.getNextPanVal());
			}
		}
	}
};


// MIDI events --------------------------------------------------------------------------------------------------------------------------------------


// this function is called any time the harmonizer's overall pitch collection is changed. With standard keyboard input, this would be once every note event; for chord triggering, this should be called once after the new chord note on/off events have been handled.
void Harmonizer::pitchCollectionChanged()
{
	if(pedalPitchIsOn)
		applyPedalPitch();
	
	if(descantIsOn)
		applyDescant();
};

// report active notes --------------------------------------------------------

Array<int> Harmonizer::reportActiveNotes() const
{
	currentlyActiveNotes.clearQuick();

	for (auto* voice : voices)
		if (voice->isVoiceActive())
			currentlyActiveNotes.add(voice->getCurrentlyPlayingNote());

	if(! currentlyActiveNotes.isEmpty())
		currentlyActiveNotes.sort();

	return currentlyActiveNotes;
};

Array<int> Harmonizer::reportActivesNoReleased() const
{
	currentlyActiveNoReleased.clearQuick();

	for (auto* voice : voices)
		if (voice->isVoiceActive() && !(voice->isPlayingButReleased()))
			currentlyActiveNoReleased.add(voice->currentlyPlayingNote);

	if(! currentlyActiveNoReleased.isEmpty())
		currentlyActiveNoReleased.sort();

	return currentlyActiveNoReleased;
};


// midi velocity sensitivity ---------------------------------------------------

void Harmonizer::updateMidiVelocitySensitivity(const int newSensitivity)
{
	const float newSens = newSensitivity/100.0f;
	
	if(velocityConverter.getCurrentSensitivity() != newSens)
	{
		const ScopedLock sl (lock);
		velocityConverter.setFloatSensitivity(newSens);
		
		for(auto* voice : voices)
			if(voice->isVoiceActive())
				voice->currentVelocityMultiplier = velocityConverter.floatVelocity(voice->lastRecievedVelocity);
	}
};


// midi latch ------------------------------------------------------------------

void Harmonizer::setMidiLatch(const bool shouldBeOn, const bool allowTailOff)
{
	if(latchIsOn != shouldBeOn)
	{
		latchIsOn = shouldBeOn;
	
		if(! shouldBeOn)
		{
			unLatched.clearQuick();
			unLatched = latchManager.turnOffLatch();
			latchManager.reset();
			if(! unLatched.isEmpty())
			{
				const float offVelocity = allowTailOff ? 0.0f : 1.0f;
				turnOffList(unLatched, offVelocity, allowTailOff);
				pitchCollectionChanged();
			}
		}
		else
		{
			latchManager.reset();
			latching.clearQuick();
			latching = reportActivesNoReleased();
			if(! latching.isEmpty())
			{
				for(int i = 0; i < latching.size(); ++i)
					latchManager.noteOnRecieved(latching.getUnchecked(i));
			}
		}
	}
};

void Harmonizer::turnOffList(Array<int>& toTurnOff, const float velocity, const bool allowTailOff)
{
	if(! toTurnOff.isEmpty())
	{
		const ScopedLock sl (lock);
		for(int i = 0; i < toTurnOff.size(); ++i)
			noteOff(toTurnOff.getUnchecked(i), velocity, allowTailOff, true, false);
	}
};


// functions for propogating midi events to HarmonizerVoices -----------------------------------------------------------

void Harmonizer::handleMidiEvent(const MidiMessage& m)
{
	if (m.isNoteOn())
		noteOn (m.getNoteNumber(), m.getFloatVelocity(), true);
	else if (m.isNoteOff())
		noteOff (m.getNoteNumber(), m.getFloatVelocity(), true, false, true);
	else if (m.isAllNotesOff() || m.isAllSoundOff())
		allNotesOff (false);
	else if (m.isPitchWheel())
		handlePitchWheel (m.getPitchWheelValue());
	else if (m.isAftertouch())
		handleAftertouch (m.getNoteNumber(), m.getAfterTouchValue());
	else if (m.isChannelPressure())
		handleChannelPressure (m.getChannelPressureValue());
	else if (m.isController())
		handleController (m.getControllerNumber(), m.getControllerValue());
};

void Harmonizer::noteOn(const int midiPitch, const float velocity, const bool isKeyboard)
{
	// N.B. the `isKeyboard` flag should be true if this note on event was triggered directly from the midi keyboard input; this flag is false if this note on event was triggered automatically by pedal pitch or descant.
	
	const ScopedLock sl (lock);
	
	// If hitting a note that's still ringing, stop it first (it could still be playing because of the sustain or sostenuto pedal).
	for (auto* voice : voices)
		if (voice->getCurrentlyPlayingNote() == midiPitch)
			stopVoice(voice, 0.5f, true);
	
	startVoice(findFreeVoice(midiPitch, shouldStealNotes), midiPitch, velocity, isKeyboard);
	
	if(isKeyboard)
	{
		if(latchIsOn)
			latchManager.noteOnRecieved(midiPitch);
		pitchCollectionChanged();
	}
};

void Harmonizer::startVoice(HarmonizerVoice* voice, const int midiPitch, const float velocity, const bool isKeyboard)
{
	if(voice != nullptr)
	{
		voice->noteOnTime = ++lastNoteOnCounter;
		
		if(! voice->isKeyDown())
			voice->setKeyDown (isKeyboard);
		
		if(midiPitch < lowestPannedNote)
			voice->setPan(64);
		else if(! voice->isVoiceActive())
			voice->setPan(panner.getNextPanVal());
		
		voice->startNote (midiPitch, velocity);
	}
};

void Harmonizer::noteOff (const int midiNoteNumber, const float velocity, const bool allowTailOff, const bool partofList, const bool isKeyboard)
{
	// N.B. the `isKeyboard` flag should be true if this note off event was triggered directly from the midi keyboard input; this flag is false if this note off event was triggered automatically by pedal pitch or descant.
	// N.B. the `partofList` flag should be false if this is a singular note off event; this flag should be true if this is a note off event in a known sequence of many quick note offs, so that pedal pitch & descant will not be updated after every single one of these events.
	
	const ScopedLock sl (lock);
	
	if(latchIsOn && isKeyboard)
		latchManager.noteOffRecieved(midiNoteNumber);
	else
	{
		for (auto* voice : voices)
		{
			if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
			{
				if(! isKeyboard)
				{
					if(! voice->isKeyDown())
						stopVoice (voice, velocity, allowTailOff);
				}
				else
				{
					voice->setKeyDown (false);
					if (! (sustainPedalDown || sostenutoPedalDown))
						stopVoice (voice, velocity, allowTailOff);
				}
			}
		}
		
		if(midiNoteNumber == lastPedalPitch)
			lastPedalPitch = -1;
		if(midiNoteNumber == lastDescantPitch)
			lastDescantPitch = -1;
	}
	
	if(! partofList && isKeyboard && ! latchIsOn)
		pitchCollectionChanged();
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
	lastPedalPitch = -1;
	lastDescantPitch = -1;
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
	
	for(auto* voice : voices)
		voice->aftertouchChanged(channelPressureValue);
};

void Harmonizer::handleController(const int controllerNumber, const int controllerValue)
{
	switch (controllerNumber)
	{
		case 0x1:	handleModWheel		  (controllerValue);	 	return;
		case 0x2:	handleBreathController(controllerValue);		return;
		case 0x4:	handleFootController  (controllerValue);		return;
		case 0x5:	handlePortamentoTime  (controllerValue);		return;
		case 0x7:	handleMainVolume	  (controllerValue);		return;
		case 0x8:	handleBalance		  (controllerValue);		return;
		case 0x40:  handleSustainPedal    (controllerValue >= 64); 	return;
		case 0x42:  handleSostenutoPedal  (controllerValue >= 64); 	return;
		case 0x43:  handleSoftPedal       (controllerValue >= 64); 	return;
		case 0x44:	handleLegato		  (controllerValue >= 64);  return;
		default:    return;
	}
};

void Harmonizer::handleSustainPedal(const bool isDown)
{
	const ScopedLock sl (lock);
	
	sustainPedalDown = isDown;
	
	if(! isDown)
	{
		for (auto* voice : voices)
			if (! voice->isKeyDown())
				stopVoice (voice, 1.0f, true);
	}
};

void Harmonizer::handleSostenutoPedal(const bool isDown)
{
	const ScopedLock sl (lock);
	
	sostenutoPedalDown = isDown;
	
	if(! isDown)
	{
		for (auto* voice : voices)
			if (! voice->isKeyDown())
				stopVoice (voice, 1.0f, true);
	}
};

void Harmonizer::handleSoftPedal(const bool isDown)
{
	ignoreUnused(isDown);
};

void Harmonizer::handleModWheel(const int wheelValue)
{
	ignoreUnused(wheelValue);
};

void Harmonizer::handleBreathController(const int controlValue)
{
	ignoreUnused(controlValue);
};

void Harmonizer::handleFootController(const int controlValue)
{
	ignoreUnused(controlValue);
};

void Harmonizer::handlePortamentoTime(const int controlValue)
{
	ignoreUnused(controlValue);
};

void Harmonizer::handleMainVolume(const int controlValue)
{
	ignoreUnused(controlValue);
};

void Harmonizer::handleBalance(const int controlValue)
{
	ignoreUnused(controlValue);
};

void Harmonizer::handleLegato(const bool isOn)
{
	ignoreUnused(isOn);
};


// pedal pitch -----------------------------------------------------------------------------------------------------------

void Harmonizer::applyPedalPitch()
{
	const ScopedLock sl (lock);
	
	const float velocity = 1.0f;
	
	int currentLowest = 128;
	for(auto* voice : voices)
		if(voice->isVoiceActive() && voice->currentlyPlayingNote < currentLowest && voice->currentlyPlayingNote != lastPedalPitch)
			currentLowest = voice->currentlyPlayingNote;
	
	if(currentLowest == 128) // pathological case -- ie, no notes playing, some error encountered, etc
	{
		if(lastPedalPitch > -1)
			noteOff(lastPedalPitch, 1.0f, false, true, false);
		lastPedalPitch = -1;
		return;
	}
	
	if(currentLowest <= pedalPitchUpperThresh)
	{
		const int newPedalPitch = currentLowest - pedalPitchInterval;
		
		if(newPedalPitch != lastPedalPitch)
		{
			if(lastPedalPitch > -1)
				noteOff(lastPedalPitch, 1.0f, false, true, false);
			
			if(! isPitchActive(newPedalPitch, false))
			{
				lastPedalPitch = newPedalPitch;
				noteOn(newPedalPitch, velocity, false);
			}
			else
				lastPedalPitch = -1; // if we get here, the new pedal pitch is already being played by a MIDI keyboard key
		}
		else
		{
			// if we get here, the new pedal pitch is already on and was triggered by pedal pitch
		}
	}
	else
	{
		if(lastPedalPitch > -1)
			noteOff(lastPedalPitch, 1.0f, false, true, false);
		lastPedalPitch = -1;
	}
};

void Harmonizer::setPedalPitch(const bool isOn) 
{
	if(pedalPitchIsOn != isOn)
	{
		pedalPitchIsOn = isOn;
		
		if(! isOn)
		{
			if(lastPedalPitch > -1)
				noteOff(lastPedalPitch, 1.0f, false, true, false);
			lastPedalPitch = -1;
		}
		else
			applyPedalPitch();
	}
};

void Harmonizer::setPedalPitchUpperThresh(const int newThresh)
{
	if(pedalPitchUpperThresh != newThresh)
	{
		pedalPitchUpperThresh = newThresh;
		if(pedalPitchIsOn)
			applyPedalPitch();
	}
};

void Harmonizer::setPedalPitchInterval(const int newInterval)
{
	if(pedalPitchInterval != newInterval)
	{
		pedalPitchInterval = newInterval;
		if(pedalPitchIsOn)
			applyPedalPitch();
	}
};

// descant ----------------------------------------------------------------------------------------------------------------

void Harmonizer::applyDescant()
{
	const ScopedLock sl (lock);
	
	const float velocity = 1.0f;
	
	int currentHighest = -1;
	for(auto* voice : voices)
		if(voice->isVoiceActive() && voice->currentlyPlayingNote > currentHighest && voice->currentlyPlayingNote != lastDescantPitch)
			currentHighest = voice->currentlyPlayingNote;
	
	if(currentHighest == -1) // pathological case -- ie, no notes playing, some error encountered, etc
	{
		if(lastDescantPitch > -1)
			noteOff(lastDescantPitch, 1.0f, false, true, false);
		lastDescantPitch = -1;
		return;
	}
	
	if(currentHighest >= descantLowerThresh)
	{
		const int newDescantPitch = currentHighest + descantInterval;
		
		if(newDescantPitch != lastDescantPitch)
		{
			if(lastDescantPitch > -1)
				noteOff(lastDescantPitch, 1.0f, false, true, false);
			
			if(! isPitchActive(newDescantPitch, false))
			{
				lastDescantPitch = newDescantPitch;
				noteOn(newDescantPitch, velocity, false);
			}
			else
				lastDescantPitch = -1; // if we get here, the new descant pitch is already on, being played by a MIDI keyboard key
		}
		else
		{
			// if we get here, the new descant pitch is already on, and was triggered by the descant function...
		}
	}
	else
	{
		if(lastDescantPitch > -1)
			noteOff(lastDescantPitch, 1.0f, false, true, false);
		lastDescantPitch = -1;
	}
};

void Harmonizer::setDescant(const bool isOn)
{
	if(descantIsOn != isOn)
	{
		descantIsOn = isOn;
	
		if(! isOn)
		{
			if(lastDescantPitch > -1)
				noteOff(lastDescantPitch, 1.0f, false, true, false);
			lastDescantPitch = -1;
		}
		else
			applyDescant();
	}
};

void Harmonizer::setDescantLowerThresh(const int newThresh)
{
	if(descantLowerThresh != newThresh)
	{
		descantLowerThresh = newThresh;
		if(descantIsOn)
			applyDescant();
	}
};

void Harmonizer::setDescantInterval(const int newInterval)
{
	if(descantInterval != newInterval)
	{
		descantInterval = newInterval;
		if(descantIsOn)
			applyDescant();
	}
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
		quickAttackParams.release  = desiredR;
		for(auto* voice : voices)
		{
			voice->quickRelease.setParameters(quickReleaseParams);
			voice->quickAttack.setParameters (quickAttackParams);
		}
	}
};

void Harmonizer::updateQuickAttackMs(const int newMs)
{
	jassert(newMs > 0);
	
	if(const float desiredA = newMs / 1000.0f; quickAttackParams.attack != desiredA)
	{
		const ScopedLock sl (lock);
		quickAttackParams.attack  = desiredA;
		quickReleaseParams.attack = desiredA;
		for(auto* voice : voices)
		{
			voice->quickAttack.setParameters (quickAttackParams);
			voice->quickRelease.setParameters(quickReleaseParams);
		}
	}
};


// functions for management of HarmonizerVoices------------------------------------------------------------------------------------------------------

HarmonizerVoice* Harmonizer::addVoice(HarmonizerVoice* newVoice)
{
	const ScopedLock sl (lock);
	
	panner.setNumberOfVoices(voices.size() + 1);
	
	return voices.add(newVoice);
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
		
		const int indexRemoving = indexToRemove > -1 ? indexToRemove : 0;
		
		HarmonizerVoice* removing = voices[indexRemoving];
		if(removing->isVoiceActive())
		{
			panner.panValTurnedOff(removing->currentMidipan);
			if(latchIsOn && latchManager.isNoteLatched(removing->currentlyPlayingNote))
				latchManager.noteOffRecieved(removing->currentlyPlayingNote);
		}
		
		voices.remove(indexRemoving);
		
		++voicesRemoved;
	}
	
	const int voicesLeft = voices.size() > 0 ? voices.size() : 1;
	panner.setNumberOfVoices(voicesLeft);
};
