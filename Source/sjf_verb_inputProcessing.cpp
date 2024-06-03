//
//  sjf_verb_inputProcessing.cpp
//  sjf_verb - VST3
//
//  Created by Simon Fay on 10/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#include "sjf_verb_inputProcessing.h"

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================

void sjf_verb_inputProcessor::initialise( Sample sampleRate, int numberOfChannels )
{
    m_SR = sampleRate;
    NCHANNELS = numberOfChannels;
    
//    if( m_preDelays.size() != NCHANNELS )
        m_preDelays.resize( NCHANNELS );
    for ( auto & pd : m_preDelays )
        pd.initialise( m_SR * 0.5, m_SR*0.001 );
    
    m_preDelaySmoother.reset( m_SR, 0.2 );
    m_LPFSmoother.reset( m_SR, 0.05 );
    m_HPFSmoother.reset( m_SR, 0.05 );
    
    m_inputLPF.resize( numberOfChannels );
    m_inputHPF.resize( numberOfChannels );
    
}

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================

void sjf_verb_inputProcessor::processBlock( juce::AudioBuffer<Sample> &revBuffer, size_t blockSize )
{
    Sample samp = 0.0, pdDT = 0, lpfCO = 0, hpfCO= 0;
    if ( m_reversed )
    {
        for ( auto i = 0; i < blockSize; i++ )
        {
            pdDT = m_preDelaySmoother.getNextValue();
            for ( auto c = 0; c < NCHANNELS; c++ )
            {
                m_preDelays[ c ].setDelayTime( pdDT );
                m_preDelays[ c ].setSample( revBuffer.getSample( c, i ) );
                revBuffer.setSample( c, i, m_preDelays[ c ].getSampleReversed() );
            }
        }
    }
    else
    {
        for ( auto i = 0; i < blockSize; i++ )
        {
            pdDT = m_preDelaySmoother.getNextValue();
            for ( auto c = 0; c < NCHANNELS; c++ )
            {
                m_preDelays[ c ].setDelayTime( pdDT );
                m_preDelays[ c ].setSample( revBuffer.getSample( c, i ) );
                revBuffer.setSample( c, i, m_preDelays[ c ].getSample() );
            }
        }
    }
    
    for ( auto i = 0; i < blockSize; i++ )
    {
        lpfCO = m_LPFSmoother.getNextValue();
        hpfCO = m_HPFSmoother.getNextValue();
        for ( auto c = 0; c < NCHANNELS; c++ )
        {
            samp = m_inputLPF[ c ].process( revBuffer.getSample( c, i ), lpfCO );
            samp = m_inputHPF[ c ].processHP( samp, hpfCO );
            revBuffer.setSample( c, i, samp );
        }
    }
    
}

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================


void sjf_verb_inputProcessor::setReversed( bool shouldReverse)
{
    m_reversed = shouldReverse;
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
