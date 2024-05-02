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



    static const juce::String earlyReflectionType = "EarlyReflectionType";
    static const juce::String lateReflectionType = "LateReflectionType";

    static const juce::String mix = "Mix";
    static const juce::String inputHPFCutoff = "InputHPFCutoff";
    static const juce::String inputLPFCutoff = "InputLPFCutoff";
    static const juce::String preDelay = "PreDelay";
    static const juce::String reverse = "Reverse";
    static const juce::String size = "Size";
    static const juce::String diffusion = "Diffusion";
    static const juce::String modRate = "ModRate";
    static const juce::String modDepth = "ModDepth";
    static const juce::String modType = "ModType";
    static const juce::String decay = "Decay";
    static const juce::String earlyHPFCutoff = "EarlyHPFCutoff";
    static const juce::String earlyLPFCutoff = "EarlyLPFCutoff";
    static const juce::String lateHPFCutoff = "LateHPFCutoff";
    static const juce::String lateLPFCutoff = "LateLPFCutoff";
    static const juce::String shimmerLevel = "ShimmerLevel";
    static const juce::String shimmerTransposition = "ShimmerTransposition";
    static const juce::String interpolationType = "interpolationType";
    static const juce::String feedbackLimit = "FeedbackLimit";
    static const juce::String monoLow = "MonoLow";



    enum class idsenum
    {
        earlyReflectionType,
        lateReflectionType,
        mix,
        inputHPFCutoff,
        inputLPFCutoff,
        preDelay,
        reverse,
        size,
        diffusion,
        modRate,
        modDepth,
        modType,
        decay,
        earlyHPFCutoff,
        earlyLPFCutoff,
        lateHPFCutoff,
        lateLPFCutoff,
        shimmerLevel,
        shimmerTransposition,
        interpolationType,
        feedbackLimit,
        monoLow
    };

    static const std::unordered_map< juce::String, idsenum > id2enum
    {
        { earlyReflectionType, idsenum::earlyReflectionType },
        { lateReflectionType, idsenum::lateReflectionType },
        { mix, idsenum::mix },
        { inputHPFCutoff, idsenum::inputHPFCutoff },
        { inputLPFCutoff, idsenum::inputLPFCutoff },
        { preDelay, idsenum::preDelay },
        { reverse, idsenum::reverse },
        { size, idsenum::size },
        { diffusion, idsenum::diffusion },
        { modRate, idsenum::modRate },
        { modDepth, idsenum::modDepth },
        { modType, idsenum::modType },
        { decay, idsenum::decay },
        { earlyHPFCutoff, idsenum::earlyHPFCutoff },
        { earlyLPFCutoff, idsenum::earlyLPFCutoff },
        { lateHPFCutoff, idsenum::lateHPFCutoff },
        { lateLPFCutoff, idsenum::lateLPFCutoff },
        { shimmerLevel, idsenum::shimmerLevel },
        { shimmerTransposition, idsenum::shimmerTransposition },
        { interpolationType, idsenum::interpolationType },
        { feedbackLimit, idsenum::feedbackLimit },
        { monoLow, idsenum::monoLow }
    };

    enum class paramType
    {
        FLOAT, INT, BOOLEAN, CHOICE
    };

    static const std::unordered_map< juce::String, paramType > id2ParamTypeEnum
    {
        { earlyReflectionType, paramType::CHOICE },
        { lateReflectionType, paramType::CHOICE },
        { mix, paramType::FLOAT },
        { inputHPFCutoff, paramType::FLOAT },
        { inputLPFCutoff, paramType::FLOAT },
        { preDelay, paramType::FLOAT },
        { reverse, paramType::BOOLEAN },
        { size, paramType::FLOAT },
        { diffusion, paramType::FLOAT },
        { modRate, paramType::FLOAT },
        { modDepth, paramType::FLOAT },
        { modType, paramType::CHOICE },
        { decay, paramType::FLOAT },
        { earlyHPFCutoff, paramType::FLOAT },
        { earlyLPFCutoff, paramType::FLOAT },
        { lateHPFCutoff, paramType::FLOAT },
        { lateLPFCutoff, paramType::FLOAT },
        { shimmerLevel, paramType::FLOAT },
        { shimmerTransposition, paramType::FLOAT },
        { interpolationType, paramType::CHOICE },
        { feedbackLimit, paramType::BOOLEAN },
        { monoLow, paramType::BOOLEAN }
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
    enum class interpTypesEnum{ none, linear, cubic, pureData, fourthOrder, godot, hermite };
    static const std::unordered_map< int, interpTypesEnum > interpMap
    {
        { interpTypes.indexOf(interp_none), interpTypesEnum::none },
        { interpTypes.indexOf(interp_lin), interpTypesEnum::linear },
        { interpTypes.indexOf(interp_cub), interpTypesEnum::cubic },
        { interpTypes.indexOf(interp_pd), interpTypesEnum::pureData },
        { interpTypes.indexOf(interp_4th), interpTypesEnum::fourthOrder },
        { interpTypes.indexOf(interp_god), interpTypesEnum::godot },
        { interpTypes.indexOf(interp_her), interpTypesEnum::hermite }
    };


        static const juce::StringArray modTypes = { "random", "sin" };
}


#endif /* parameterIDs_h */
