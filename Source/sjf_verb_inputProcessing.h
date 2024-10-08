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
#include "../sjf_audio/sjf_parameterHandler.h"

#include "parameterIDs.h"

template< sjf::interpolation::interpolatorTypes interpType = sjf::interpolation::interpolatorTypes::pureData >
class sjf_verb_inputProcessor
{
    using Sample = float;
public:
    sjf_verb_inputProcessor( juce::AudioProcessorValueTreeState& vts ) : m_paramHandler(vts)
    { addParametersToHandler( vts ); }
    ~sjf_verb_inputProcessor(){}
    
    void initialise( Sample sampleRate, int numberOfChannels );
    
    void processBlock( juce::AudioBuffer< Sample >& revBuffer, size_t blockSize );
    
    void setReversed( bool shouldReverse);

    juce::LinearSmoothedValue< Sample > m_preDelaySmoother, m_LPFSmoother, m_HPFSmoother;
private:
    void addParametersToHandler( juce::AudioProcessorValueTreeState& vts );
    
    Sample m_SR {44100};
    
    
    std::vector< sjf::delayLine::reverseDelay< Sample, interpType > > m_preDelays;
    std::vector< sjf::filters::damper< Sample > > m_inputLPF, m_inputHPF;
    
    size_t NCHANNELS{2};
    bool m_reversed{false};
    
    sjf::parameterHandler::paramHandlerVector m_paramHandler;
};

#endif /* sjf_verb_inputProcessing_h */
