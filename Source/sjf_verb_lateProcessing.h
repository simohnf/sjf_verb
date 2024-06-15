//
//  sjf_verb_lateProcessing.h
//  sjf_verb
//
//  Created by Simon Fay on 26/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_lateProcessing_h
#define sjf_verb_lateProcessing_h

#include "../sjf_audio/sjf_audioUtilities.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_rev.h"
#include "../sjf_audio/sjf_oscillators.h"
#include "../sjf_audio/sjf_modulator.h"
#include "../sjf_audio/sjf_parameterHandler.h"
#include "parameterIDs.h"
#include "sjf_verb_lateProcessor_DSP_Wrappers.h"
#include "sjf_verb_lateVariants&Visitors.h"

using Sample = float;
using LSV = juce::LinearSmoothedValue< Sample >;
using randArray = sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP +'l'+'a'+'t'+'e' >;


template< sjf::interpolation::interpolatorTypes interpType >
class sjf_verb_lateProcessor
{
public:
    sjf_verb_lateProcessor( juce::AudioProcessorValueTreeState& vts ) : m_paramHandler(vts)
        { addParametersToHandler( vts ); }
    ~sjf_verb_lateProcessor(){}
    
    
    size_t initialise( Sample sampleRate, int numberOfChannels );
    
    void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize );
    
    void setLateType( parameterIDs::lateTypesEnum type );
    
    void setMixType( sjf::mixers::mixerTypes mixType );
    
    void setFBLimit( bool shouldLimitFeedback );
    
    lateDSP::varHolder<Sample> m_varHolder;
    
private:
    
    void addParametersToHandler( juce::AudioProcessorValueTreeState& vts );
    
    Sample m_SR{ 44100 };
    parameterIDs::lateTypesEnum m_lateType{ parameterIDs::lateTypesEnum::fdn };
    size_t NCHANNELS{2};
    size_t apl_NSTAGES{6}, apl_NAP_PERSTAGE{2};
    size_t fdn_NCHANNELS{8};
    static constexpr randArray m_randArray; // random values for delayTimes
    bool m_fbLimit{false};
    bool m_stateChanged{false};
    sjf::mixers::mixerTypes m_fdnMixer{ sjf::mixers::mixerTypes::householder };

    typename fdnVariantStruct<interpType>::fdnVariant m_fdn;
    typename apLoopVariantStruct<interpType>::apLoopVariant m_apLoop;
    
    
    
    
    
    sjf::parameterHandler::paramHandlerVector m_paramHandler;
};
#endif /* sjf_verb_lateProcessing_h */
