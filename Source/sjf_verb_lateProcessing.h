//
//  sjf_verb_lateProcessing.h
//  sjf_verb
//
//  Created by Simon Fay on 26/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_lateProcessing_h
#define sjf_verb_lateProcessing_h

#include "../sjf_audio/sjf_audioUtilitiesC++.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_rev.h"
#include "../sjf_audio/sjf_oscillators.h"
#include "parameterIDs.h"
#include "sjf_verb_lateProcessor_DSP_Wrappers.h"

class sjf_verb_lateProcessor
{
    using Sample = float;
    using LSV = juce::LinearSmoothedValue< Sample >;
    using randArray = sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP +'l'+'a'+'t'+'e' >;
    
public:
    sjf_verb_lateProcessor() {}
    ~sjf_verb_lateProcessor(){}
    
    
    size_t initialise( Sample sampleRate, int numberOfChannels );
    
    void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize );
    
    void setLateType( parameterIDs::lateTypesEnum type );
    
    lateDSP::varHolder<Sample> m_varHolder;
    
private:
    
    sjf::rev::mixers m_mixType{sjf::rev::mixers::householder};
    
    Sample m_SR{ 44100 };
    parameterIDs::lateTypesEnum m_lateType{ parameterIDs::lateTypesEnum::fdn };
    size_t NCHANNELS{2};
    size_t apl_NSTAGES{6}, apl_NAP_PERSTAGE{2};
    size_t fdn_NCHANNELS{8};
    static constexpr randArray m_randArray; // random values for delayTimes
    
    std::unique_ptr< lateDSP::fdnWrapper<Sample> >       m_fdn;
    std::unique_ptr< lateDSP::apLoopWrapper<Sample> >    m_apLoop;
};
#endif /* sjf_verb_lateProcessing_h */
