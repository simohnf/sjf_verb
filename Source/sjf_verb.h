/*
  ==============================================================================

    sjf_verb.h
    Created: 1 May 2024 3:38:49pm
    Author:  Simon Fay

  ==============================================================================
*/

#pragma once

#include "../sjf_audio/sjf_audioUtilities.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_parameterHandler.h"
#include "../sjf_audio/sjf_rev.h"
#include "parameterIDs.h"
#include "sjf_verb_dspWrapper.h"
#include "sjf_verb_inputProcessing.h"



template < typename Sample >
class sjf_verb
{
public:
    
    sjf_verb() {}
    ~sjf_verb() { m_smoothers.clear(); }
    
    inline void process( juce::AudioBuffer<Sample>& buffer );
    
    void addParametersToHandler( juce::AudioProcessorValueTreeState &vts, juce::Array<juce::AudioProcessorParameter*>& params  );
    
    void initialise( Sample sampleRate, int numberOfChannels );
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout( );
private:
    
    inline Sample getSmoothedVal( parameterIDs::idsenum idE )
    {
        jassert( m_smootherMap.find( idE ) != m_smootherMap.end() );
        return ( m_smootherMap.find( idE ) )->second->getNextValue();
    }
    
    
    sjf::parameterHandler::paramHandlerVector m_paramHandler;
    
    std::vector< juce::LinearSmoothedValue< Sample > > m_smoothers;
    std::unordered_map<  parameterIDs::idsenum, juce::LinearSmoothedValue< Sample >* > m_smootherMap;
    
    
    Sample m_SR{44100.0}, m_erLevel{1.0}, m_lrLevel{1.0}, m_dry{0.0}, m_wet{1.0};
    
    sjf_verb_DSP_wrapper< Sample > m_dspWrap;
    
    std::vector< Sample > m_samples, m_outSamps, m_inSamps;
    
    sjf_verb_inputProcessor< Sample > m_inputProcessor;
    
};

