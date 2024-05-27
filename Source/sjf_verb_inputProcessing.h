//
//  sjf_verb_inputProcessing.hpp
//  sjf_verb - VST3
//
//  Created by Simon Fay on 10/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_inputProcessing_h
#define sjf_verb_inputProcessing_h

#include "../sjf_audio/sjf_audioUtilitiesC++.h"
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
    
    inline void processBlock( const juce::AudioBuffer< Sample >& inputBuffer, juce::AudioBuffer< Sample >& revBuffer, size_t blockSize );
    
    void setInterpolationType( sjf::interpolation::interpolatorTypes interpType );
    
    void setReversed( bool shouldReverse);

    juce::LinearSmoothedValue< Sample > m_preDelaySmoother, m_LPFSmoother, m_HPFSmoother;
private:
    Sample m_SR {44100};
    
    
    std::vector< sjf::delayLine::reverseDelay< Sample > > m_preDelays;
    std::vector< sjf::filters::damper< Sample > > m_inputLPF, m_inputHPF;
    
    unsigned NCHANNELS{2};
    bool m_reversed{false};
    
};

#endif /* sjf_verb_inputProcessing_h */
