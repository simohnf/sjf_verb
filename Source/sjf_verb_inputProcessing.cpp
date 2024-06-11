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
template< typename INTERPOLATION >
void sjf_verb_inputProcessor<INTERPOLATION>::initialise( Sample sampleRate, int numberOfChannels )
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
template< typename INTERPOLATION >
void sjf_verb_inputProcessor<INTERPOLATION>::processBlock( juce::AudioBuffer<Sample> &revBuffer, size_t blockSize )
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

template< typename INTERPOLATION >
void sjf_verb_inputProcessor<INTERPOLATION>::setReversed( bool shouldReverse)
{
    m_reversed = shouldReverse;
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================

template class sjf_verb_inputProcessor<sjf::interpolation::noneInterpolate<float> >;
template class sjf_verb_inputProcessor<sjf::interpolation::linearInterpolate<float> >;
template class sjf_verb_inputProcessor<sjf::interpolation::cubicInterpolate<float> >;
template class sjf_verb_inputProcessor<sjf::interpolation::fourPointInterpolatePD<float> >;
template class sjf_verb_inputProcessor<sjf::interpolation::fourPointFourthOrderOptimal<float> >;
template class sjf_verb_inputProcessor<sjf::interpolation::cubicInterpolateGodot<float> >;
template class sjf_verb_inputProcessor<sjf::interpolation::cubicInterpolateHermite<float> >;
