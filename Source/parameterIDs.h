//
//  parameterIDs.h
//  sjf_verb
//
//  Created by Simon Fay on 01/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef parameterIDs_h
#define parameterIDs_h
#include <JuceHeader.h>

namespace parameterIDs
{

    static const juce::String mainName = "sjf_verb_";

    
    static const juce::String inputHPFCutoff = "InputHPFCutoff"; //y
    static const juce::String inputLPFCutoff = "InputLPFCutoff"; //y
    static const juce::String preDelay = "PreDelay"; //y
    static const juce::String reverse = "Reverse";


    static const juce::String size = "Size"; //y

    static const juce::String earlyHPFCutoff = "EarlyHPFCutoff"; //y
    static const juce::String earlyLPFCutoff = "EarlyLPFCutoff"; //y
    static const juce::String earlyDiffusion = "EarlyDiffusion"; //y
    static const juce::String earlyReflectionType = "EarlyReflectionType";
    static const juce::String earlyReflectionLevel = "EarlyReflectionLevel";


    static const juce::String lateDiffusion = "LateDiffusion"; //y
    static const juce::String decay = "Decay"; //y
    static const juce::String lateHPFCutoff = "LateHPFCutoff"; //y
    static const juce::String lateLPFCutoff = "LateLPFCutoff"; //y
    static const juce::String lateReflectionType = "LateReflectionType";
    static const juce::String lateReflectionLevel = "LateReflectionLevel";
    static const juce::String fdnMixType = "FDN_Mix";
    
    static const juce::String modRate = "ModRate"; //y
    static const juce::String modDepth = "ModDepth"; //y
    static const juce::String modType = "ModType";

    static const juce::String shimmerLevel = "ShimmerLevel"; //y
    static const juce::String shimmerTransposition = "ShimmerTransposition"; //y
    static const juce::String shimmerDualVoice = "ShimmerDualVoice"; //y

    static const juce::String interpolationType = "interpolationType";
    static const juce::String feedbackLimit = "FeedbackLimit";
    static const juce::String monoLow = "MonoLow";
    static const juce::String mix = "Mix"; //y



    enum class idsenum
    {
        inputHPFCutoff,
        inputLPFCutoff,
        preDelay,
        reverse,
        
        size,
        
        earlyHPFCutoff,
        earlyLPFCutoff,
        earlyReflectionType,
        earlyReflectionLevel,
        earlyDiffusion,
        
        lateDiffusion,
        lateReflectionType,
        lateReflectionLevel,
        lateHPFCutoff,
        lateLPFCutoff,
        fdnMixType,
        decay,
        
        modRate,
        modDepth,
        modType,

        shimmerLevel,
        shimmerTransposition,
        shimmerDualVoice,
        
        interpolationType,
        feedbackLimit,
        monoLow,
        mix,
        dry,
        wet
    };

    static const std::unordered_map< juce::String, idsenum > id2enum
    {
        { inputHPFCutoff, idsenum::inputHPFCutoff },
        { inputLPFCutoff, idsenum::inputLPFCutoff },
        { preDelay, idsenum::preDelay },
        { reverse, idsenum::reverse },
        { size, idsenum::size },
        
        { earlyReflectionType, idsenum::earlyReflectionType },
        { earlyReflectionLevel, idsenum::earlyReflectionLevel },
        { earlyHPFCutoff, idsenum::earlyHPFCutoff },
        { earlyLPFCutoff, idsenum::earlyLPFCutoff },
        { earlyDiffusion, idsenum::earlyDiffusion },
        
        { lateDiffusion, idsenum::lateDiffusion },
        { lateReflectionType, idsenum::lateReflectionType },
        { lateReflectionLevel, idsenum::lateReflectionLevel },
        { decay, idsenum::decay },
        { lateHPFCutoff, idsenum::lateHPFCutoff },
        { lateLPFCutoff, idsenum::lateLPFCutoff },
        { fdnMixType, idsenum::fdnMixType },
        
        { modRate, idsenum::modRate },
        { modDepth, idsenum::modDepth },
        { modType, idsenum::modType },
        
        { shimmerLevel, idsenum::shimmerLevel },
        { shimmerTransposition, idsenum::shimmerTransposition },
        { shimmerDualVoice, idsenum::shimmerDualVoice },
        
        { interpolationType, idsenum::interpolationType },
        { feedbackLimit, idsenum::feedbackLimit },
        { monoLow, idsenum::monoLow },
        { mix, idsenum::mix }
    };

    enum class paramType
    {
        FLOAT, INT, BOOLEAN, CHOICE
    };

