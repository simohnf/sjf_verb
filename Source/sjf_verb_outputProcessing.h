//
//  sjf_verb_outputProcessing.hpp
//  sjf_verb
//
//  Created by Simon Fay on 12/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_outputProcessing_h
#define sjf_verb_outputProcessing_h

#include "../sjf_audio/sjf_audioUtilities.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_rev.h"
#include "parameterIDs.h"

template < typename Sample >
class sjf_verb_outputProcessor
{
//    typedef sjf::utilities::classMemberFunctionPointer< sjf_verb_outputProcessor, void, std::vector< Sample >& > memFunc;
public:
    sjf_verb_outputProcessor(){}
    ~sjf_verb_outputProcessor(){}
    
    void initialise( Sample sampleRate, int nChannels );
    
    inline void processBlock( juce::AudioBuffer< Sample >& revBuffer, size_t blockSize );
    
    void process( std::vector< Sample >& samples );
    
    void setMonoLow( bool monoLow );
    
private:
    void applyMonoLow( std::vector< Sample >& samples );
    void noMonoLow( std::vector< Sample >& samples ) { return; }
    
    size_t NCHANNELS{2};
    bool m_monoLow{false};
    sjf::filters::damper< Sample > m_monoLowFilt;
    Sample m_coef{0.1};
//    memFunc monoLowProcessor{this, &sjf_verb_outputProcessor::noMonoLow};
    
};


#endif /* sjf_verb_outputProcessing_h */
