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

template< typename INTERPOLATION = sjf::interpolation::fourPointInterpolatePD< float > >
class sjf_verb_outputProcessor
{
    using Sample = float;
public:
    sjf_verb_outputProcessor(){}
    ~sjf_verb_outputProcessor(){}
    
    void initialise( Sample sampleRate, int nChannels );
    
    void processBlock( juce::AudioBuffer< Sample >& outputBuffer, juce::AudioBuffer< Sample >& revBuffer, size_t blockSize );
    
    void setMonoLow( bool monoLow );
    
    std::array< juce::LinearSmoothedValue<Sample>, 2 > m_shimShiftSmoother;
    juce::LinearSmoothedValue<Sample> m_shimLevelSmoother;
    
    void setShimmerDualVoice( bool dualVoice ){ m_shimmerDualVoice = dualVoice; }
    
private:
    
    size_t NCHANNELS{2};
    bool m_monoLow{false}, m_shimmerDualVoice{false};
    std::array< sjf::delayLine::pitchShift<Sample, INTERPOLATION>, 2 > m_shimVoice;
    sjf::filters::damper< Sample > m_monoLowFilt;
    Sample m_coef{0.1};
    
    
};


#endif /* sjf_verb_outputProcessing_h */
