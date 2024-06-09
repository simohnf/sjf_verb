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

class sjf_verb
{
    using Sample = float;
public:
    
    sjf_verb() {}
    ~sjf_verb(){}
    
    void processBlock( juce::AudioBuffer<Sample>& buffer );
    
    void addParametersToHandler( juce::AudioProcessorValueTreeState &vts, juce::Array<juce::AudioProcessorParameter*>& params  );
    
    void initialise( Sample sampleRate, int samplesPerBlock, int numberOfChannels );
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout( );
private:
    sjf::parameterHandler::paramHandlerVector m_paramHandler;
    
    juce::LinearSmoothedValue< Sample > m_erLevelSmoother, m_lrLevelSmoother, m_drySmoother, m_wetSmoother;
    
    Sample m_SR{44100.0};
    
    sjf_verb_inputProcessor   m_inputProcessor;
    sjf_verb_earlyProcessor   m_earlyReflections;
    sjf_verb_lateProcessor< sjf::interpolation::fourPointInterpolatePD<float> >    m_lateReflections;
    sjf_verb_outputProcessor  m_outputProcessor;
    
    juce::AudioBuffer<Sample> m_revBuffer, m_outputBuffer;
};

