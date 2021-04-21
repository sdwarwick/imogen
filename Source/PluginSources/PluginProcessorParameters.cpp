
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
    std::function< bool (const juce::String& text) >                     toggle_boolFromString = [](const juce::String& text) { if (text.containsIgnoreCase (TRANS("On")) || text.containsIgnoreCase (TRANS("Yes")) || text.containsIgnoreCase (TRANS("True"))) return true; return false; };
   
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
       
#define imgn_USE_SHARPS_FOR_PITCHES true           
    std::function< juce::String (int value, int maximumStringLength) >   pitch_stringFromInt = [](int value, int maxLength) { return bav::midi::pitchToString (value, imgn_USE_SHARPS_FOR_PITCHES).substring(0, maxLength); };
    std::function< int (const juce::String& text) >                      pitch_intFromString = [](const juce::String& text) 
                                                                                               { 
                                                                                                    const auto pitchClassTokens = juce::String("AaBbCcDdEeFfGg#") + bav::gui::getSharpSymbol() + bav::gui::getFlatSymbol();
               
                                                                                                    if (text.containsAnyOf (pitchClassTokens))
                                                                                                        return bav::midi::stringToPitch (text.trim(), imgn_USE_SHARPS_FOR_PITCHES);
               
                                                                                                    return text.trim().getIntValue();
                                                                                               };