    static const std::unordered_map< juce::String, paramType > id2ParamTypeEnum
    {
        { inputHPFCutoff, paramType::FLOAT },
        { inputLPFCutoff, paramType::FLOAT },
        { preDelay, paramType::FLOAT },
        { reverse, paramType::BOOLEAN },
        { size, paramType::FLOAT },
        
        { earlyReflectionType, paramType::CHOICE },
        { earlyReflectionLevel, paramType::FLOAT },
        { earlyDiffusion, paramType::FLOAT },
        { earlyHPFCutoff, paramType::FLOAT },
        { earlyLPFCutoff, paramType::FLOAT },
        
        { lateReflectionType, paramType::CHOICE },
        { lateReflectionLevel, paramType::FLOAT },
        { lateDiffusion, paramType::FLOAT },
        { fdnMixType, paramType::CHOICE },
        { decay, paramType::FLOAT },
        { lateHPFCutoff, paramType::FLOAT },
        { lateLPFCutoff, paramType::FLOAT },
        
        { modRate, paramType::FLOAT },
        { modDepth, paramType::FLOAT },
        { modType, paramType::CHOICE },
        
        { shimmerLevel, paramType::FLOAT },
        { shimmerTransposition, paramType::FLOAT },
        { shimmerDualVoice, paramType::BOOLEAN },
        
        { interpolationType, paramType::CHOICE },
        { feedbackLimit, paramType::BOOLEAN },
        { monoLow, paramType::BOOLEAN },
        { mix, paramType::FLOAT },
    };

    static const juce::String et_rdd{ "rotDelDif" };
    static const juce::String et_mt{ "multitap" };
    static const juce::String et_sap{ "seriesAP" };
    static const juce::String et_mt_sAP{ "mt->sAP" };
    static const juce::StringArray earlyTypes = { et_rdd, et_mt, et_sap, et_mt_sAP  };
    enum class earlyTypesEnum{ rotDelDif, multitap, seriesAP, mt_sAP };
    static const std::unordered_map< int, earlyTypesEnum > earlyTypeMap
    {
        { earlyTypes.indexOf( et_rdd ), earlyTypesEnum::rotDelDif },
        { earlyTypes.indexOf( et_mt ), earlyTypesEnum::multitap },
        { earlyTypes.indexOf( et_sap ), earlyTypesEnum::seriesAP },
        { earlyTypes.indexOf( et_mt_sAP ), earlyTypesEnum::mt_sAP }
    };




    static const juce::String lt_fdn{ "fdn" };
    static const juce::String lt_apl{ "apLoop" };
    static const juce::StringArray lateTypes = { lt_fdn, lt_apl };
    enum class lateTypesEnum{ fdn, apLoop };
    static const std::unordered_map< int, lateTypesEnum > lateTypeMap
    {
        { lateTypes.indexOf( lt_fdn ), lateTypesEnum::fdn },
        { lateTypes.indexOf( lt_apl ), lateTypesEnum::apLoop }
    };

    static const juce::String interp_none{ "none" };
    static const juce::String interp_lin{ "linear" };
    static const juce::String interp_cub{ "cubic" };
    static const juce::String interp_pd{ "pureData" };
    static const juce::String interp_4th{ "4thOrder" };
    static const juce::String interp_god{ "godot" };
    static const juce::String interp_her{ "hermite" };
    static const juce::StringArray interpTypes = { interp_none, interp_lin, interp_cub, interp_pd, interp_4th, interp_god, interp_her };

    static const std::unordered_map< int, sjf::interpolation::interpolatorTypes > interpMap
    {
        { interpTypes.indexOf(interp_none), sjf::interpolation::interpolatorTypes::none },
        { interpTypes.indexOf(interp_lin), sjf::interpolation::interpolatorTypes::linear },
        { interpTypes.indexOf(interp_cub), sjf::interpolation::interpolatorTypes::cubic },
        { interpTypes.indexOf(interp_pd), sjf::interpolation::interpolatorTypes::pureData },
        { interpTypes.indexOf(interp_4th), sjf::interpolation::interpolatorTypes::fourthOrder },
        { interpTypes.indexOf(interp_god), sjf::interpolation::interpolatorTypes::godot },
        { interpTypes.indexOf(interp_her), sjf::interpolation::interpolatorTypes::hermite }
    };

    static const juce::StringArray modTypes = { "random", "sin" };
    
    static const juce::String mixNone{ "None" };
    static const juce::String mixHouseholder{ "Householder" };
    static const juce::String mixHadamard{ "Hadamard" };
    static const juce::StringArray fdnMixTypes = { mixNone, mixHouseholder, mixHadamard };
    static const std::unordered_map< int, sjf::mixers::mixerTypes > fdnMixMap
    {
        { fdnMixTypes.indexOf(mixNone), sjf::mixers::mixerTypes::none },
        { fdnMixTypes.indexOf(mixHouseholder), sjf::mixers::mixerTypes::householder },
        { fdnMixTypes.indexOf(mixHadamard), sjf::mixers::mixerTypes::hadamard }
    };

}


#endif /* parameterIDs_h */

