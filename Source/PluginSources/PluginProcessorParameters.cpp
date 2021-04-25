
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
 ============================================================================================================================*/

/*
    Creates Imogen's parameter layout.
*/

juce::AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters()
{
    using Group = juce::AudioProcessorParameterGroup;

    std::vector<std::unique_ptr<Group>> groups;
    
    juce::NormalisableRange<float> gainRange (-60.0f, 0.0f, 0.01f);
    juce::NormalisableRange<float> zeroToOneRange (0.0f, 1.0f, 0.01f);
    juce::NormalisableRange<float> msRange (0.001f, 1.0f, 0.001f);
    juce::NormalisableRange<float> hzRange (40.0f, 10000.0f, 1.0f);
           
    const auto generic = juce::AudioProcessorParameter::genericParameter;
           
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
        auto inputMode = std::make_unique<IntParameter> ("inputSource", TRANS ("Input source"), 1, 3, 1, emptyString,
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
               
        auto dryWetP = std::make_unique<IntParameter>   ("masterDryWet", TRANS ("% wet"), 0, 100, 100, emptyString, pcnt_stringFromInt, pcnt_intFromString);
        auto inGain  = std::make_unique<FloatParameter> ("inputGain", TRANS ("Input gain"),   gainRange, 0.0f,  emptyString, juce::AudioProcessorParameter::inputGain, gain_stringFromFloat, gain_floatFromString);               
        auto outGain = std::make_unique<FloatParameter> ("outputGain", TRANS ("Output gain"), gainRange, -4.0f, emptyString, juce::AudioProcessorParameter::outputGain, gain_stringFromFloat, gain_floatFromString);        
               
        //  subgroup: bypasses
        auto mainBypassP = std::make_unique<BoolParameter>  ("mainBypass", TRANS ("Bypass"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto leadBypassP = std::make_unique<BoolParameter>  ("leadBypass", TRANS ("Lead bypass"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
        auto harmonyBypassP = std::make_unique<BoolParameter>  ("harmonyBypass", TRANS ("Harmony bypass"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
                   
        auto bypasses = std::make_unique<Group> ("Bypasses", TRANS ("Bypasses"), "|", std::move (mainBypassP), std::move (leadBypassP), std::move (harmonyBypassP));
               
        //  subgroup: stereo image     
        auto stereo_width  = std::make_unique<IntParameter> ("stereoWidth", TRANS ("Stereo Width"), 0, 100, 100, emptyString, pcnt_stringFromInt, pcnt_intFromString);               
        auto stereo_lowest = std::make_unique<IntParameter> ("lowestPan", TRANS ("Lowest panned midiPitch"), 0, 127, 0, emptyString, pitch_stringFromInt, pitch_intFromString);
               
        auto stereo_leadPan = std::make_unique<IntParameter>  ("dryPan", TRANS ("Dry vox pan"), 0, 127, 64, emptyString, 
                                                               [](int value, int maxLength) { return bav::midiPanIntToString (value).substring(0, maxLength); }, 
                                                               [](const juce::String& text) { return bav::midiPanStringToInt (text); });        
               
        auto stereo = std::make_unique<Group> ("Stereo image", TRANS ("Stereo image"), "|", std::move (stereo_width), std::move (stereo_lowest), std::move (stereo_leadPan));       
               
        groups.emplace_back (std::make_unique<Group> ("Mixing", TRANS ("Mixing"), "|", 
                                                      std::move (inputMode), std::move (dryWetP), std::move (inGain), std::move (outGain), std::move (bypasses), std::move (stereo)));
    }       
    {   /* MIDI */     
        auto pitchbendRange = std::make_unique<IntParameter>   ("PitchBendRange", TRANS ("Pitch bend range"), 0, 12, 2, emptyString, st_stringFromInt, st_intFromString);
        auto velocitySensP = std::make_unique<IntParameter>     ("midiVelocitySens", TRANS ("MIDI Velocity Sensitivity"), 0, 100, 100, emptyString, pcnt_stringFromInt, pcnt_intFromString);
        auto aftertouchToggle = std::make_unique<BoolParameter>("aftertouchGainToggle", TRANS ("Aftertouch gain on/off"), true, emptyString, toggle_stringFromBool, toggle_boolFromString);       
        auto voiceStealingP = std::make_unique<BoolParameter>   ("voiceStealing", TRANS ("Voice stealing"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
               
        //  subgroup: pedal pitch
        auto pedal_toggle = std::make_unique<BoolParameter>  ("pedalPitchToggle", TRANS ("Pedal pitch on/off"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);             
        auto pedal_thresh = std::make_unique<IntParameter>   ("pedalPitchThresh", TRANS ("Pedal pitch upper threshold"), 0, 127, 0, emptyString, pitch_stringFromInt, pitch_intFromString);               
        auto pedal_interval = std::make_unique<IntParameter> ("pedalPitchInterval", TRANS ("Pedal pitch interval"), 1, 12, 12, emptyString, st_stringFromInt, st_intFromString);
               
        auto pedal = std::make_unique<Group> ("Pedal pitch", TRANS ("Pedal pitch"), "|", std::move (pedal_toggle), std::move (pedal_thresh), std::move (pedal_interval));     
               
        //  subgroup: descant       
        auto descant_toggle = std::make_unique<BoolParameter>  ("descantToggle", TRANS ("Descant on/off"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);              
        auto descant_thresh = std::make_unique<IntParameter>   ("descantThresh", TRANS ("Descant lower threshold"), 0, 127, 127, emptyString, pcnt_stringFromInt, pitch_intFromString);         
        auto descant_interval = std::make_unique<IntParameter> ("descantInterval", TRANS ("Descant interval"), 1, 12, 12, emptyString, st_stringFromInt, st_intFromString);
               
        auto descant = std::make_unique<Group> ("Descant", TRANS ("Descant"), "|", std::move (descant_toggle), std::move (descant_thresh), std::move (descant_interval));
               
        groups.emplace_back (std::make_unique<Group> ("MIDI", TRANS ("MIDI"), "|", 
                                                      std::move(pitchbendRange), std::move (velocitySensP), std::move (aftertouchToggle), std::move (voiceStealingP), std::move (pedal), std::move (descant)));
    }           
    {   /* ADSR */
        auto attack  = std::make_unique<FloatParameter> ("adsrAttack", TRANS ("ADSR Attack"), msRange, 0.35f, emptyString, generic, sec_stringFromFloat, sec_floatFromString);
        auto decay   = std::make_unique<FloatParameter> ("adsrDecay", TRANS ("ADSR Decay"), msRange, 0.06f, emptyString, generic, sec_stringFromFloat, sec_floatFromString);               
        auto sustain = std::make_unique<FloatParameter> ("adsrSustain", TRANS ("ADSR Sustain"), zeroToOneRange, 0.8f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);           
        auto release = std::make_unique<FloatParameter> ("adsrRelease", TRANS ("ADSR Release"), msRange, 0.1f, emptyString, generic, sec_stringFromFloat, sec_floatFromString);
               
        groups.emplace_back (std::make_unique<Group> ("ADSR", TRANS ("ADSR"), "|", 
                                                      std::move (attack), std::move (decay), std::move (sustain), std::move (release)));       
    }        
    {    /* EFFECTS */
        //  subgroup: noise gate   
        auto gate_toggle = std::make_unique<BoolParameter>  ("noiseGateIsOn", TRANS ("Noise gate toggle"), true, emptyString, toggle_stringFromBool, toggle_boolFromString);              
        auto gate_thresh = std::make_unique<FloatParameter> ("noiseGateThresh", TRANS ("Noise gate threshold"), gainRange, -20.0f, emptyString, generic, gain_stringFromFloat, gain_floatFromString);
        
        auto gate = std::make_unique<Group> ("Noise gate", TRANS ("Noise gate"), "|", std::move (gate_toggle), std::move (gate_thresh));       
               
        //  subgroup: de-esser
        auto ess_toggle = std::make_unique<BoolParameter>  ("deEsserIsOn", TRANS ("De-esser toggle"), true, emptyString, toggle_stringFromBool, toggle_boolFromString);               
        auto ess_thresh = std::make_unique<FloatParameter> ("deEsserThresh", TRANS ("De-esser thresh"), gainRange, -6.0f, emptyString, generic, gain_stringFromFloat, gain_floatFromString);             
        auto ess_amount = std::make_unique<FloatParameter> ("deEsserAmount", TRANS ("De-esser amount"), zeroToOneRange, 0.5f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);       
                   
        auto deEss = std::make_unique<Group> ("De-esser", TRANS ("De-esser"), "|", std::move (ess_toggle), std::move (ess_thresh), std::move (ess_amount));      

        //  subgroup: compressor
        auto comp_toggle = std::make_unique<BoolParameter>  ("compressorToggle", TRANS ("Compressor on/off"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);                 
        auto comp_amount = std::make_unique<FloatParameter> ("compressorAmount", TRANS ("Compressor amount"), zeroToOneRange, 0.35f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        
        auto compressor = std::make_unique<Group> ("Compressor", TRANS ("Compressor"), "|", std::move (comp_toggle), std::move (comp_amount));       
               
        //  subgroup: reverb       
        auto verb_toggle = std::make_unique<BoolParameter>  ("reverbIsOn", TRANS ("Reverb toggle"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);               
        auto verb_dryWet = std::make_unique<IntParameter>   ("reverbDryWet", TRANS ("Reverb dry/wet"), 0, 100, 35, emptyString, pcnt_stringFromInt, pcnt_intFromString);               
        auto verb_decay  = std::make_unique<FloatParameter> ("reverbDecay", TRANS ("Reverb decay"), zeroToOneRange, 0.6f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);             
        auto verb_duck   = std::make_unique<FloatParameter> ("reverbDuck", TRANS ("Reverb duck"), zeroToOneRange, 0.3f, emptyString, generic, normPcnt_stringFromInt, normPcnt_intFromString);               
        auto verb_loCut  = std::make_unique<FloatParameter> ("reverbLoCut", TRANS ("Reverb low cut"), hzRange, 80.0f, emptyString, generic, hz_stringFromFloat, hz_floatFromString);             
        auto verb_hiCut  = std::make_unique<FloatParameter> ("reverbHiCut", TRANS ("Reverb high cut"), hzRange, 5500.0f, emptyString, generic, hz_stringFromFloat, hz_floatFromString);
        
        auto reverb = std::make_unique<Group> ("Reverb", TRANS ("Reverb"), "|", std::move (verb_toggle), std::move (verb_dryWet), std::move (verb_decay), std::move (verb_duck), std::move (verb_loCut), std::move (verb_hiCut));
               
        //  limiter   
        auto limiter = std::make_unique<Group> ("Limiter", TRANS ("Limiter"), "|", 
                                                std::make_unique<BoolParameter>  ("limiterIsOn", TRANS ("Limiter on/off"), true, emptyString, toggle_stringFromBool, toggle_boolFromString));       
               
        groups.emplace_back (std::make_unique<Group> ("Effects", TRANS ("Effects"), "|", 
                                                      std::move (gate), std::move (deEss), std::move (compressor), std::move (reverb), std::move (limiter)));       
    } 
    
    return { groups.begin(), groups.end() };                                             
}


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Initializes pointers to each parameter object.
*/

void ImogenAudioProcessor::initializeParameterPointers()
{
    mainBypass           = makeParameterPointer <BoolParamPtr>  ("mainBypass");    
    leadBypass           = makeParameterPointer <BoolParamPtr>  ("leadBypass");
    harmonyBypass        = makeParameterPointer <BoolParamPtr>  ("harmonyBypass");
    inputSource          = makeParameterPointer <IntParamPtr>   ("inputSource");
    dryPan               = makeParameterPointer <IntParamPtr>   ("dryPan");
    dryWet               = makeParameterPointer <IntParamPtr>   ("masterDryWet");
    adsrAttack           = makeParameterPointer <FloatParamPtr> ("adsrAttack");        
    adsrDecay            = makeParameterPointer <FloatParamPtr> ("adsrDecay");     
    adsrSustain          = makeParameterPointer <FloatParamPtr> ("adsrSustain");
    adsrRelease          = makeParameterPointer <FloatParamPtr> ("adsrRelease");
    stereoWidth          = makeParameterPointer <IntParamPtr>   ("stereoWidth");    
    lowestPanned         = makeParameterPointer <IntParamPtr>   ("lowestPan");
    velocitySens         = makeParameterPointer <IntParamPtr>   ("midiVelocitySens");     
    pitchBendRange       = makeParameterPointer <IntParamPtr>   ("PitchBendRange");       
    pedalPitchIsOn       = makeParameterPointer <BoolParamPtr>  ("pedalPitchToggle");    
    pedalPitchThresh     = makeParameterPointer <IntParamPtr>   ("pedalPitchThresh");      
    pedalPitchInterval   = makeParameterPointer <IntParamPtr>   ("pedalPitchInterval");        
    descantIsOn          = makeParameterPointer <BoolParamPtr>  ("descantToggle");       
    descantThresh        = makeParameterPointer <IntParamPtr>   ("descantThresh");     
    descantInterval      = makeParameterPointer <IntParamPtr>   ("descantInterval");      
    voiceStealing        = makeParameterPointer <BoolParamPtr>  ("voiceStealing");    
    inputGain            = makeParameterPointer <FloatParamPtr> ("inputGain");      
    outputGain           = makeParameterPointer <FloatParamPtr> ("outputGain");      
    limiterToggle        = makeParameterPointer <BoolParamPtr>  ("limiterIsOn");      
    noiseGateThreshold   = makeParameterPointer <FloatParamPtr> ("noiseGateThresh");   
    noiseGateToggle      = makeParameterPointer <BoolParamPtr>  ("noiseGateIsOn");  
    compressorToggle     = makeParameterPointer <BoolParamPtr>  ("compressorToggle");     
    compressorAmount     = makeParameterPointer <FloatParamPtr> ("compressorAmount");     
    aftertouchGainToggle = makeParameterPointer <BoolParamPtr>  ("aftertouchGainToggle");       
    deEsserToggle        = makeParameterPointer <BoolParamPtr>  ("deEsserIsOn");       
    deEsserThresh        = makeParameterPointer <FloatParamPtr> ("deEsserThresh");       
    deEsserAmount        = makeParameterPointer <FloatParamPtr> ("deEsserAmount");        
    reverbToggle         = makeParameterPointer <BoolParamPtr>  ("reverbIsOn");       
    reverbDryWet         = makeParameterPointer <IntParamPtr>   ("reverbDryWet");      
    reverbDecay          = makeParameterPointer <FloatParamPtr> ("reverbDecay");     
    reverbDuck           = makeParameterPointer <FloatParamPtr> ("reverbDuck"); 
    reverbLoCut          = makeParameterPointer <FloatParamPtr> ("reverbLoCut");        
    reverbHiCut          = makeParameterPointer <FloatParamPtr> ("reverbHiCut");      
}


template<typename PointerType>
PointerType ImogenAudioProcessor::makeParameterPointer (const juce::String& name)
{
    auto* param = dynamic_cast<PointerType> (tree.getParameter (name));    
    jassert (param != nullptr);
    return param;
}
///template function instantiations...
template bav::FloatParameter* ImogenAudioProcessor::makeParameterPointer (const juce::String& name);
template bav::IntParameter*   ImogenAudioProcessor::makeParameterPointer (const juce::String& name);
template bav::BoolParameter*  ImogenAudioProcessor::makeParameterPointer (const juce::String& name);


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Initializes individual listener callbacks for each parameter.
*/

// creates parameter listeners & messengers for each parameter
void ImogenAudioProcessor::initializeParameterListeners()
{
    parameterMessengers.reserve (numParams);
           
    for (int i = 0; i < numParams; ++i)
        addParameterMessenger (parameterID(i));
}


// creates a single parameter listener & messenger for a requested parameter
void ImogenAudioProcessor::addParameterMessenger (parameterID paramID)
{
    auto* param = getParameterPntr (paramID);
    auto& messenger { parameterMessengers.emplace_back (ParameterMessenger (*this, paramChanges, param, paramID)) };
    param->orig()->addListener (&messenger);
}


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Initializes OSC mappings for each parameter.
*/

void ImogenAudioProcessor::initializeParameterOscMappings()
{
    for (int i = 0; i < numParams; ++i)
    {
        auto* param = getParameterPntr (parameterID(i));
        oscMapper.addNewMapping (param, juce::String("/imogen/") + param->orig()->paramID);
    }    
}


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Returns one of the processor's parameter objects, referenced by its parameterID.
*/

bav::Parameter* ImogenAudioProcessor::getParameterPntr (const parameterID paramID) const
{
    switch (paramID)
    {
        case (mainBypassID):            return mainBypass;
        case (leadBypassID):            return leadBypass;
        case (harmonyBypassID):         return harmonyBypass;
        case (dryPanID):                return dryPan;
        case (dryWetID):                return dryWet;
        case (adsrAttackID):            return adsrAttack;
        case (adsrDecayID):             return adsrDecay;
        case (adsrSustainID):           return adsrSustain;
        case (adsrReleaseID):           return adsrRelease;
        case (stereoWidthID):           return stereoWidth;
        case (lowestPannedID):          return lowestPanned;
        case (velocitySensID):          return velocitySens;
        case (pitchBendRangeID):        return pitchBendRange;
        case (pedalPitchIsOnID):        return pedalPitchIsOn;
        case (pedalPitchThreshID):      return pedalPitchThresh;
        case (pedalPitchIntervalID):    return pedalPitchInterval;
        case (descantIsOnID):           return descantIsOn;
        case (descantThreshID):         return descantThresh;
        case (descantIntervalID):       return descantInterval;
        case (voiceStealingID):         return voiceStealing;
        case (inputGainID):             return inputGain;
        case (outputGainID):            return outputGain;
        case (limiterToggleID):         return limiterToggle;
        case (noiseGateToggleID):       return noiseGateToggle;
        case (noiseGateThresholdID):    return noiseGateThreshold;
        case (compressorToggleID):      return compressorToggle;
        case (compressorAmountID):      return compressorAmount;
        case (aftertouchGainToggleID):  return aftertouchGainToggle;
        case (deEsserToggleID):         return deEsserToggle;
        case (deEsserThreshID):         return deEsserThresh;
        case (deEsserAmountID):         return deEsserAmount;
        case (reverbToggleID):          return reverbToggle;
        case (reverbDryWetID):          return reverbDryWet;
        case (reverbDecayID):           return reverbDecay;
        case (reverbDuckID):            return reverbDuck;
        case (reverbLoCutID):           return reverbLoCut;
        case (reverbHiCutID):           return reverbHiCut;
        case (inputSourceID):           return inputSource;
        default:                        return nullptr;
    }
}


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Returns the corresponding parameterID for the passed parameter.
*/

ImogenAudioProcessor::parameterID ImogenAudioProcessor::parameterPntrToID (const Parameter* const parameter) const
{
    if (parameter == mainBypass)           return mainBypassID;
    if (parameter == leadBypass)           return leadBypassID;
    if (parameter == harmonyBypass)        return harmonyBypassID;
    if (parameter == dryPan)               return dryPanID;
    if (parameter == dryWet)               return dryWetID;
    if (parameter == adsrAttack)           return adsrAttackID;
    if (parameter == adsrDecay)            return adsrDecayID;
    if (parameter == adsrSustain)          return adsrSustainID;
    if (parameter == adsrRelease)          return adsrReleaseID;
    if (parameter == stereoWidth)          return stereoWidthID;
    if (parameter == lowestPanned)         return lowestPannedID;
    if (parameter == velocitySens)         return velocitySensID;
    if (parameter == pitchBendRange)       return pitchBendRangeID;
    if (parameter == pedalPitchIsOn)       return pedalPitchIsOnID;
    if (parameter == pedalPitchThresh)     return pedalPitchThreshID;
    if (parameter == pedalPitchInterval)   return pedalPitchIntervalID;
    if (parameter == descantIsOn)          return descantIsOnID;
    if (parameter == descantThresh)        return descantThreshID;
    if (parameter == descantInterval)      return descantIntervalID;
    if (parameter == voiceStealing)        return voiceStealingID;
    if (parameter == inputGain)            return inputGainID;
    if (parameter == outputGain)           return outputGainID;
    if (parameter == limiterToggle)        return limiterToggleID;
    if (parameter == noiseGateToggle)      return noiseGateToggleID;
    if (parameter == noiseGateThreshold)   return noiseGateThresholdID;
    if (parameter == compressorToggle)     return compressorToggleID;
    if (parameter == compressorAmount)     return compressorAmountID;
    if (parameter == aftertouchGainToggle) return aftertouchGainToggleID;
    if (parameter == deEsserToggle)        return deEsserToggleID;
    if (parameter == deEsserThresh)        return deEsserThreshID;
    if (parameter == deEsserAmount)        return deEsserAmountID;
    if (parameter == reverbToggle)         return reverbToggleID;
    if (parameter == reverbDryWet)         return reverbDryWetID;
    if (parameter == reverbDecay)          return reverbDecayID;
    if (parameter == reverbDuck)           return reverbDuckID;
    if (parameter == reverbLoCut)          return reverbLoCutID;
    if (parameter == reverbHiCut)          return reverbHiCutID;
    if (parameter == inputSource)          return inputSourceID;
    return mainBypassID;
}


/*===========================================================================================================================
 ============================================================================================================================*/


/*
    Updates the compressor's settings based on the "one knob"
*/

template<typename SampleType>
void ImogenAudioProcessor::updateCompressor (bav::ImogenEngine<SampleType>& activeEngine,
                                             bool compressorIsOn, float knobValue)
{
    jassert (knobValue >= 0.0f && knobValue <= 1.0f);
    
    activeEngine.updateCompressor (juce::jmap (knobValue, 0.0f, -60.0f),  // threshold (dB)
                                   juce::jmap (knobValue, 1.0f, 10.0f),  // ratio
                                   compressorIsOn);
}


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Updates all parameters based on their current values
*/

template<typename SampleType>
void ImogenAudioProcessor::updateAllParameters (bav::ImogenEngine<SampleType>& activeEngine)
{
    activeEngine.updateBypassStates (leadBypass->get(), harmonyBypass->get());

    activeEngine.updateInputGain (juce::Decibels::decibelsToGain (inputGain->get()));
    activeEngine.updateOutputGain (juce::Decibels::decibelsToGain (outputGain->get()));
    activeEngine.updateDryVoxPan (dryPan->get());
    activeEngine.updateDryWet (dryWet->get());
    activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get());
    activeEngine.updateStereoWidth (stereoWidth->get(), lowestPanned->get());
    activeEngine.updateMidiVelocitySensitivity (velocitySens->get());
    activeEngine.updatePitchbendRange (pitchBendRange->get());
    activeEngine.updatePedalPitch (pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
    activeEngine.updateDescant (descantIsOn->get(), descantThresh->get(), descantInterval->get());
    activeEngine.updateNoteStealing (voiceStealing->get());
    activeEngine.updateAftertouchGainOnOff (aftertouchGainToggle->get());
    activeEngine.setModulatorSource (inputSource->get());
    activeEngine.updateLimiter (limiterToggle->get());
    activeEngine.updateNoiseGate (noiseGateThreshold->get(), noiseGateToggle->get());
    activeEngine.updateDeEsser (deEsserAmount->get(), deEsserThresh->get(), deEsserToggle->get());

    updateCompressor (activeEngine, compressorToggle->get(), compressorAmount->get());

    activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                               reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
}
///function template instantiations...
template void ImogenAudioProcessor::updateAllParameters (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::updateAllParameters (bav::ImogenEngine<double>& activeEngine);


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Processes all the parameter changes in the message queue.
*/

template<typename SampleType>
void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<SampleType>& activeEngine)
{
    paramChanges.getReadyMessages (currentMessages);
    
    bool adsr = false;
    float adsrA = adsrAttack->get(), adsrD = adsrDecay->get(), adsrS = adsrSustain->get(), adsrR = adsrRelease->get();
    
    bool reverb = false;
    int rDryWet = reverbDryWet->get();
    float rDecay = reverbDecay->get(), rDuck = reverbDuck->get(), rLoCut = reverbLoCut->get(), rHiCut = reverbHiCut->get();
    bool rToggle = reverbToggle->get();
    
    // converts a message's raw normalized value to its actual float value using the normalisable range of the selected parameter p
#define _FLOAT_MSG getParameterPntr(parameterID(type))->denormalize (value)
    
    // converts a message's raw normalized value to its actual integer value using the normalisable range of the selected parameter p
#define _INT_MSG juce::roundToInt (_FLOAT_MSG)
    
    // converts a message's raw normalized value to its actual boolean true/false value
#define _BOOL_MSG value >= 0.5f
    
    for (const auto& msg : currentMessages)
    {
        if (! msg.isValid())
            continue;
        
        const float value = msg.value();
        
        jassert (value >= 0.0f && value <= 1.0f);
        
        const int type = msg.type();
        
        switch (type)
        {
            default:             continue;
            case (mainBypassID): continue;
            case (leadBypassID):     activeEngine.updateBypassStates (_BOOL_MSG, harmonyBypass->get());
            case (harmonyBypassID):  activeEngine.updateBypassStates (leadBypass->get(), _BOOL_MSG);
            case (inputSourceID):    activeEngine.setModulatorSource (_INT_MSG);
            case (dryPanID):         activeEngine.updateDryVoxPan (_INT_MSG);
            case (dryWetID):         activeEngine.updateDryWet (_INT_MSG);
            case (stereoWidthID):    activeEngine.updateStereoWidth (_INT_MSG, lowestPanned->get());
            case (lowestPannedID):   activeEngine.updateStereoWidth (stereoWidth->get(), _INT_MSG);
            case (velocitySensID):   activeEngine.updateMidiVelocitySensitivity (_INT_MSG);
            case (pitchBendRangeID): activeEngine.updatePitchbendRange (_INT_MSG);
            case (voiceStealingID):         activeEngine.updateNoteStealing (_BOOL_MSG);
            case (inputGainID):             activeEngine.updateInputGain (juce::Decibels::decibelsToGain (_FLOAT_MSG));
            case (outputGainID):            activeEngine.updateOutputGain (juce::Decibels::decibelsToGain (_FLOAT_MSG));
            case (limiterToggleID):         activeEngine.updateLimiter (_BOOL_MSG);
            case (noiseGateToggleID):       activeEngine.updateNoiseGate (noiseGateThreshold->get(), _BOOL_MSG);
            case (noiseGateThresholdID):    activeEngine.updateNoiseGate (_FLOAT_MSG, noiseGateToggle->get());
            case (compressorToggleID):      updateCompressor (activeEngine, _BOOL_MSG, compressorAmount->get());
            case (compressorAmountID):      updateCompressor (activeEngine, compressorToggle->get(), _FLOAT_MSG);
            case (aftertouchGainToggleID):  activeEngine.updateAftertouchGainOnOff (_BOOL_MSG);
                
            case (pedalPitchIsOnID):     activeEngine.updatePedalPitch (_BOOL_MSG, pedalPitchThresh->get(), pedalPitchInterval->get());
            case (pedalPitchThreshID):   activeEngine.updatePedalPitch (pedalPitchIsOn->get(), _INT_MSG, pedalPitchInterval->get());
            case (pedalPitchIntervalID): activeEngine.updatePedalPitch (pedalPitchIsOn->get(), pedalPitchThresh->get(), _INT_MSG);
                
            case (descantIsOnID):     activeEngine.updateDescant (_BOOL_MSG, descantThresh->get(), descantInterval->get());
            case (descantThreshID):   activeEngine.updateDescant (descantIsOn->get(), _INT_MSG, descantInterval->get());
            case (descantIntervalID): activeEngine.updateDescant (descantIsOn->get(), descantThresh->get(), _INT_MSG);
                
            case (deEsserToggleID):   activeEngine.updateDeEsser (deEsserAmount->get(), deEsserThresh->get(), _BOOL_MSG);
            case (deEsserThreshID):   activeEngine.updateDeEsser (deEsserAmount->get(), _FLOAT_MSG, deEsserToggle->get());
            case (deEsserAmountID):   activeEngine.updateDeEsser (_FLOAT_MSG, deEsserThresh->get(), deEsserToggle->get());
                
            case (reverbToggleID): rToggle = _BOOL_MSG;   reverb = true;
            case (reverbDryWetID): rDryWet = _INT_MSG;    reverb = true;
            case (reverbDecayID):  rDecay  = _FLOAT_MSG;  reverb = true;
            case (reverbDuckID):   rDuck   = _FLOAT_MSG;  reverb = true;
            case (reverbLoCutID):  rLoCut  = _FLOAT_MSG;  reverb = true;
            case (reverbHiCutID):  rHiCut  = _FLOAT_MSG;  reverb = true;
                
            case (adsrAttackID):  adsrA = _FLOAT_MSG;     adsr = true;
            case (adsrDecayID):   adsrD = _FLOAT_MSG;     adsr = true;
            case (adsrSustainID): adsrS = _FLOAT_MSG;     adsr = true;
            case (adsrReleaseID): adsrR = _FLOAT_MSG;     adsr = true;
        }
    }
    
    if (adsr)
        activeEngine.updateAdsr (adsrA, adsrD, adsrS, adsrR);
    
    if (reverb)
        activeEngine.updateReverb (rDryWet, rDecay, rDuck, rLoCut, rHiCut, rToggle);
}
///function template instantiations...
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<double>& activeEngine);

#undef _FLOAT_MSG
#undef _INT_MSG
#undef _BOOL_MSG



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
            case (killAllMidi): activeEngine.killAllMidi();  // any message of this type triggers this, regardless of its value
            case (midiLatch):   activeEngine.updateMidiLatch (value >= 0.5f);
            case (pitchBendFromEditor): activeEngine.recieveExternalPitchbend (juce::roundToInt (pitchbendNormalizedRange.convertFrom0to1 (value)));
        }
    }
}
///function template instantiations...
template void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<double>& activeEngine);


/*===========================================================================================================================
 ============================================================================================================================*/


// This function reassigns each parameter's internally stored default value to the parameter's current value. Run this function after loading a preset, etc.
void ImogenAudioProcessor::updateParameterDefaults()
{
    for (int paramID = 0; paramID < numParams; ++paramID)
        getParameterPntr(parameterID(paramID))->refreshDefault();
    
    parameterDefaultsAreDirty.store (true);
}
