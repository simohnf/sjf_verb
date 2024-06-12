/*
  ==============================================================================

    sjf_verb.h
    Created: 1 May 2024 3:38:49pm
    Author:  Simon Fay

  ==============================================================================
*/

#pragma once

#include "../sjf_audio/sjf_audioUtilitiesC++.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_parameterHandler.h"
#include "../sjf_audio/sjf_rev.h"
#include "parameterIDs.h"
#include "sjf_verb_inputProcessing.h"
#include "sjf_verb_earlyProcessing.h"
#include "sjf_verb_lateProcessing.h"
#include "sjf_verb_outputProcessing.h"

template< typename INTERPOLATION = sjf::interpolation::fourPointInterpolatePD< float > >
class sjf_verb
{
    using Sample = float;
public:
    
    sjf_verb( juce::AudioProcessorValueTreeState &vts ) :
        m_vts(vts), m_paramHandler(m_vts), m_inputProcessor(m_vts),
        m_earlyReflections(m_vts), m_lateReflections(m_vts), m_outputProcessor(m_vts)
    {}
    ~sjf_verb(){}
    
    void processBlock( juce::AudioBuffer<Sample>& buffer );
    
    void addParametersToHandler();
    
    void initialise( Sample sampleRate, int samplesPerBlock, int numberOfChannels );
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout( );
private:
    juce::AudioProcessorValueTreeState& m_vts;
    sjf::parameterHandler::paramHandlerVector m_paramHandler;
    
    
    juce::LinearSmoothedValue< Sample > m_erLevelSmoother, m_lrLevelSmoother, m_drySmoother, m_wetSmoother;
    
    Sample m_SR{44100.0};
//    
//    using earlyRandMod = sjf_verb_earlyProcessor<sjf::modulator::randMod<float>, INTERPOLATION>;
//    using earlySinMod = sjf_verb_earlyProcessor<sjf::modulator::sinMod<float>, INTERPOLATION>;
//    using earlyRandModPtr = std::unique_ptr< sjf_verb_earlyProcessor<sjf::modulator::randMod<float>, INTERPOLATION> >;
//    using earlySinModPtr = std::unique_ptr< sjf_verb_earlyProcessor<sjf::modulator::sinMod<float>, INTERPOLATION> >;
//    
//    using earlyVariant = std::variant< earlyRandModPtr, earlySinModPtr >;
//    
//    struct earlyReseter
//    {
//        void operator()( earlyRandModPtr& e ){ e.reset(); }
//        void operator()( earlySinModPtr& e ){ e.reset(); }
//    };
//    
//    struct earlyVisitor
//    {
//        earlyVisitor( juce::AudioBuffer< Sample >& revBuffer, size_t blockSize ) : m_revBuffer(revBuffer), m_blockSize(blockSize){}
//      
//    private:
//        juce::AudioBuffer< Sample >& m_revBuffer;
//        size_t m_blockSize;
//    };
//    
    sjf_verb_inputProcessor<INTERPOLATION>   m_inputProcessor;
    sjf_verb_earlyProcessor<INTERPOLATION>   m_earlyReflections;
    sjf_verb_lateProcessor<INTERPOLATION>    m_lateReflections;
    sjf_verb_outputProcessor<INTERPOLATION>  m_outputProcessor;
    
    juce::AudioBuffer<Sample> m_revBuffer, m_outputBuffer;
};