#undef imgn_USE_SHARPS_FOR_PITCHES
           
     
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
               
        auto dryWet  = std::make_unique<IntParameter>   ("masterDryWet", TRANS ("% wet"), 0, 100, 100, emptyString, pcnt_stringFromInt, pcnt_intFromString);                  
        auto inGain  = std::make_unique<FloatParameter> ("inputGain", TRANS ("Input gain"),   gainRange, 0.0f,  emptyString, juce::AudioProcessorParameter::inputGain, gain_stringFromFloat, gain_floatFromString);               
        auto outGain = std::make_unique<FloatParameter> ("outputGain", TRANS ("Output gain"), gainRange, -4.0f, emptyString, juce::AudioProcessorParameter::outputGain, gain_stringFromFloat, gain_floatFromString);        
               
        //  subgroup: bypasses
        auto mainBypass = std::make_unique<BoolParameter>  ("mainBypass", TRANS ("Bypass"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);        
        auto leadBypass = std::make_unique<BoolParameter>  ("leadBypass", TRANS ("Lead bypass"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);               
        auto harmonyBypass = std::make_unique<BoolParameter>  ("harmonyBypass", TRANS ("Harmony bypass"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);
                   
        auto bypasses = std::make_unique<Group> ("Bypasses", TRANS ("Bypasses"), "|", std::move (mainBypass), std::move (leadBypass), std::move (harmonyBypass));       
               
        //  subgroup: stereo image     
        auto stereo_width  = std::make_unique<IntParameter> ("stereoWidth", TRANS ("Stereo Width"), 0, 100, 100, emptyString, pcnt_stringFromInt, pcnt_intFromString);               
        auto stereo_lowest = std::make_unique<IntParameter> ("lowestPan", TRANS ("Lowest panned midiPitch"), 0, 127, 0, emptyString, pitch_stringFromInt, pitch_intFromString);
               
        auto stereo_leadPan = std::make_unique<IntParameter>  ("dryPan", TRANS ("Dry vox pan"), 0, 127, 64, emptyString, 
                                                               [](int value, int maxLength) { return bav::midiPanIntToString (value).substring(0, maxLength); }, 
                                                               [](const juce::String& text) { return bav::midiPanStringToInt (text); });        
               
        auto stereo = std::make_unique<Group> ("Stereo image", TRANS ("Stereo image"), "|", std::move (stereo_width), std::move (stereo_lowest), std::move (stereo_leadPan));       
               
        groups.emplace_back (std::make_unique<Group> ("Mixing", TRANS ("Mixing"), "|", 
                                                      std::move (inputMode), std::move (dryWet), std::move (inGain), std::move (outGain), std::move (bypasses), std::move (stereo)));  
    }       
    {   /* MIDI */     
        auto pitchbendRange = std::make_unique<IntParameter>   ("PitchBendRange", TRANS ("Pitch bend range"), 0, 12, 2, emptyString, st_stringFromInt, st_intFromString);             
        auto velocitySens = std::make_unique<IntParameter>     ("midiVelocitySens", TRANS ("MIDI Velocity Sensitivity"), 0, 100, 100, emptyString, pcnt_stringFromInt, pcnt_intFromString);       
        auto aftertouchToggle = std::make_unique<BoolParameter>("aftertouchGainToggle", TRANS ("Aftertouch gain on/off"), true, emptyString, toggle_stringFromBool, toggle_boolFromString);       
        auto voiceStealing = std::make_unique<BoolParameter>   ("voiceStealing", TRANS ("Voice stealing"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);       
               
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
                                                      std::move(pitchbendRange), std::move (velocitySens), std::move (aftertouchToggle), std::move (voiceStealing), std::move (pedal), std::move (descant)));       
    }           
    {   /* ADSR */
        auto attack  = std::make_unique<FloatParameter> ("adsrAttack", TRANS ("ADSR Attack"), msRange, 0.35f, emptyString, generic, sec_stringFromFloat, sec_floatFromString);
        auto decay   = std::make_unique<FloatParameter> ("adsrDecay", TRANS ("ADSR Decay"), msRange, 0.06f, emptyString, generic, sec_stringFromFloat, sec_floatFromString); 
               
        auto sustain = std::make_unique<FloatParameter> ("adsrSustain", TRANS ("ADSR Sustain"), zeroToOneRange, 0.8f, emptyString, generic,
                                                         [](float value, int maxLength) { auto string = juce::String(value * 100.0f) + "%"; return string.substring(0, maxLength); },
                                                         [](const juce::String& text) 
                                                         { 
                                                             const auto token_location = text.indexOf ("%");
               
                                                             if (token_location > -1)
                                                                 return text.substring (0, token_location).trim().getFloatValue() * 0.01f;

                                                             return text.trim().getFloatValue() * 0.01f;
                                                         });
               
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
        auto ess_amount = std::make_unique<FloatParameter> ("deEsserAmount", TRANS ("De-esser amount"), zeroToOneRange, 0.5f, emptyString, generic, nullptr, nullptr);       
                   
        auto deEss = std::make_unique<Group> ("De-esser", TRANS ("De-esser"), "|", std::move (ess_toggle), std::move (ess_thresh), std::move (ess_amount));      

        //  subgroup: compressor
        auto comp_toggle = std::make_unique<BoolParameter>  ("compressorToggle", TRANS ("Compressor on/off"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);                 
        auto comp_amount = std::make_unique<FloatParameter> ("compressorAmount", TRANS ("Compressor amount"), zeroToOneRange, 0.35f, emptyString, generic, nullptr, nullptr);
        
        auto compressor = std::make_unique<Group> ("Compressor", TRANS ("Compressor"), "|", std::move (comp_toggle), std::move (comp_amount));       
               
        //  subgroup: reverb       
        auto verb_toggle = std::make_unique<BoolParameter>  ("reverbIsOn", TRANS ("Reverb toggle"), false, emptyString, toggle_stringFromBool, toggle_boolFromString);               
        auto verb_dryWet = std::make_unique<IntParameter>   ("reverbDryWet", TRANS ("Reverb dry/wet"), 0, 100, 35, emptyString, pcnt_stringFromInt, pcnt_intFromString);               
        auto verb_decay  = std::make_unique<FloatParameter> ("reverbDecay", TRANS ("Reverb decay"), zeroToOneRange, 0.6f, emptyString, generic, nullptr, nullptr);             
        auto verb_duck   = std::make_unique<FloatParameter> ("reverbDuck", TRANS ("Duck amount"), zeroToOneRange, 0.3f, emptyString, generic, nullptr, nullptr);               
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
    mainBypass           = dynamic_cast<BoolParamPtr>  (tree.getParameter ("mainBypass"));                   jassert (mainBypass);
    leadBypass           = dynamic_cast<BoolParamPtr>  (tree.getParameter ("leadBypass"));                   jassert (leadBypass);
    harmonyBypass        = dynamic_cast<BoolParamPtr>  (tree.getParameter ("harmonyBypass"));                jassert (harmonyBypass);
    inputSource          = dynamic_cast<IntParamPtr>   (tree.getParameter ("inputSource"));                  jassert (inputSource);
    dryPan               = dynamic_cast<IntParamPtr>   (tree.getParameter ("dryPan"));                       jassert (dryPan);
    dryWet               = dynamic_cast<IntParamPtr>   (tree.getParameter ("masterDryWet"));                 jassert (dryWet);
    adsrAttack           = dynamic_cast<FloatParamPtr> (tree.getParameter ("adsrAttack"));                   jassert (adsrAttack);
    adsrDecay            = dynamic_cast<FloatParamPtr> (tree.getParameter ("adsrDecay"));                    jassert (adsrDecay);
    adsrSustain          = dynamic_cast<FloatParamPtr> (tree.getParameter ("adsrSustain"));                  jassert (adsrSustain);
    adsrRelease          = dynamic_cast<FloatParamPtr> (tree.getParameter ("adsrRelease"));                  jassert (adsrRelease);
    stereoWidth          = dynamic_cast<IntParamPtr>   (tree.getParameter ("stereoWidth"));                  jassert (stereoWidth);
    lowestPanned         = dynamic_cast<IntParamPtr>   (tree.getParameter ("lowestPan"));                    jassert (lowestPanned);
    velocitySens         = dynamic_cast<IntParamPtr>   (tree.getParameter ("midiVelocitySens"));             jassert (velocitySens);
    pitchBendRange       = dynamic_cast<IntParamPtr>   (tree.getParameter ("PitchBendRange"));               jassert (pitchBendRange);
    pedalPitchIsOn       = dynamic_cast<BoolParamPtr>  (tree.getParameter ("pedalPitchToggle"));             jassert (pedalPitchIsOn);
    pedalPitchThresh     = dynamic_cast<IntParamPtr>   (tree.getParameter ("pedalPitchThresh"));             jassert (pedalPitchThresh);
    pedalPitchInterval   = dynamic_cast<IntParamPtr>   (tree.getParameter ("pedalPitchInterval"));           jassert (pedalPitchInterval);
    descantIsOn          = dynamic_cast<BoolParamPtr>  (tree.getParameter ("descantToggle"));                jassert (descantIsOn);
    descantThresh        = dynamic_cast<IntParamPtr>   (tree.getParameter ("descantThresh"));                jassert (descantThresh);
    descantInterval      = dynamic_cast<IntParamPtr>   (tree.getParameter ("descantInterval"));              jassert (descantInterval);
    voiceStealing        = dynamic_cast<BoolParamPtr>  (tree.getParameter ("voiceStealing"));                jassert (voiceStealing);
    inputGain            = dynamic_cast<FloatParamPtr> (tree.getParameter ("inputGain"));                    jassert (inputGain);
    outputGain           = dynamic_cast<FloatParamPtr> (tree.getParameter ("outputGain"));                   jassert (outputGain);
    limiterToggle        = dynamic_cast<BoolParamPtr>  (tree.getParameter ("limiterIsOn"));                  jassert (limiterToggle);
    noiseGateThreshold   = dynamic_cast<FloatParamPtr> (tree.getParameter ("noiseGateThresh"));              jassert (noiseGateThreshold);
    noiseGateToggle      = dynamic_cast<BoolParamPtr>  (tree.getParameter ("noiseGateIsOn"));                jassert (noiseGateToggle);
    compressorToggle     = dynamic_cast<BoolParamPtr>  (tree.getParameter ("compressorToggle"));             jassert (compressorToggle);
    compressorAmount     = dynamic_cast<FloatParamPtr> (tree.getParameter ("compressorAmount"));             jassert (compressorAmount);
    aftertouchGainToggle = dynamic_cast<BoolParamPtr>  (tree.getParameter ("aftertouchGainToggle"));         jassert (aftertouchGainToggle);
    deEsserToggle        = dynamic_cast<BoolParamPtr>  (tree.getParameter ("deEsserIsOn"));                  jassert (deEsserToggle);
    deEsserThresh        = dynamic_cast<FloatParamPtr> (tree.getParameter ("deEsserThresh"));                jassert (deEsserThresh);
    deEsserAmount        = dynamic_cast<FloatParamPtr> (tree.getParameter ("deEsserAmount"));                jassert (deEsserAmount);
    reverbToggle         = dynamic_cast<BoolParamPtr>  (tree.getParameter ("reverbIsOn"));                   jassert (reverbToggle);
    reverbDryWet         = dynamic_cast<IntParamPtr>   (tree.getParameter ("reverbDryWet"));                 jassert (reverbDryWet);
    reverbDecay          = dynamic_cast<FloatParamPtr> (tree.getParameter ("reverbDecay"));                  jassert (reverbDecay);
    reverbDuck           = dynamic_cast<FloatParamPtr> (tree.getParameter ("reverbDuck"));                   jassert (reverbDuck);
    reverbLoCut          = dynamic_cast<FloatParamPtr> (tree.getParameter ("reverbLoCut"));                  jassert (reverbLoCut);
    reverbHiCut          = dynamic_cast<FloatParamPtr> (tree.getParameter ("reverbHiCut"));                  jassert (reverbHiCut);
}


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Initializes individual listener callbacks for each parameter.
*/

// creates parameter listeners & messengers for each parameter
void ImogenAudioProcessor::initializeParameterListeners()
{
    parameterMessengers.reserve (IMGN_NUM_PARAMS);
           
    for (int i = 0; i < IMGN_NUM_PARAMS; ++i)
        addParameterMessenger (parameterID(i));
}


// creates a single parameter listener & messenger for a requested parameter
void ImogenAudioProcessor::addParameterMessenger (parameterID paramID)
{
    auto* param = getParameterPntr (paramID);
    auto& messenger { parameterMessengers.emplace_back (ParameterMessenger (paramChanges, param, paramID)) };
    tree.addParameterListener (param->orig()->paramID, &messenger);
}


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Initializes OSC mappings for each parameter.
*/

void ImogenAudioProcessor::initializeParameterOscMappings()
{
    for (int i = 0; i < IMGN_NUM_PARAMS; ++i)
    {
        auto* param = getParameterPntr (parameterID(i));
        oscMapper.addNewMapping (param, juce::String("imogen/") + param->orig()->paramID);
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
    if      (parameter == mainBypass)           return mainBypassID;
    else if (parameter == leadBypass)           return leadBypassID;
    else if (parameter == harmonyBypass)        return harmonyBypassID;
    else if (parameter == dryPan)               return dryPanID;
    else if (parameter == dryWet)               return dryWetID;
    else if (parameter == adsrAttack)           return adsrAttackID;
    else if (parameter == adsrDecay)            return adsrDecayID;
    else if (parameter == adsrSustain)          return adsrSustainID;
    else if (parameter == adsrRelease)          return adsrReleaseID;
    else if (parameter == stereoWidth)          return stereoWidthID;
    else if (parameter == lowestPanned)         return lowestPannedID;
    else if (parameter == velocitySens)         return velocitySensID;
    else if (parameter == pitchBendRange)       return pitchBendRangeID;
    else if (parameter == pedalPitchIsOn)       return pedalPitchIsOnID;
    else if (parameter == pedalPitchThresh)     return pedalPitchThreshID;
    else if (parameter == pedalPitchInterval)   return pedalPitchIntervalID;
    else if (parameter == descantIsOn)          return descantIsOnID;
    else if (parameter == descantThresh)        return descantThreshID;
    else if (parameter == descantInterval)      return descantIntervalID;
    else if (parameter == voiceStealing)        return voiceStealingID;
    else if (parameter == inputGain)            return inputGainID;
    else if (parameter == outputGain)           return outputGainID;
    else if (parameter == limiterToggle)        return limiterToggleID;
    else if (parameter == noiseGateToggle)      return noiseGateToggleID;
    else if (parameter == noiseGateThreshold)   return noiseGateThresholdID;
    else if (parameter == compressorToggle)     return compressorToggleID;
    else if (parameter == compressorAmount)     return compressorAmountID;
    else if (parameter == aftertouchGainToggle) return aftertouchGainToggleID;
    else if (parameter == deEsserToggle)        return deEsserToggleID;
    else if (parameter == deEsserThresh)        return deEsserThreshID;
    else if (parameter == deEsserAmount)        return deEsserAmountID;
    else if (parameter == reverbToggle)         return reverbToggleID;
    else if (parameter == reverbDryWet)         return reverbDryWetID;
    else if (parameter == reverbDecay)          return reverbDecayID;
    else if (parameter == reverbDuck)           return reverbDuckID;
    else if (parameter == reverbLoCut)          return reverbLoCutID;
    else if (parameter == reverbHiCut)          return reverbHiCutID;
    else if (parameter == inputSource)          return inputSourceID;
    else return mainBypassID;
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
    for (int paramID = 0; paramID < IMGN_NUM_PARAMS; ++paramID)
        getParameterPntr(parameterID(paramID))->refreshDefault();
    
    parameterDefaultsAreDirty.store (true);
}

// Tracks whether or not the processor has updated its default parameter values since the last call to this function.
bool ImogenAudioProcessor::hasUpdatedParamDefaults()
{
    if (parameterDefaultsAreDirty.load())
    {
        parameterDefaultsAreDirty.store (false);
        return true;
    }
    
    return false;
}
