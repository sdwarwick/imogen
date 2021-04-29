
/*======================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 PluginProcessorParameters.cpp: This file contains functions dealing with parameter creation, management, and updating.
 
======================================================================================================================================================*/


#include "PluginProcessor.h"


/*===========================================================================================================================
 ProcessorStateChangeReciever functions
 ============================================================================================================================*/

void ImogenAudioProcessor::recieveExternalParameterChange (ParameterID param, float newValue)
{
    getParameterPntr(param)->orig()->setValueNotifyingHost (newValue);
}

void ImogenAudioProcessor::recieveExternalParameterGesture (ParameterID param, bool gestureStart)
{
    if (gestureStart)
        getParameterPntr(param)->orig()->beginChangeGesture();
    else
        getParameterPntr(param)->orig()->endChangeGesture();
}


/*===========================================================================================================================
    Creates Imogen's parameter tree.
 ============================================================================================================================*/

juce::AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters()
{
    using Group = juce::AudioProcessorParameterGroup;

    std::vector<std::unique_ptr<Group>> groups;
    
    juce::NormalisableRange<float> gainRange (-60.0f, 0.0f, 0.01f);
    juce::NormalisableRange<float> zeroToOneRange (0.0f, 1.0f, 0.01f);
    juce::NormalisableRange<float> msRange (0.001f, 1.0f, 0.001f);
    juce::NormalisableRange<float> hzRange (40.0f, 10000.0f, 1.0f);
           
    constexpr auto generic = juce::AudioProcessorParameter::genericParameter;
    const auto emptyString = juce::String();
           
    std::function< juce::String (bool value, int maximumStringLength) >  toggle_stringFromBool = [](bool value, int maxLength) { return value ? TRANS("On").substring(0, maxLength) : TRANS("Off").substring(0, maxLength); };
    std::function< bool (const juce::String& text) >                     toggle_boolFromString = [](const juce::String& text) { return (text.containsIgnoreCase (TRANS("On")) || text.containsIgnoreCase (TRANS("Yes")) || text.containsIgnoreCase (TRANS("True"))); };
   
    std::function< juce::String (float value, int maximumStringLength) > gain_stringFromFloat = [](float value, int maxLength) { return (juce::String(value) + " " + TRANS("dB")).substring(0, maxLength); };          
    std::function< float (const juce::String& text) >                    gain_floatFromString = [](const juce::String& text) 
                                                                                                {
                                                                                                    const auto token_location = text.indexOfWholeWordIgnoreCase (TRANS("dB"));
               
                                                                                                    if (token_location > -1)       
                                                                                                        return text.substring (0, token_location).trim().getFloatValue();
               
                                                                                                    return text.trim().getFloatValue();
                                                                                                };
    
    std::function< juce::String (int value, int maximumStringLength) >   pcnt_stringFromInt  = [](int value, int maxLength) { return (juce::String(value) + "%").substring(0, maxLength); };
    std::function< int (const juce::String& text) >                      pcnt_intFromString  = [](const juce::String& text) 
                                                                                               { 
                                                                                                    const auto token_location = text.indexOf ("%");
               
                                                                                                    if (token_location > -1)
                                                                                                        return text.substring (0, token_location).trim().getIntValue();     
                                                                                                               
                                                                                                    return text.trim().getIntValue(); 
                                                                                               };

    std::function< juce::String (float value, int maximumStringLength) > sec_stringFromFloat = [](float value, int maxLength) { return (juce::String(value) + " " + TRANS("sec")).substring(0, maxLength); };          
    std::function< float (const juce::String& text) >                    sec_floatFromString = [](const juce::String& text) 
                                                                                               { 
                                                                                                   const auto token_location = text.indexOfWholeWordIgnoreCase (TRANS("sec"));
               
                                                                                                   if (token_location > -1)
                                                                                                       return text.substring (0, token_location).trim().getFloatValue();
                                                                                                   
                                                                                                   return text.trim().getFloatValue(); 
                                                                                               };    
           
    std::function< juce::String (float value, int maximumStringLength) > hz_stringFromFloat  = [](float value, int maxLength) { auto string = (value < 1000.0f) ? juce::String (value) + " " + TRANS("Hz") : juce::String (value * 0.001f) + " " + TRANS("kHz"); return string.substring(0, maxLength); };
    std::function< float (const juce::String& text) >                    hz_floatFromString  = [](juce::String text) 
                                                                                               { 
                                                                                                   const auto kHz_token_location = text.indexOfWholeWordIgnoreCase (TRANS("kHz"));
               
                                                                                                   if (kHz_token_location > -1)
                                                                                                       return text.substring (0, kHz_token_location).trim().getFloatValue() * 1000.0f;
               
                                                                                                   const auto hz_token_location = text.indexOfWholeWordIgnoreCase (TRANS("Hz"));
               
                                                                                                   if (hz_token_location > -1)
                                                                                                       return text.substring (0, hz_token_location).trim().getFloatValue();
               
                                                                                                   return text.trim().getFloatValue();
                                                                                               };
           
    std::function< juce::String (int value, int maximumStringLength) >   st_stringFromInt    = [](int value, int maxLength) { return (juce::String(value) + " " + TRANS("st")).substring(0, maxLength); };
    std::function< int (const juce::String& text) >                      st_intFromString    = [](const juce::String& text) 
                                                                                               { 
                                                                                                    const auto token_location = text.indexOfWholeWordIgnoreCase (TRANS("st"));
               
                                                                                                    if (token_location > -1)
                                                                                                        return text.substring (0, token_location).trim().getIntValue();

                                                                                                    return text.trim().getIntValue();
                                                                                               };
                 
    std::function< juce::String (int value, int maximumStringLength) >   pitch_stringFromInt = [](int value, int maxLength) { return bav::midi::pitchToString (value, true).substring(0, maxLength); };
    std::function< int (const juce::String& text) >                      pitch_intFromString = [](const juce::String& text) 
                                                                                               { 
                                                                                                    const auto pitchClassTokens = juce::String("AaBbCcDdEeFfGg#") + bav::gui::getSharpSymbol() + bav::gui::getFlatSymbol();
               
                                                                                                    if (text.containsAnyOf (pitchClassTokens))
                                                                                                        return bav::midi::stringToPitch (text.trim());
               
                                                                                                    return text.trim().getIntValue();
                                                                                               };
           
    std::function< juce::String (int value, int maximumStringLength) >   normPcnt_stringFromInt = [](int value, int maxLength) { return (juce::String(value * 100.0f) + "%").substring(0, maxLength); };
    std::function< int (const juce::String& text) >                      normPcnt_intFromString = [](const juce::String& text) 
                                                                                                  { 
                                                                                                      const auto token_location = text.indexOf ("%");
               
                                                                                                      if (token_location > -1)
                                                                                                          return text.substring (0, token_location).trim().getFloatValue() * 0.01f;

                                                                                                      return text.trim().getFloatValue();   
                                                                                                  };
           
     
    {   /* MIXING */
        auto inputMode = std::make_unique<IntParameter> (inputSourceID, "inputSource", getParameterNameVerbose (inputSourceID), 1, 3, 1, emptyString,
                                                         [](int value, int maxLength) 
                                                         { 
                                                             switch (value)
                                                             {
                                                                 case (2): return TRANS("Right").substring(0, maxLength);  
                                                                 case (3): return TRANS("Mix to mono").substring(0, maxLength);
                                                                 default:  return TRANS("Left").substring(0, maxLength);
                                                             }      
                                                         },
                                                         [](const juce::String& text) 
                                                         {  
                                                             if (text.containsIgnoreCase (TRANS("Right"))) return 2;
                                                             if (text.containsIgnoreCase (TRANS("mono")) || text.containsIgnoreCase (TRANS("mix"))) return 3;
                                                             return 1;
                                                         });  
               
        auto dryWetP = std::make_unique<IntParameter>   (dryWetID, "masterDryWet", getParameterNameVerbose (dryWetID), 0, 100, 100, emptyString, pcnt_stringFromInt, pcnt_intFromString);
        auto inGain  = std::make_unique<FloatParameter> (inputGainID, "inputGain", getParameterNameVerbose (inputGainID), gainRange, 0.0f,  emptyString, juce::AudioProcessorParameter::inputGain, gain_stringFromFloat, gain_floatFromString);
        auto outGain = std::make_unique<FloatParameter> (outputGainID, "outputGain", getParameterNameVerbose (outputGainID), gainRange, -4.0f, emptyString, juce::AudioProcessorParameter::outputGain, gain_stringFromFloat, gain_floatFromString);
               
        //  subgroup: bypasses
        auto mainBypassP = std::make_unique<BoolParameter>  (mainBypassID, "mainBypass", getParameterNameVerbose (mainBypassID), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto leadBypassP = std::make_unique<BoolParameter>  (leadBypassID, "leadBypass", getParameterNameVerbose (leadBypassID), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto harmonyBypassP = std::make_unique<BoolParameter>  (harmonyBypassID, "harmonyBypass", getParameterNameVerbose (harmonyBypassID), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
                   
        auto bypasses = std::make_unique<Group> ("Bypasses", TRANS ("Bypasses"), "|", std::move (mainBypassP), std::move (leadBypassP), std::move (harmonyBypassP));
               
        //  subgroup: stereo image     
        auto stereo_width  = std::make_unique<IntParameter> (stereoWidthID, "stereoWidth", getParameterNameVerbose (stereoWidthID), 0, 100, 100, emptyString, pcnt_stringFromInt, pcnt_intFromString);
        auto stereo_lowest = std::make_unique<IntParameter> (lowestPannedID, "lowestPan", getParameterNameVerbose (lowestPannedID), 0, 127, 0, emptyString, pitch_stringFromInt, pitch_intFromString);
               
        auto stereo_leadPan = std::make_unique<IntParameter>  (dryPanID, "dryPan", getParameterNameVerbose (dryPanID), 0, 127, 64, emptyString,
                                                               [](int value, int maxLength) { return bav::midiPanIntToString (value).substring(0, maxLength); }, 
                                                               [](const juce::String& text) { return bav::midiPanStringToInt (text); });        
               
        auto stereo = std::make_unique<Group> ("Stereo image", TRANS ("Stereo image"), "|", std::move (stereo_width), std::move (stereo_lowest), std::move (stereo_leadPan));       
               
        groups.emplace_back (std::make_unique<Group> ("Mixing", TRANS ("Mixing"), "|", 
                                                      std::move (inputMode), std::move (dryWetP), std::move (inGain), std::move (outGain), std::move (bypasses), std::move (stereo)));
    }       
    {   /* MIDI */     
        auto pitchbendRange = std::make_unique<IntParameter>   (pitchBendRangeID, "PitchBendRange", getParameterNameVerbose (pitchBendRangeID), 0, 12, 2, emptyString, st_stringFromInt, st_intFromString);
        auto velocitySensP = std::make_unique<IntParameter>     (velocitySensID, "midiVelocitySens", getParameterNameVerbose (velocitySensID), 0, 100, 100, emptyString, pcnt_stringFromInt, pcnt_intFromString);
        auto aftertouchToggle = std::make_unique<BoolParameter>(aftertouchGainToggleID, "aftertouchGainToggle", getParameterNameVerbose (aftertouchGainToggleID), true, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto voiceStealingP = std::make_unique<BoolParameter>   (voiceStealingID, "voiceStealing", getParameterNameVerbose (voiceStealingID), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
               
        //  subgroup: pedal pitch
        auto pedal_toggle = std::make_unique<BoolParameter>  (pedalPitchIsOnID, "pedalPitchToggle", getParameterNameVerbose (pedalPitchIsOnID), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto pedal_thresh = std::make_unique<IntParameter>   (pedalPitchThreshID, "pedalPitchThresh", getParameterNameVerbose (pedalPitchThreshID), 0, 127, 0, emptyString, pitch_stringFromInt, pitch_intFromString);
        auto pedal_interval = std::make_unique<IntParameter> (pedalPitchIntervalID, "pedalPitchInterval", getParameterNameVerbose (pedalPitchIntervalID), 1, 12, 12, emptyString, st_stringFromInt, st_intFromString);
               
        auto pedal = std::make_unique<Group> ("Pedal pitch", TRANS ("Pedal pitch"), "|", std::move (pedal_toggle), std::move (pedal_thresh), std::move (pedal_interval));     
               
        //  subgroup: descant       
        auto descant_toggle = std::make_unique<BoolParameter>  (descantIsOnID, "descantToggle", getParameterNameVerbose (descantIsOnID), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto descant_thresh = std::make_unique<IntParameter>   (descantThreshID, "descantThresh", getParameterNameVerbose (descantThreshID), 0, 127, 127, emptyString, pcnt_stringFromInt, pitch_intFromString);
        auto descant_interval = std::make_unique<IntParameter> (descantIntervalID, "descantInterval", getParameterNameVerbose (descantIntervalID), 1, 12, 12, emptyString, st_stringFromInt, st_intFromString);
               
        auto descant = std::make_unique<Group> ("Descant", TRANS ("Descant"), "|", std::move (descant_toggle), std::move (descant_thresh), std::move (descant_interval));
               
        groups.emplace_back (std::make_unique<Group> ("MIDI", TRANS ("MIDI"), "|", 
                                                      std::move(pitchbendRange), std::move (velocitySensP), std::move (aftertouchToggle), std::move (voiceStealingP), std::move (pedal), std::move (descant)));
    }           
    {   /* ADSR */
        auto attack  = std::make_unique<FloatParameter> (adsrAttackID, "adsrAttack", getParameterNameVerbose (adsrAttackID), msRange, 0.35f, emptyString, generic, sec_stringFromFloat, sec_floatFromString);
        auto decay   = std::make_unique<FloatParameter> (adsrDecayID, "adsrDecay", getParameterNameVerbose (adsrDecayID), msRange, 0.06f, emptyString, generic, sec_stringFromFloat, sec_floatFromString);
        auto sustain = std::make_unique<FloatParameter> (adsrSustainID, "adsrSustain", getParameterNameVerbose (adsrSustainID), zeroToOneRange, 0.8f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        auto release = std::make_unique<FloatParameter> (adsrReleaseID, "adsrRelease", getParameterNameVerbose (adsrReleaseID), msRange, 0.1f, emptyString, generic, sec_stringFromFloat, sec_floatFromString);
               
        groups.emplace_back (std::make_unique<Group> ("ADSR", TRANS ("ADSR"), "|", 
                                                      std::move (attack), std::move (decay), std::move (sustain), std::move (release)));       
    }        
    {    /* EFFECTS */
        //  subgroup: noise gate   
        auto gate_toggle = std::make_unique<BoolParameter>  (noiseGateToggleID, "noiseGateIsOn", getParameterNameVerbose (noiseGateToggleID), true, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto gate_thresh = std::make_unique<FloatParameter> (noiseGateThresholdID, "noiseGateThresh", getParameterNameVerbose (noiseGateThresholdID), gainRange, -20.0f, emptyString, generic, gain_stringFromFloat, gain_floatFromString);
        
        auto gate = std::make_unique<Group> ("Noise gate", TRANS ("Noise gate"), "|", std::move (gate_toggle), std::move (gate_thresh));       
               
        //  subgroup: de-esser
        auto ess_toggle = std::make_unique<BoolParameter>  (deEsserToggleID, "deEsserIsOn", getParameterNameVerbose (deEsserToggleID), true, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto ess_thresh = std::make_unique<FloatParameter> (deEsserThreshID, "deEsserThresh", getParameterNameVerbose (deEsserThreshID), gainRange, -6.0f, emptyString, generic, gain_stringFromFloat, gain_floatFromString);
        auto ess_amount = std::make_unique<FloatParameter> (deEsserAmountID, "deEsserAmount", getParameterNameVerbose (deEsserAmountID), zeroToOneRange, 0.5f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);
                   
        auto deEss = std::make_unique<Group> ("De-esser", TRANS ("De-esser"), "|", std::move (ess_toggle), std::move (ess_thresh), std::move (ess_amount));      

        //  subgroup: compressor
        auto comp_toggle = std::make_unique<BoolParameter>  (compressorToggleID, "compressorToggle", getParameterNameVerbose (compressorToggleID), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto comp_amount = std::make_unique<FloatParameter> (compressorAmountID, "compressorAmount", getParameterNameVerbose (compressorAmountID), zeroToOneRange, 0.35f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        
        auto compressor = std::make_unique<Group> ("Compressor", TRANS ("Compressor"), "|", std::move (comp_toggle), std::move (comp_amount));
        
        //  subgroup: delay
        auto delay_toggle = std::make_unique<BoolParameter> (delayToggleID, "delayIsOn", getParameterNameVerbose (delayToggleID), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto delay_mix = std::make_unique<IntParameter>     (delayDryWetID, "delayDryWet", getParameterNameVerbose (delayDryWetID), 0, 100, 35, emptyString,
                                                             pcnt_stringFromInt, pcnt_intFromString);
        
        auto delay = std::make_unique<Group> ("Delay", TRANS ("Delay"), "|", std::move (delay_toggle), std::move (delay_mix));
               
        //  subgroup: reverb       
        auto verb_toggle = std::make_unique<BoolParameter>  (reverbToggleID, "reverbIsOn", getParameterNameVerbose (reverbToggleID), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto verb_dryWet = std::make_unique<IntParameter>   (reverbDryWetID, "reverbDryWet", getParameterNameVerbose (reverbDryWetID), 0, 100, 35, emptyString, pcnt_stringFromInt, pcnt_intFromString);
        auto verb_decay  = std::make_unique<FloatParameter> (reverbDecayID, "reverbDecay", getParameterNameVerbose (reverbDecayID), zeroToOneRange, 0.6f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        auto verb_duck   = std::make_unique<FloatParameter> (reverbDuckID, "reverbDuck", getParameterNameVerbose (reverbDuckID), zeroToOneRange, 0.3f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        auto verb_loCut  = std::make_unique<FloatParameter> (reverbLoCutID, "reverbLoCut", getParameterNameVerbose (reverbLoCutID), hzRange, 80.0f, emptyString, generic, hz_stringFromFloat, hz_floatFromString);
        auto verb_hiCut  = std::make_unique<FloatParameter> (reverbHiCutID, "reverbHiCut", getParameterNameVerbose (reverbHiCutID), hzRange, 5500.0f, emptyString, generic, hz_stringFromFloat, hz_floatFromString);
        
        auto reverb = std::make_unique<Group> ("Reverb", TRANS ("Reverb"), "|", std::move (verb_toggle), std::move (verb_dryWet), std::move (verb_decay), std::move (verb_duck), std::move (verb_loCut), std::move (verb_hiCut));
               
        //  limiter   
        auto limiter = std::make_unique<Group> ("Limiter", TRANS ("Limiter"), "|", 
                                                std::make_unique<BoolParameter>  (limiterToggleID, "limiterIsOn", getParameterNameVerbose (limiterToggleID), true, emptyString, toggle_stringFromBool, toggle_boolFromString));
               
        groups.emplace_back (std::make_unique<Group> ("Effects", TRANS ("Effects"), "|", 
                                                      std::move (gate), std::move (deEss), std::move (compressor),
                                                      std::move (delay), std::move (reverb), std::move (limiter)));
    } 
    
    return { groups.begin(), groups.end() };                                             
}


/*===========================================================================================================================
    Initializes pointers to each parameter object.
 ============================================================================================================================*/

void ImogenAudioProcessor::initializeParameterPointers()
{
    parameterPointers.reserve (numParams);
    
    for (auto* rawParam : getParameters())
        parameterPointers.push_back (dynamic_cast<Parameter*>(rawParam));
}


template <typename SampleType>
void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine<SampleType>& engine)
{
    // these functions will all be called with the current denormalized value as a float -- so just static_cast to int, etc
    getParameterPntr(inputSourceID)->actionableFunction   = [&engine](float value) { engine.setModulatorSource (juce::roundToInt (value)); };
    getParameterPntr(mainBypassID)->actionableFunction    = [&engine](float value) { engine.updateLeadBypass (value >= 0.5f); };
    getParameterPntr(leadBypassID)->actionableFunction    = [&engine](float value) { engine.updateLeadBypass (value >= 0.5f); };
    getParameterPntr(harmonyBypassID)->actionableFunction = [&engine](float value) { engine.updateHarmonyBypass (value >= 0.5f); };
    getParameterPntr(dryPanID)->actionableFunction = [&engine](float value) { engine.updateDryVoxPan (juce::roundToInt (value)); };
    getParameterPntr(adsrAttackID)->actionableFunction  = [&engine](float value) { engine.updateAdsrAttack (value); };
    getParameterPntr(adsrDecayID)->actionableFunction   = [&engine](float value) { engine.updateAdsrDecay (value); };
    getParameterPntr(adsrSustainID)->actionableFunction = [&engine](float value) { engine.updateAdsrSustain (value); };
    getParameterPntr(adsrReleaseID)->actionableFunction = [&engine](float value) { engine.updateAdsrRelease (value); };
    getParameterPntr(stereoWidthID)->actionableFunction = [&engine](float value) { engine.updateStereoWidth (juce::roundToInt (value)); };
    getParameterPntr(lowestPannedID)->actionableFunction = [&engine](float value) { engine.updateLowestPannedNote (juce::roundToInt (value)); };
    getParameterPntr(velocitySensID)->actionableFunction = [&engine](float value) { engine.updateMidiVelocitySensitivity(juce::roundToInt(value)); };
    getParameterPntr(pitchBendRangeID)->actionableFunction     = [&engine](float value) { engine.updatePitchbendRange (juce::roundToInt (value)); };
    getParameterPntr(pedalPitchIsOnID)->actionableFunction     = [&engine](float value) { engine.updatePedalToggle (value >= 0.5f); };
    getParameterPntr(pedalPitchThreshID)->actionableFunction   = [&engine](float value) { engine.updatePedalThresh (juce::roundToInt (value)); };
    getParameterPntr(pedalPitchIntervalID)->actionableFunction = [&engine](float value) { engine.updatePedalInterval (juce::roundToInt (value)); };
    getParameterPntr(descantIsOnID)->actionableFunction        = [&engine](float value) { engine.updateDescantToggle (value >= 0.5f); };
    getParameterPntr(descantThreshID)->actionableFunction      = [&engine](float value) { engine.updateDescantThresh (juce::roundToInt (value)); };
    getParameterPntr(descantIntervalID)->actionableFunction    = [&engine](float value) { engine.updateDescantInterval (juce::roundToInt (value)); };
    getParameterPntr(voiceStealingID)->actionableFunction      = [&engine](float value) { engine.updateNoteStealing (value >= 0.5f); };
    getParameterPntr(inputGainID)->actionableFunction          = [&engine](float value) { engine.updateInputGain (value); };
    getParameterPntr(outputGainID)->actionableFunction         = [&engine](float value) { engine.updateOutputGain (value); };
    getParameterPntr(limiterToggleID)->actionableFunction      = [&engine](float value) { engine.updateLimiter (value >= 0.5f); };
    getParameterPntr(noiseGateToggleID)->actionableFunction    = [&engine](float value) { engine.updateNoiseGateToggle (value >= 0.5f); };
    getParameterPntr(noiseGateThresholdID)->actionableFunction = [&engine](float value) { engine.updateNoiseGateThresh (value); };
    getParameterPntr(compressorToggleID)->actionableFunction   = [&engine](float value) { engine.updateCompressorToggle (value >= 0.5f); };
    getParameterPntr(compressorAmountID)->actionableFunction   = [&engine](float value) { engine.updateCompressorAmount (value); };
    getParameterPntr(aftertouchGainToggleID)->actionableFunction = [&engine](float value) { engine.updateAftertouchGainOnOff (value >= 0.5f); };
    getParameterPntr(deEsserToggleID)->actionableFunction = [&engine](float value) { engine.updateDeEsserToggle (value >= 0.5f); };
    getParameterPntr(deEsserThreshID)->actionableFunction = [&engine](float value) { engine.updateDeEsserThresh (value); };
    getParameterPntr(deEsserAmountID)->actionableFunction = [&engine](float value) { engine.updateDeEsserAmount (value); };
    getParameterPntr(reverbToggleID)->actionableFunction  = [&engine](float value) { engine.updateReverbToggle (value >= 0.5f); };
    getParameterPntr(reverbDryWetID)->actionableFunction  = [&engine](float value) { engine.updateReverbDryWet (juce::roundToInt (value)); };
    getParameterPntr(reverbDecayID)->actionableFunction   = [&engine](float value) { engine.updateReverbDecay (value); };
    getParameterPntr(reverbDuckID)->actionableFunction    = [&engine](float value) { engine.updateReverbDuck (value); };
    getParameterPntr(reverbLoCutID)->actionableFunction   = [&engine](float value) { engine.updateReverbLoCut (value); };
    getParameterPntr(reverbHiCutID)->actionableFunction   = [&engine](float value) { engine.updateReverbHiCut (value); };
    getParameterPntr(delayToggleID)->actionableFunction   = [&engine](float value) { engine.updateDelayToggle (value >= 0.5f); };
    getParameterPntr(delayDryWetID)->actionableFunction   = [&engine](float value) { engine.updateDelayDryWet (juce::roundToInt (value)); };
}
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine<float>& engine);
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine<double>& engine);

/*===========================================================================================================================
    Returns one of the processor's parameter objects, referenced by its parameterID.
 ============================================================================================================================*/

bav::Parameter* ImogenAudioProcessor::getParameterPntr (const ParameterID paramID) const
{
    for (auto* pntr : parameterPointers)
        if (static_cast<ParameterID>(pntr->key()) == paramID)
            return pntr;
    
    return nullptr;
}


/*===========================================================================================================================
    Processes all the non-parameter events in the message queue.
 ============================================================================================================================*/

template<typename SampleType>
void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<SampleType>& activeEngine)
{
    nonParamEvents.getReadyMessages (currentMessages);
    
    for (const auto& msg : currentMessages)
    {
        if (! msg.isValid())
            continue;
        
        const auto value = msg.value();
        
        jassert (value >= 0.0f && value <= 1.0f);
        
        switch (msg.type())
        {
            default: continue;
            case (killAllMidiID): activeEngine.killAllMidi();  // any message of this type triggers this, regardless of its value
            case (midiLatchID):   changeMidiLatchState (value >= 0.5f);
            case (pitchBendFromEditorID): activeEngine.recieveExternalPitchbend (juce::roundToInt (pitchbendNormalizedRange.convertFrom0to1 (value)));
        }
    }
}
///function template instantiations...
template void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<double>& activeEngine);

