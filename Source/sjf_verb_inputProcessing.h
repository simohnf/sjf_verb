//
//  sjf_verb_inputProcessing.hpp
//  sjf_verb - VST3
//
//  Created by Simon Fay on 10/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_inputProcessing_h
#define sjf_verb_inputProcessing_h

#include "../sjf_audio/sjf_audioUtilities.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_rev.h"
#include "parameterIDs.h"

template< typename Sample >
class sjf_verb_inputProcessor
{
public:
    sjf_verb_inputProcessor(){}
    ~sjf_verb_inputProcessor(){}
    
    void initialise( Sample sampleRate, int numberOfChannels );
    
    inline void process( std::vector< Sample >& samples );
    
    void setInterpolationType( sjf_interpolators::interpolatorTypes interpType );
    
    void reverse( bool shouldReverse);
public:
    Sample m_preDelayTime{0.0}, m_inputLPFCutoff{0.0}, m_inputHPFCutoff{1.0};
    
private:
    Sample m_SR {44100};
    
    
    std::vector< sjf::delayLine::reverseDelay< Sample > > m_preDelays;
    std::vector< sjf::filters::damper< Sample > > m_inputLPF, m_inputHPF;
    
    unsigned NCHANNELS{2};
};

#endif /* sjf_verb_inputProcessing_h */
